use synthetto::{CounterTrackUnit, EventTrack, Process, Synthetto, TracePacket, Track};

use crate::Trace;

use super::{TaskKind, TaskState};

impl Trace {
    pub(crate) fn generate_freertos_queue_tracks(&self, syn: &mut Synthetto, evts: &mut Vec<TracePacket>) {
        for (queue_id, queue) in &self.freertos.queues {
            let trace_name = format!("{} State", self.freertos.name_queue(*queue_id));
            if queue.kind.is_mutex() {
                let track = syn.new_global_track(trace_name);
                evts.extend(syn.new_descriptor_trace_evts());

                let mut in_event = false;
                for evt in &queue.state.0 {
                    let ts = self.convert_ts(evt.ts);
                    if in_event {
                        evts.push(track.slice_end_evt(ts));
                    }
                    let state_name = if evt.inner.fill == 0 {
                        if let Some(locked_by_id) = evt.inner.by_task {
                            format!("Taken by {}", self.freertos.name_task(locked_by_id))
                        } else {
                            String::from("Taken")
                        }
                    } else {
                        String::from("Available")
                    };
                    evts.push(track.slice_begin_evt(ts, Some(state_name)));
                    in_event = true;
                }
            } else {
                let track = syn.new_global_counter_track(trace_name, CounterTrackUnit::Count, 1, false);
                evts.extend(syn.new_descriptor_trace_evts());
                for evt in &queue.state.0 {
                    let ts = self.convert_ts(evt.ts);
                    evts.push(track.int_counter_evt(ts, evt.inner.fill));
                }
            }
        }
    }

    pub(crate) fn generate_freertos_task_tracks(&self, syn: &mut Synthetto, evts: &mut Vec<TracePacket>) {
        for (task_id, task) in &self.freertos.tasks {
            let process_name = self.freertos.name_task(*task_id);

            let task_track_process =
                syn.new_process((*task_id as i32) + self.rtos_pid_offset(), process_name.clone(), vec![], None);
            let state_track = syn.new_process_track(format!("{process_name} State"), &task_track_process);
            let priority_track = syn.new_process_counter_track(
                format!("{process_name} Priority"),
                CounterTrackUnit::Custom(String::from("Priority")),
                1,
                false,
                &task_track_process,
            );
            let running_track = syn.new_process_track(self.freertos.name_task(*task_id), &task_track_process);
            evts.extend(syn.new_descriptor_trace_evts());

            // Generate "running" track:
            let mut running = false;
            for evt in &task.state.0 {
                let ts = self.convert_ts(evt.ts);
                let state_name = evt.inner.rich_name(&self.freertos);
                if let TaskState::Running { .. } = evt.inner {
                    if !running {
                        evts.push(running_track.slice_begin_evt(ts, Some(state_name)));
                        running = true;
                    }
                } else if running {
                    evts.push(running_track.slice_end_evt(ts));
                    running = false;
                }
            }

            // Generate "state" track:
            for (idx, evt) in task.state.0.iter().enumerate() {
                let ts = self.convert_ts(evt.ts);
                let state_name = evt.inner.rich_name(&self.freertos);

                if idx != 0 {
                    evts.push(state_track.slice_end_evt(ts));
                }
                evts.push(state_track.slice_begin_evt(ts, Some(state_name)));
            }

            // Generate "priority" track:
            for priority in &task.priority.0 {
                let ts = self.convert_ts(priority.ts);
                evts.push(priority_track.int_counter_evt(ts, priority.inner));
            }

            // Generate "user event marker" tracks:
            for (marker_id, marker) in &task.user_evt_markers {
                let marker_name = task.name_user_evtmarker(*marker_id);
                let track = syn.new_process_track(marker_name, &task_track_process);
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

            // Generate "user value marker" tracks:
            for (marker_id, marker) in &task.user_val_markers {
                let marker_name = task.name_user_valmarker(*marker_id);
                let track = syn.new_process_counter_track(
                    marker_name,
                    CounterTrackUnit::Unspecified,
                    1,
                    false,
                    &task_track_process,
                );
                evts.extend(syn.new_descriptor_trace_evts());

                for evt in &marker.vals.0 {
                    let ts = self.convert_ts(evt.ts);
                    evts.push(track.int_counter_evt(ts, evt.inner));
                }
            }
        }
    }

    pub(crate) fn generate_freertos_core_tracks(
        &self,
        syn: &mut Synthetto,
        evts: &mut Vec<TracePacket>,
        core_id: usize,
        parent_track: &Track<Process, EventTrack>,
    ) {
        for (task_id, task) in &self.freertos.tasks {
            let core_track = syn.new_stacked_process_track(parent_track);
            evts.extend(syn.new_descriptor_trace_evts());

            let mut task_running_on_core = false;

            for evt in &task.state.0 {
                let ts = self.convert_ts(evt.ts);

                if let TaskState::Running { core_id: task_core_id } = evt.inner {
                    if core_id == task_core_id {
                        if let TaskKind::Idle { .. } = &task.kind {
                            if task_running_on_core {
                                evts.push(core_track.slice_end_evt(ts));
                                task_running_on_core = false;
                            }
                        } else {
                            if task_running_on_core {
                                evts.push(core_track.slice_end_evt(ts));
                            }
                            evts.push(core_track.slice_begin_evt(ts, Some(self.freertos.name_task(*task_id))));
                            task_running_on_core = true;
                        }
                    }
                } else if task_running_on_core {
                    evts.push(core_track.slice_end_evt(ts));
                    task_running_on_core = false;
                }
            }
        }
    }
}
