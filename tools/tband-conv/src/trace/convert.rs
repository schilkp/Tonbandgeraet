use crate::{
    decode::{
        evts::{BaseEvt, BaseEvtKind, InvalidEvt, RawEvt, TraceMode},
        StreamDecoder,
    },
    Trace, TraceErrMarker, TraceEvtMarker,
};
use anyhow::anyhow;
use log::{debug, info, trace, warn};

#[derive(Debug, Clone)]
struct TraceEvt {
    core_id: usize,
    ts: Option<u64>,
    kind: RawEvt,
}

// ==== Trace Converter ========================================================

pub struct TraceConverter {
    mode: TraceMode,
    core_count: usize,
    common_stream_decoder: StreamDecoder,
    core_stream_decoder: Vec<StreamDecoder>,
    evts: TraceEvtSequence,
}

impl TraceConverter {
    pub fn new(core_count: usize, mode: TraceMode) -> anyhow::Result<Self> {
        if core_count == 0 {
            return Err(anyhow!("Core count must be greater than 0."));
        }

        Ok(TraceConverter {
            core_count,
            mode,
            common_stream_decoder: StreamDecoder::new(mode),
            core_stream_decoder: Vec::from_iter(std::iter::repeat_n(StreamDecoder::new(mode), core_count)),
            evts: TraceEvtSequence::new(core_count),
        })
    }

    pub fn add_binary(&mut self, data: &[u8]) -> anyhow::Result<()> {
        let evts = self.common_stream_decoder.process_binary(data);
        self.add_evts(&evts)
    }

    pub fn add_binary_to_core(&mut self, data: &[u8], core_id: u32) -> anyhow::Result<()> {
        if core_id as usize >= self.core_count {
            return Err(anyhow!(
                "Attempted to add binary for core {} but trace converter was configure for {} cores!",
                core_id,
                self.core_count
            ));
        }

        let evts = self.core_stream_decoder[core_id as usize].process_binary(data);

        self.add_evts_to_core(&evts, core_id)
    }

    pub fn add_evts(&mut self, evts: &[RawEvt]) -> anyhow::Result<()> {
        self.evts.add_evts(evts)
    }

    pub fn add_evts_to_core(&mut self, evts: &[RawEvt], core_id: u32) -> anyhow::Result<()> {
        self.evts.add_evts_to_core(evts, core_id)
    }

    pub fn convert(&mut self) -> anyhow::Result<Trace> {
        info!("Starting initial conversion for {}-core trace. Event count: {}.", self.core_count, self.evts.len());

        for (core_id, stream_decoder) in self.core_stream_decoder.iter().enumerate() {
            let bytes_left = stream_decoder.get_bytes_in_buffer();
            if bytes_left != 0 {
                warn!("Unfinished frame of {bytes_left} trailing bytes in input stream for core {core_id}!");
            }
        }

        let Some(max_idx) = self.evts.convertable_evt_idx() else {
            return Err(anyhow!("Cannot convert with zero results."));
        };

        debug!("Number of events that can be processed: {}.", max_idx + 1);
        debug!("Number of events that can not be processed: {}.", self.evts.len() - max_idx - 1);

        let mut trace = Trace::new(self.core_count, self.mode);

        for evt_idx in 0..=max_idx {
            let evt = &self.evts.evts[evt_idx];
            let core_id = evt.core_id;

            self.generate_trace_event_marker(&mut trace, evt);

            match &evt.kind {
                RawEvt::Invalid(evt) => self.convert_invalid_evt(&mut trace, core_id, evt),
                RawEvt::Base(evt) => self.convert_base_evt(&mut trace, core_id, evt),
                RawEvt::BaseMetadata(evt) => self.convert_base_metadata_evt(&mut trace, core_id, evt),
                RawEvt::FreeRTOS(evt) => self.convert_freertos_evt(&mut trace, core_id, evt),
                RawEvt::FreeRTOSMetadata(evt) => self.convert_freertos_metadata_evt(&mut trace, core_id, evt),
            }
        }

        if trace.ts_resolution_ns.is_none() {
            warn!("Trace did not include timestamp timer resolution - assuming 1ns.");
        }

        Ok(trace)
    }

    fn generate_trace_event_marker(&self, t: &mut Trace, e: &TraceEvt) {
        if let Some(ts) = e.ts {
            t.cores
                .get_mut(&e.core_id)
                .unwrap()
                .evts
                .push(ts, TraceEvtMarker(e.kind.clone()));
        }
    }

    fn convert_invalid_evt(&self, t: &mut Trace, core_id: usize, e: &InvalidEvt) {
        if let Some(ts) = e.ts {
            t.error_evts.push(ts, TraceErrMarker::invalid(core_id, e));
        }
    }
}

// ==== Trace Event Sequence ===================================================

struct TraceEvtSequence {
    core_count: usize,
    current_core: usize,
    evts: Vec<TraceEvt>,
    core_max_ts: Vec<u64>,
}

impl TraceEvtSequence {
    fn new(core_count: usize) -> Self {
        TraceEvtSequence {
            current_core: 0,
            evts: vec![],
            core_count,
            core_max_ts: Vec::from_iter(std::iter::repeat_n(0, core_count)),
        }
    }

    fn add_evts(&mut self, evts: &[RawEvt]) -> anyhow::Result<()> {
        self.evts.reserve_exact(evts.len());

        for evt in evts {
            let ts = evt.ts();

            if let RawEvt::Base(BaseEvt {
                ts,
                kind: BaseEvtKind::CoreId(evt),
            }) = &evt
            {
                let core_id = evt.core_id as usize;

                if core_id >= self.core_count {
                    return Err(anyhow!(
                        "Received core id event for core {} but trace converter was configure for {} cores!",
                        core_id,
                        self.core_count
                    ));
                }

                if self.core_max_ts[core_id] > *ts {
                    return Err(anyhow!("Trace event time stamps for core {} are out of order.", self.current_core));
                }
                self.core_max_ts[core_id] = *ts;
                self.current_core = core_id;

                continue;
            }

            if let Some(ts) = ts {
                trace!("[{ts:012}] [C{:01}] {:?}", self.current_core, evt);

                if self.core_max_ts[self.current_core] > ts {
                    return Err(anyhow!("Trace event time stamps for core {} are out of order.", self.current_core));
                }

                self.core_max_ts[self.current_core] = ts;

                self.evts.push(TraceEvt {
                    core_id: self.current_core,
                    kind: evt.clone(),
                    ts: Some(ts),
                })
            } else {
                debug!("[-----??-----] [C{:01}] {:?}", self.current_core, evt);
                self.evts.push(TraceEvt {
                    core_id: self.current_core,
                    kind: evt.clone(),
                    ts: None,
                })
            }
        }

        Ok(())
    }

    fn len(&self) -> usize {
        self.evts.len()
    }

    fn add_evts_to_core(&mut self, evts: &[RawEvt], core_id: u32) -> anyhow::Result<()> {
        let core_id = core_id as usize;

        if core_id >= self.core_count {
            return Err(anyhow!(
                "Attempted to add event for core {} but trace converter was configure for {} cores!",
                core_id,
                self.core_count
            ));
        }

        self.current_core = core_id;
        self.add_evts(evts)
    }

    fn max_shared_ts(&self) -> u64 {
        *self.core_max_ts.iter().min().unwrap()
    }

    fn convertable_evt_idx(&mut self) -> Option<usize> {
        self.evts.sort_unstable_by_key(|x| x.ts.unwrap_or(0));

        for (core_id, max_ts) in self.core_max_ts.iter().enumerate() {
            debug!("Largest timestamp on core {core_id}: {max_ts}.");
        }

        let conversion_timestamp_limit = self.max_shared_ts();

        match self
            .evts
            .binary_search_by_key(&conversion_timestamp_limit, |x| x.ts.unwrap_or(0))
        {
            Ok(mut idx) => {
                while (idx + 1 < self.evts.len()) && (self.evts[idx + 1].ts.unwrap_or(0) == conversion_timestamp_limit)
                {
                    idx += 1;
                }
                Some(idx)
            }
            Err(insert_idx) => {
                if insert_idx == 0 {
                    None
                } else {
                    Some(insert_idx - 1)
                }
            }
        }
    }
}

#[cfg(test)]
mod tests {

    use super::*;

    use crate::decode::evts::{BaseCoreIdEvt, BaseEvt, BaseEvtKind, BaseIsrEnterEvt};

    fn dummy_core_id_evt(ts: u64, core_id: u32) -> RawEvt {
        RawEvt::Base(BaseEvt {
            ts,
            kind: BaseEvtKind::CoreId(BaseCoreIdEvt { core_id }),
        })
    }

    fn dummy_raw_evt(ts: u64) -> RawEvt {
        RawEvt::Base(BaseEvt {
            ts,
            kind: BaseEvtKind::IsrEnter(BaseIsrEnterEvt { isr_id: 0 }),
        })
    }

    #[test]
    fn out_of_order_evts_one_core() {
        let mut t = TraceEvtSequence::new(1);
        t.add_evts(&[dummy_raw_evt(0)]).unwrap();
        t.add_evts(&[dummy_raw_evt(1)]).unwrap();
        t.add_evts(&[dummy_raw_evt(2)]).unwrap();
        t.add_evts(&[dummy_raw_evt(2)]).unwrap();
        t.add_evts(&[dummy_raw_evt(1)]).unwrap_err(); // Event out of order
    }

    #[test]
    fn out_of_order_evts_multi_core() {
        let mut t = TraceEvtSequence::new(2);
        t.add_evts(&[dummy_raw_evt(0)]).unwrap(); // Core 0, TS: 0
        t.add_evts(&[dummy_raw_evt(2)]).unwrap(); // Core 0, TS: 2
        t.add_evts(&[dummy_core_id_evt(0, 1)]).unwrap(); // Core 1, TS: 0
        t.add_evts(&[dummy_raw_evt(1)]).unwrap();
        t.add_evts(&[dummy_raw_evt(1)]).unwrap(); // Core 1, TS: 1
        t.add_evts(&[dummy_core_id_evt(0, 0)]).unwrap_err(); // Core 0, TS: 0
    }

    #[test]
    fn max_shared_ts_idx_single_core() {
        let mut t = TraceEvtSequence::new(1);
        assert_eq!(t.convertable_evt_idx(), None);
        t.add_evts(&[dummy_raw_evt(0)]).unwrap();
        t.add_evts(&[dummy_raw_evt(1)]).unwrap();
        assert_eq!(t.convertable_evt_idx(), Some(1));
        t.add_evts(&[dummy_raw_evt(2)]).unwrap();
        assert_eq!(t.convertable_evt_idx(), Some(2));
    }

    #[test]
    fn max_shared_ts_idx_multi_core() {
        let mut t = TraceEvtSequence::new(2);
        assert_eq!(t.convertable_evt_idx(), None);

        // Core 0, TS: 0
        t.add_evts(&[dummy_raw_evt(1)]).unwrap();
        // > Core 0: | 1
        //   Core 1: |
        assert_eq!(t.convertable_evt_idx(), None);

        // Core 1, TS: 0
        t.add_evts(&[dummy_core_id_evt(1, 1)]).unwrap();
        //   Core 0: 1 |
        // > Core 1:   |
        assert_eq!(t.convertable_evt_idx(), Some(0));

        // Core 1, TS: 2
        t.add_evts(&[dummy_raw_evt(2)]).unwrap();
        //   Core 0: 1 |
        // > Core 1:   | 2
        assert_eq!(t.convertable_evt_idx(), Some(0));

        // Core 0, TS: 1
        t.add_evts(&[dummy_core_id_evt(1, 0)]).unwrap();
        // > Core 0: 1 |
        //   Core 1:   | 2
        assert_eq!(t.convertable_evt_idx(), Some(0));

        // Core 0, TS: 2
        t.add_evts(&[dummy_raw_evt(2)]).unwrap();
        // > Core 0: 1 2 |
        //   Core 1:   2 |
        assert_eq!(t.convertable_evt_idx(), Some(2));

        // Core 0, TS: 10
        t.add_evts(&[dummy_raw_evt(10)]).unwrap();
        // > Core 0: 1 2 | 10
        //   Core 1:   2 |
        assert_eq!(t.convertable_evt_idx(), Some(2));

        // Core 1, TS: 9
        t.add_evts(&[dummy_core_id_evt(9, 1)]).unwrap();
        //   Core 0: 1 2 | 10
        // > Core 1:   2 |
        assert_eq!(t.convertable_evt_idx(), Some(2));

        // Core 0, TS: 11
        t.add_evts(&[dummy_raw_evt(11)]).unwrap();
        //   Core 0: 1 2 10 |
        // > Core 1:   2    | 11
        assert_eq!(t.convertable_evt_idx(), Some(3));
    }
}
