use log::warn;

use crate::{
    convert::TraceConverter,
    decode::evts::{BaseEvt, BaseEvtKind, BaseMetadataEvt},
    ISRState, Trace, TraceErrMarker, UserEvtMarker,
};

impl TraceConverter {
    pub(crate) fn convert_base_evt(&self, t: &mut Trace, core_id: usize, e: &BaseEvt) {
        let ts = e.ts;

        match &e.kind {
            // Core Seperators (ignore)
            BaseEvtKind::CoreId(_) => (),

            BaseEvtKind::DroppedEvtCnt(evt) => {
                if t.dropped_evt_cnt < evt.cnt {
                    let dropped = u32::wrapping_sub(evt.cnt, t.dropped_evt_cnt);
                    t.dropped_evt_cnt = evt.cnt;
                    warn!(
                        "[{ts:012}] Detected {} dropped events! Total droppped count: {}.",
                        dropped, t.dropped_evt_cnt
                    );
                    t.error_evts
                        .push(ts, TraceErrMarker::dropped(dropped, evt.cnt, core_id));
                }
            }

            BaseEvtKind::IsrEnter(evt) => {
                let isr_id = evt.isr_id as usize;
                let isr = t.core_mut(core_id).isrs.get_mut_or_create(isr_id);
                if matches!(isr.current_state, ISRState::NotActive) {
                    isr.state.push(ts, ISRState::Active);
                    isr.current_state = ISRState::Active;
                }
            }

            BaseEvtKind::IsrExit(evt) => {
                let isr_id = evt.isr_id as usize;
                let isr = t.core_mut(core_id).isrs.get_mut_or_create(isr_id);
                if matches!(isr.current_state, ISRState::Active) {
                    isr.state.push(ts, ISRState::NotActive);
                    isr.current_state = ISRState::NotActive;
                }
            }

            BaseEvtKind::Evtmarker(evt) => {
                let evtmarker_id = evt.evtmarker_id as usize;
                let evtmarker = t.user_evt_markers.get_mut_or_create(evtmarker_id);
                evtmarker
                    .markers
                    .push(ts, UserEvtMarker::Instant { msg: evt.msg.clone() })
            }

            BaseEvtKind::EvtmarkerBegin(evt) => {
                let evtmarker_id = evt.evtmarker_id as usize;
                let evtmarker = t.user_evt_markers.get_mut_or_create(evtmarker_id);
                evtmarker
                    .markers
                    .push(ts, UserEvtMarker::SliceBegin { msg: evt.msg.clone() })
            }

            BaseEvtKind::EvtmarkerEnd(evt) => {
                let evtmarker_id = evt.evtmarker_id as usize;
                let evtmarker = t.user_evt_markers.get_mut_or_create(evtmarker_id);
                evtmarker.markers.push(ts, UserEvtMarker::SliceEnd)
            }

            BaseEvtKind::Valmarker(evt) => {
                let valmarker_id = evt.valmarker_id as usize;
                let valmarker = t.user_val_markers.get_mut_or_create(valmarker_id);
                valmarker.vals.push(ts, evt.val)
            }
        }
    }

    pub(crate) fn convert_base_metadata_evt(&self, t: &mut Trace, core_id: usize, e: &BaseMetadataEvt) {
        match e {
            BaseMetadataEvt::TsResolutionNs(evt) => {
                if evt.ns_per_ts == 0 {
                    warn!("[--METADATA--] Received invalid ts resolution of {}ns - Ignoring.", evt.ns_per_ts);
                    return;
                }

                if let Some(current_val) = t.ts_resolution_ns {
                    if current_val != evt.ns_per_ts {
                        warn!(
                        "[--METADATA--] Warning: received ts resolution of {}ns, overwritting previous value of {}ns.",
                        evt.ns_per_ts, current_val
                        );
                    }
                }

                t.ts_resolution_ns = Some(evt.ns_per_ts);
            }

            BaseMetadataEvt::IsrName(evt) => {
                let isr_id = evt.isr_id as usize;
                let isr = t.core_mut(core_id).isrs.get_mut_or_create(isr_id);
                if let Some(previous_name) = &mut isr.name {
                    if *previous_name != evt.name {
                        warn!("[--METADATA--] Overiding isr #{isr_id} name from '{previous_name}' to '{}'.", evt.name);
                    }
                }
                isr.name = Some(evt.name.clone());
            }

            BaseMetadataEvt::EvtmarkerName(evt) => {
                let evtmarker_id = evt.evtmarker_id as usize;
                let evtmarker = t.user_evt_markers.get_mut_or_create(evtmarker_id);
                if let Some(previous_name) = &evtmarker.name {
                    if *previous_name != evt.name {
                        warn!(
                            "[--METADATA--] Overiding Event Marker #{evtmarker_id} name from '{previous_name}' to '{}'.",
                            evt.name
                        );
                    }
                }
                evtmarker.name = Some(evt.name.clone());
            }

            BaseMetadataEvt::ValmarkerName(evt) => {
                let valmarker_id = evt.valmarker_id as usize;
                let valmarker = t.user_val_markers.get_mut_or_create(valmarker_id);
                if let Some(previous_name) = &valmarker.name {
                    if *previous_name != evt.name {
                        warn!(
                            "[--METADATA--] Overiding Value Marker #{valmarker_id} name from '{previous_name}' to '{}'.",
                            evt.name
                        );
                    }
                }
                valmarker.name = Some(evt.name.clone());
            }
        }
    }
}
