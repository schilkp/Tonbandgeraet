use synthetto::{encode_trace, CounterTrackUnit, Synthetto, TracePacket};

use crate::{TaskKind, TaskState, Trace};

impl Trace {
    pub fn generate_perfetto_trace(&self) -> Vec<u8> {
        let mut proto_evts = vec![];

        let mut syn = Synthetto::new();

        // Global Tracks:
        self.generate_error_track(&mut syn, &mut proto_evts);
        self.generate_queue_tracks(&mut syn, &mut proto_evts);

        // Per-Task tracks:
        self.generate_task_tracks(&mut syn, &mut proto_evts);
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

    fn generate_queue_tracks(&self, syn: &mut Synthetto, evts: &mut Vec<TracePacket>) {
        for (queue_id, queue) in &self.queues {
            let trace_name = format!("{} State", self.name_queue(*queue_id));
            if queue.kind.is_mutex() {
                let track = syn.new_global_track(trace_name);
                evts.extend(syn.new_descriptor_trace_evts());

                let mut in_event = false;
                for evt in &queue.state.0 {
                    let ts = evt.ts;
                    if in_event {
                        evts.push(track.slice_end_evt(ts));
                    }
                    let state_name = if evt.inner.fill == 0 {
                        if let Some(locked_by_id) = evt.inner.by_task {
                            format!("Tacken by {}", self.name_task(locked_by_id))
                        } else {
                            String::from("Tacken")
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
                    let ts = evt.ts;
                    evts.push(track.int_counter_evt(ts, evt.inner.fill));
                }
            }
        }

        let track = syn.new_global_track("Tracing Errors".to_string());
        evts.extend(syn.new_descriptor_trace_evts());

        for error_evt in &self.error_evts.0 {
            let ts = self.convert_ts(error_evt.ts);
            evts.push(track.instant_evt(ts, format!("{:?}", error_evt.inner)));
        }
    }

    fn core_pid_offset(&self) -> i32 {
        1
    }

    fn task_pid_offset(&self) -> i32 {
        i32::max(self.core_count as i32 + self.core_pid_offset(), 20)
    }

    fn generate_task_tracks(&self, syn: &mut Synthetto, evts: &mut Vec<TracePacket>) {
        for (task_id, task) in &self.tasks {
            let process_name = self.name_task(*task_id);

            let task_track_process =
                syn.new_process((*task_id as i32) + self.task_pid_offset(), process_name, vec![], None);
            let state_track = syn.new_process_track("State".to_string(), &task_track_process);
            let priority_track = syn.new_process_counter_track(
                "Priority".to_string(),
                CounterTrackUnit::Custom(String::from("Priority")),
                1,
                false,
                &task_track_process,
            );
            let running_track = syn.new_process_track(self.name_task(*task_id), &task_track_process);
            evts.extend(syn.new_descriptor_trace_evts());

            // Generate "running" track:
            let mut running = false;
            for evt in &task.state.0 {
                let ts = self.convert_ts(evt.ts);
                let state_name = evt.inner.rich_name(self);
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
                let state_name = evt.inner.rich_name(self);

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
        }
    }

    fn generate_core_tracks(&self, syn: &mut Synthetto, evts: &mut Vec<TracePacket>) {
        for (core_id, core) in &self.cores {
            let core_name = format!("Core #{core_id}");

            let core_process =
                syn.new_process((*core_id as i32) + self.core_pid_offset(), core_name.clone(), vec![], None);
            let core_parent_track = syn.new_process_track(core_name.clone(), &core_process);
            evts.extend(syn.new_descriptor_trace_evts());

            for (task_id, task) in &self.tasks {
                let core_track = syn.new_stacked_process_track(&core_parent_track);
                evts.extend(syn.new_descriptor_trace_evts());

                let mut task_running_on_core = false;

                for evt in &task.state.0 {
                    let ts = self.convert_ts(evt.ts);

                    if let TaskState::Running { core_id: task_core_id } = evt.inner {
                        if *core_id == task_core_id {
                            if let TaskKind::Idle { .. } = &task.kind {
                                if task_running_on_core {
                                    evts.push(core_track.slice_end_evt(ts));
                                    task_running_on_core = false;
                                }
                            } else {
                                if task_running_on_core {
                                    evts.push(core_track.slice_end_evt(ts));
                                }
                                evts.push(core_track.slice_begin_evt(ts, Some(self.name_task(*task_id))));
                                task_running_on_core = true;
                            }
                        }
                    } else if task_running_on_core {
                        evts.push(core_track.slice_end_evt(ts));
                        task_running_on_core = false;
                    }
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
