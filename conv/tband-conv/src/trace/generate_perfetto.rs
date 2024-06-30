use synthetto::{encode_trace, CounterTrackUnit, Synthetto, TracePacket};

use crate::Trace;

impl Trace {
    pub fn generate_perfetto_trace(&self) -> Vec<u8> {
        let mut proto_evts = vec![];

        let mut syn = Synthetto::new();

        // Global Tracks:
        self.generate_error_track(&mut syn, &mut proto_evts);
        self.generate_marker_tracks(&mut syn, &mut proto_evts);

        match self.mode {
            crate::decode::evts::TraceMode::Base => (),
            crate::decode::evts::TraceMode::FreeRTOS => {
                self.generate_freertos_queue_tracks(&mut syn, &mut proto_evts);
                self.generate_freertos_task_tracks(&mut syn, &mut proto_evts);
            }
        }

        // Core tracks:
        self.generate_core_tracks(&mut syn, &mut proto_evts);

        encode_trace(proto_evts)
    }

    fn generate_error_track(&self, syn: &mut Synthetto, evts: &mut Vec<TracePacket>) {
        let track = syn.new_global_track("Tracing Errors".to_string());
        evts.extend(syn.new_descriptor_trace_evts());

        for error_evt in &self.error_evts.0 {
            let ts = self.convert_ts(error_evt.ts);
            evts.push(track.instant_evt(ts, format!("{:?}", error_evt.inner)));
        }
    }

    fn generate_marker_tracks(&self, syn: &mut Synthetto, evts: &mut Vec<TracePacket>) {
        for (marker_id, marker) in &self.user_evt_markers {
            let marker_name = self.name_user_evtmarker(*marker_id);

            let track = syn.new_global_track(marker_name);
            evts.extend(syn.new_descriptor_trace_evts());

            for evt in &marker.markers.0 {
                let ts = self.convert_ts(evt.ts);
                match &evt.inner {
                    crate::UserEvtMarker::Instant { msg } => {
                        evts.push(track.instant_evt(ts, msg.clone()));
                    }
                    crate::UserEvtMarker::SliceBegin { msg } => {
                        evts.push(track.slice_begin_evt(ts, Some(msg.clone())));
                    }
                    crate::UserEvtMarker::SliceEnd => {
                        evts.push(track.slice_end_evt(ts));
                    }
                }
            }
        }

        for (marker_id, marker) in &self.user_val_markers {
            let marker_name = self.name_user_valmarker(*marker_id);
            let track = syn.new_global_counter_track(marker_name, CounterTrackUnit::Unspecified, 1, false);
            evts.extend(syn.new_descriptor_trace_evts());

            for evt in &marker.vals.0 {
                let ts = self.convert_ts(evt.ts);
                evts.push(track.int_counter_evt(ts, evt.inner));
            }
        }
    }

    pub(crate) fn core_pid_offset(&self) -> i32 {
        1
    }

    pub(crate) fn rtos_pid_offset(&self) -> i32 {
        i32::max(self.core_count as i32 + self.core_pid_offset(), 20)
    }

    fn generate_core_tracks(&self, syn: &mut Synthetto, evts: &mut Vec<TracePacket>) {
        for (core_id, core) in &self.cores {
            let core_name = format!("Core #{core_id}");

            let core_process =
                syn.new_process((*core_id as i32) + self.core_pid_offset(), core_name.clone(), vec![], None);
            let core_parent_track = syn.new_process_track(core_name.clone(), &core_process);
            evts.extend(syn.new_descriptor_trace_evts());

            match self.mode {
                crate::decode::evts::TraceMode::Base => (),
                crate::decode::evts::TraceMode::FreeRTOS => {
                    self.generate_freertos_core_tracks(syn, evts, *core_id, &core_parent_track);
                }
            }

            for (isr_id, isr) in &core.isrs {
                let track_name = format!("{core_name} {}", self.name_isr(*core_id, *isr_id));

                let isr_track = syn.new_process_track(track_name, &core_process);
                evts.extend(syn.new_descriptor_trace_evts());

                let mut isr_active = false;

                for evt in &isr.state.0 {
                    let ts = self.convert_ts(evt.ts);
                    let state = &evt.inner;

                    match state {
                        crate::ISRState::Active => {
                            if !isr_active {
                                evts.push(isr_track.slice_begin_evt(ts, Some(self.name_isr(*core_id, *isr_id))));
                                isr_active = true;
                            }
                        }
                        crate::ISRState::NotActive => {
                            if isr_active {
                                evts.push(isr_track.slice_end_evt(ts));
                                isr_active = false;
                            }
                        }
                    }
                }
            }

            let evt_track_name = format!("{core_name} Trace Events");
            let evt_track = syn.new_process_track(evt_track_name, &core_process);
            evts.extend(syn.new_descriptor_trace_evts());

            for evt in &core.evts.0 {
                let ts = self.convert_ts(evt.ts);
                let evt = &evt.inner.0;
                evts.push(evt_track.instant_evt(ts, format!("{:?}", evt)));
            }
        }
    }
}
