pub mod generate_perfetto;

use crate::{
    decode::{
        evts::{RawEvt, RawInvalidEvt, RawMetadataEvt, RawTraceEvt, RawTraceEvtKind},
        StreamDecoder,
    },
    ISRState, QueueKind, QueueState, TaskBlockingReason, TaskKind, TaskState, Trace, TraceErrMarker, TraceEvtMarker,
    UserEvtMarker,
};
use anyhow::anyhow;
use log::{debug, info, trace, warn};
use std::iter::repeat;

#[derive(Debug, Clone)]
struct TraceEvt {
    core_id: usize,
    ts: Option<u64>,
    kind: RawEvt,
}

// ==== Trace Converter ========================================================

pub struct TraceConverter {
    core_count: usize,
    common_stream_decoder: StreamDecoder,
    core_stream_decoder: Vec<StreamDecoder>,
    evts: TraceEvtSequence,
}

impl TraceConverter {
    pub fn new(core_count: usize) -> anyhow::Result<Self> {
        if core_count == 0 {
            return Err(anyhow!("Core count must be greater than 0."));
        }

        Ok(TraceConverter {
            core_count,
            common_stream_decoder: StreamDecoder::new(),
            core_stream_decoder: Vec::from_iter(repeat(StreamDecoder::new()).take(core_count)),
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

        let mut trace = Trace::new(self.core_count);

        for evt_idx in 0..max_idx {
            let evt = &self.evts.evts[evt_idx];
            let core_id = evt.core_id;

            self.generate_trace_event_marker(&mut trace, evt);

            match &evt.kind {
                RawEvt::Metadata(evt) => self.convert_metadata_evt(&mut trace, core_id, evt),
                RawEvt::Trace(evt) => self.convert_trace_evt(&mut trace, core_id, evt),
                RawEvt::Invalid(evt) => self.convert_invalid_evt(&mut trace, core_id, evt),
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

    fn convert_metadata_evt(&self, t: &mut Trace, core_id: usize, e: &RawMetadataEvt) {
        match e {
            RawMetadataEvt::TsResolutionNs(evt) => {
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

            RawMetadataEvt::TaskName(evt) => {
                let task_id = evt.task_id as usize;
                let task = t.tasks.get_mut_or_create(task_id);
                if let Some(previous_name) = &mut task.name {
                    if *previous_name != evt.name {
                        warn!(
                            "[--METADATA--] Overiding task #{task_id} name from '{previous_name}' to '{}'.",
                            evt.name
                        );
                    }
                }
                task.name = Some(evt.name.clone());
            }

            RawMetadataEvt::TaskIsIdleTask(evt) => {
                let task_id = evt.task_id as usize;
                let task = t.tasks.get_mut_or_create(task_id);
                let new_kind = TaskKind::Idle {
                    core_id: evt.core_id as usize,
                };
                if !matches!(task.kind, TaskKind::Normal) && new_kind != task.kind {
                    warn!("[--METADATA--] Overriding task #{task_id} type from '{}' to '{}'.", task.kind, new_kind);
                }
                task.kind = new_kind;
            }

            RawMetadataEvt::TaskIsTimerTask(evt) => {
                let task_id = evt.task_id as usize;
                let task = t.tasks.get_mut_or_create(task_id);
                let new_kind = TaskKind::TimerSvc;
                if !matches!(task.kind, TaskKind::Normal) && new_kind != task.kind {
                    warn!("[--METADATA--] Overriding task #{task_id} type from '{}' to '{}'.", task.kind, new_kind);
                }
                task.kind = new_kind;
            }

            RawMetadataEvt::IsrName(evt) => {
                let isr_id = evt.isr_id as usize;
                let isr = t.core_mut(core_id).isrs.get_mut_or_create(isr_id);
                if let Some(previous_name) = &mut isr.name {
                    if *previous_name != evt.name {
                        warn!("[--METADATA--] Overiding isr #{isr_id} name from '{previous_name}' to '{}'.", evt.name);
                    }
                }
                isr.name = Some(evt.name.clone());
            }

            RawMetadataEvt::QueueName(evt) => {
                let queue_id = evt.queue_id as usize;
                let queue = t.queues.get_mut_or_create(queue_id);
                if let Some(previous_name) = &mut queue.name {
                    if *previous_name != evt.name {
                        warn!(
                            "[--METADATA--] Overiding queue #{queue_id} name from '{previous_name}' to '{}'.",
                            evt.name
                        );
                    }
                }
                queue.name = Some(evt.name.clone());
            }

            RawMetadataEvt::QueueKind(evt) => {
                let queue_id = evt.queue_id as usize;
                let queue = t.queues.get_mut_or_create(queue_id);
                let new_kind: QueueKind = evt.kind.clone().into();

                if queue.kind == new_kind && !matches!(new_kind, QueueKind::Queue) {
                    warn!("[--METADATA--] Overriding queue #{queue_id} type from '{}' to '{}'.", queue.kind, new_kind);
                }
                queue.kind = new_kind;
            }

            RawMetadataEvt::EvtmarkerName(evt) => {
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

            RawMetadataEvt::ValmarkerName(evt) => {
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

            RawMetadataEvt::TaskEvtmarkerName(evt) => {
                let task_id = evt.task_id as usize;
                let evtmarker_id = evt.evtmarker_id as usize;
                let evtmarker = t
                    .tasks
                    .get_mut_or_create(task_id)
                    .user_evt_markers
                    .get_mut_or_create(evtmarker_id);
                if let Some(previous_name) = &evtmarker.name {
                    if *previous_name != evt.name {
                        warn!(
                            "[--METADATA--] Overiding Task #{} Marker #{evtmarker_id} name from '{previous_name}' to '{}'.",
                            task_id,
                            evt.name
                        );
                    }
                }
                evtmarker.name = Some(evt.name.clone());
            }

            RawMetadataEvt::TaskValmarkerName(evt) => {
                let task_id = evt.task_id as usize;
                let valmarker_id = evt.valmarker_id as usize;
                let valmarker = t
                    .tasks
                    .get_mut_or_create(task_id)
                    .user_val_markers
                    .get_mut_or_create(valmarker_id);
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

    fn convert_trace_evt(&self, t: &mut Trace, core_id: usize, e: &RawTraceEvt) {
        let ts = e.ts;

        match &e.kind {
            // Core Seperators (ignore)
            RawTraceEvtKind::CoreId(_) => (),

            RawTraceEvtKind::DroppedEvtCnt(evt) => {
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

            RawTraceEvtKind::TaskSwitchedIn(evt) => {
                let task_id = evt.task_id as usize;
                t.tasks.ensure_exists(task_id);

                // Switch-out previous task (if any):
                if let Some(previous_task_id) = t.core(core_id).current_task_id {
                    let previous_task = t.tasks.get_mut_or_create(previous_task_id);
                    previous_task
                        .state
                        .push(ts, previous_task.state_when_switched_out.clone());
                }

                // Switch-in next task:
                t.tasks.get_mut_or_create(task_id).state_when_switched_out = TaskState::Ready;
                t.tasks
                    .get_mut_or_create(task_id)
                    .state
                    .push(ts, TaskState::Running { core_id });
                t.core_mut(core_id).current_task_id = Some(task_id);
            }

            RawTraceEvtKind::TaskToRdyState(evt) => {
                let task_id = evt.task_id as usize;
                let task = t.tasks.get_mut_or_create(task_id);
                if !task.is_running() {
                    task.state.push(ts, TaskState::Ready);
                }
            }

            RawTraceEvtKind::TaskResumed(evt) => {
                let task_id = evt.task_id as usize;
                let task = t.tasks.get_mut_or_create(task_id);
                task.state.push(ts, TaskState::Ready);
            }

            RawTraceEvtKind::TaskResumedFromIsr(evt) => {
                let task_id = evt.task_id as usize;
                let task = t.tasks.get_mut_or_create(task_id);
                task.state.push(ts, TaskState::Ready);
            }

            RawTraceEvtKind::TaskSuspended(evt) => {
                let task_id = evt.task_id as usize;
                let current_task_id = t.core(core_id).current_task_id;
                let task = t.tasks.get_mut_or_create(task_id);
                if task.is_running() {
                    task.state_when_switched_out = TaskState::Suspended {
                        by_task_id: current_task_id,
                    };
                } else {
                    task.state.push(
                        ts,
                        TaskState::Suspended {
                            by_task_id: current_task_id,
                        },
                    );
                }
            }

            RawTraceEvtKind::CurtaskDelay(evt) => {
                if let Some(current_task_id) = t.core(core_id).current_task_id {
                    t.tasks.get_mut_or_create(current_task_id).state_when_switched_out =
                        TaskState::Blocked(TaskBlockingReason::Delay { ticks: evt.ticks })
                } else {
                    warn!("[{ts:012}] Received current task event while current task is not known ({:?}).", evt);
                    t.error_evts.push(ts, TraceErrMarker::no_current_task(core_id));
                }
            }

            RawTraceEvtKind::CurtaskDelayUntil(evt) => {
                if let Some(current_task_id) = t.core(core_id).current_task_id {
                    t.tasks.get_mut_or_create(current_task_id).state_when_switched_out =
                        TaskState::Blocked(TaskBlockingReason::DelayUntil {
                            time_to_wake: evt.time_to_wake,
                        })
                } else {
                    warn!("[{ts:012}] Received current task event while current task is not known ({:?}).", evt);
                    t.error_evts.push(ts, TraceErrMarker::no_current_task(core_id));
                }
            }

            RawTraceEvtKind::TaskPrioritySet(evt) => {
                let task_id = evt.task_id as usize;
                t.tasks.get_mut_or_create(task_id).priority.push(ts, evt.priority);
            }

            RawTraceEvtKind::TaskPriorityInherit(evt) => {
                let task_id = evt.task_id as usize;
                t.tasks.get_mut_or_create(task_id).priority.push(ts, evt.priority);
            }

            RawTraceEvtKind::TaskPriorityDisinherit(evt) => {
                let task_id = evt.task_id as usize;
                t.tasks.get_mut_or_create(task_id).priority.push(ts, evt.priority);
            }

            RawTraceEvtKind::TaskCreated(evt) => {
                let task_id = evt.task_id as usize;
                t.tasks.ensure_exists(task_id);
            }

            RawTraceEvtKind::TaskDeleted(evt) => {
                let task_id = evt.task_id as usize;
                let current_task_id = t.core(core_id).current_task_id;
                let task = t.tasks.get_mut_or_create(task_id);
                if task.is_running() {
                    task.state_when_switched_out = TaskState::Deleted {
                        by_task_id: current_task_id,
                    };
                } else {
                    task.state.push(
                        ts,
                        TaskState::Deleted {
                            by_task_id: current_task_id,
                        },
                    );
                }
            }

            RawTraceEvtKind::IsrEnter(evt) => {
                let isr_id = evt.isr_id as usize;
                let isr = t.core_mut(core_id).isrs.get_mut_or_create(isr_id);
                if matches!(isr.current_state, ISRState::NotActive) {
                    isr.state.push(ts, ISRState::Active);
                    isr.current_state = ISRState::Active;
                }
            }

            RawTraceEvtKind::IsrExit(evt) => {
                let isr_id = evt.isr_id as usize;
                let isr = t.core_mut(core_id).isrs.get_mut_or_create(isr_id);
                if matches!(isr.current_state, ISRState::Active) {
                    isr.state.push(ts, ISRState::NotActive);
                    isr.current_state = ISRState::NotActive;
                }
            }

            RawTraceEvtKind::QueueCreated(evt) => {
                let queue_id = evt.queue_id as usize;
                t.queues.ensure_exists(queue_id);
            }

            RawTraceEvtKind::QueueSend(evt) => {
                let current_task = t.core(core_id).current_task_id;
                let queue_id = evt.queue_id as usize;
                let queue = t.queues.get_mut_or_create(queue_id);
                queue.state.push(
                    ts,
                    QueueState {
                        fill: evt.len_after,
                        by_task: current_task,
                    },
                );
            }

            RawTraceEvtKind::QueueSendFromIsr(evt) => {
                let queue_id = evt.queue_id as usize;
                let queue = t.queues.get_mut_or_create(queue_id);
                queue.state.push(
                    ts,
                    QueueState {
                        fill: evt.len_after,
                        by_task: None,
                    },
                );
            }

            RawTraceEvtKind::QueueOverwrite(evt) => {
                let current_task = t.core(core_id).current_task_id;
                let queue_id = evt.queue_id as usize;
                let queue = t.queues.get_mut_or_create(queue_id);
                queue.state.push(
                    ts,
                    QueueState {
                        fill: evt.len_after,
                        by_task: current_task,
                    },
                );
            }

            RawTraceEvtKind::QueueOverwriteFromIsr(evt) => {
                let queue_id = evt.queue_id as usize;
                let queue = t.queues.get_mut_or_create(queue_id);
                queue.state.push(
                    ts,
                    QueueState {
                        fill: evt.len_after,
                        by_task: None,
                    },
                );
            }

            RawTraceEvtKind::QueueReceive(evt) => {
                let current_task = t.core(core_id).current_task_id;
                let queue_id = evt.queue_id as usize;
                let queue = t.queues.get_mut_or_create(queue_id);
                queue.state.push(
                    ts,
                    QueueState {
                        fill: evt.len_after,
                        by_task: current_task,
                    },
                );
            }

            RawTraceEvtKind::QueueReceiveFromIsr(evt) => {
                let current_task = t.core(core_id).current_task_id;
                let queue_id = evt.queue_id as usize;
                let queue = t.queues.get_mut_or_create(queue_id);
                queue.state.push(
                    ts,
                    QueueState {
                        fill: evt.len_after,
                        by_task: current_task,
                    },
                );
            }

            RawTraceEvtKind::QueueReset(evt) => {
                let current_task = t.core(core_id).current_task_id;
                let queue_id = evt.queue_id as usize;
                let queue = t.queues.get_mut_or_create(queue_id);
                queue.state.push(
                    ts,
                    QueueState {
                        fill: 0,
                        by_task: current_task,
                    },
                );
            }

            RawTraceEvtKind::CurtaskBlockOnQueuePeek(evt) => {
                let queue_id = evt.queue_id as usize;
                t.queues.ensure_exists(queue_id);
                if let Some(current_task_id) = t.core(core_id).current_task_id {
                    t.tasks.get_mut_or_create(current_task_id).state_when_switched_out =
                        TaskState::Blocked(TaskBlockingReason::QueuePeek { queue_id })
                } else {
                    warn!("[{ts:012}] Received current task event while current task is not known ({:?}).", evt);
                    t.error_evts.push(ts, TraceErrMarker::no_current_task(core_id));
                }
            }

            RawTraceEvtKind::CurtaskBlockOnQueueSend(evt) => {
                let queue_id = evt.queue_id as usize;
                t.queues.ensure_exists(queue_id);
                if let Some(current_task_id) = t.core(core_id).current_task_id {
                    t.tasks.get_mut_or_create(current_task_id).state_when_switched_out =
                        TaskState::Blocked(TaskBlockingReason::QueueSend { queue_id })
                } else {
                    warn!("[{ts:012}] Received current task event while current task is not known ({:?}).", evt);
                    t.error_evts.push(ts, TraceErrMarker::no_current_task(core_id));
                }
            }

            RawTraceEvtKind::CurtaskBlockOnQueueReceive(evt) => {
                let queue_id = evt.queue_id as usize;
                t.queues.ensure_exists(queue_id);

                if let Some(current_task_id) = t.core(core_id).current_task_id {
                    t.tasks.get_mut_or_create(current_task_id).state_when_switched_out =
                        TaskState::Blocked(TaskBlockingReason::QueueReceive { queue_id })
                } else {
                    warn!("[{ts:012}] Received current task event while current task is not known ({:?}).", evt);
                    t.error_evts.push(ts, TraceErrMarker::no_current_task(core_id));
                }
            }

            RawTraceEvtKind::Evtmarker(evt) => {
                let evtmarker_id = evt.evtmarker_id as usize;
                let evtmarker = t.user_evt_markers.get_mut_or_create(evtmarker_id);
                evtmarker
                    .markers
                    .push(ts, UserEvtMarker::Instant { msg: evt.msg.clone() })
            }

            RawTraceEvtKind::EvtmarkerBegin(evt) => {
                let evtmarker_id = evt.evtmarker_id as usize;
                let evtmarker = t.user_evt_markers.get_mut_or_create(evtmarker_id);
                evtmarker
                    .markers
                    .push(ts, UserEvtMarker::SliceBegin { msg: evt.msg.clone() })
            }

            RawTraceEvtKind::EvtmarkerEnd(evt) => {
                let evtmarker_id = evt.evtmarker_id as usize;
                let evtmarker = t.user_evt_markers.get_mut_or_create(evtmarker_id);
                evtmarker.markers.push(ts, UserEvtMarker::SliceEnd)
            }

            RawTraceEvtKind::Valmarker(evt) => {
                let valmarker_id = evt.valmarker_id as usize;
                let valmarker = t.user_val_markers.get_mut_or_create(valmarker_id);
                valmarker.vals.push(ts, evt.val)
            }

            RawTraceEvtKind::TaskEvtmarker(evt) => {
                let evtmarker_id = evt.evtmarker_id as usize;
                if let Some(current_task_id) = t.core(core_id).current_task_id {
                    let evtmarker = t
                        .tasks
                        .get_mut_or_create(current_task_id)
                        .user_evt_markers
                        .get_mut_or_create(evtmarker_id);
                    evtmarker
                        .markers
                        .push(ts, UserEvtMarker::Instant { msg: evt.msg.clone() })
                } else {
                    warn!("[{ts:012}] Received current task event while current task is not known ({:?}).", evt);
                    t.error_evts.push(ts, TraceErrMarker::no_current_task(core_id));
                }
            }

            RawTraceEvtKind::TaskEvtmarkerBegin(evt) => {
                let evtmarker_id = evt.evtmarker_id as usize;
                if let Some(current_task_id) = t.core(core_id).current_task_id {
                    let evtmarker = t
                        .tasks
                        .get_mut_or_create(current_task_id)
                        .user_evt_markers
                        .get_mut_or_create(evtmarker_id);
                    evtmarker
                        .markers
                        .push(ts, UserEvtMarker::SliceBegin { msg: evt.msg.clone() })
                } else {
                    warn!("[{ts:012}] Received current task event while current task is not known ({:?}).", evt);
                    t.error_evts.push(ts, TraceErrMarker::no_current_task(core_id));
                }
            }

            RawTraceEvtKind::TaskEvtmarkerEnd(evt) => {
                let evtmarker_id = evt.evtmarker_id as usize;
                if let Some(current_task_id) = t.core(core_id).current_task_id {
                    let evtmarker = t
                        .tasks
                        .get_mut_or_create(current_task_id)
                        .user_evt_markers
                        .get_mut_or_create(evtmarker_id);
                    evtmarker.markers.push(ts, UserEvtMarker::SliceEnd)
                } else {
                    warn!("[{ts:012}] Received current task event while current task is not known ({:?}).", evt);
                    t.error_evts.push(ts, TraceErrMarker::no_current_task(core_id));
                }
            }

            RawTraceEvtKind::TaskValmarker(evt) => {
                let valmarker_id = evt.valmarker_id as usize;
                if let Some(current_task_id) = t.core(core_id).current_task_id {
                    let valmarker = t
                        .tasks
                        .get_mut_or_create(current_task_id)
                        .user_val_markers
                        .get_mut_or_create(valmarker_id);
                    valmarker.vals.push(ts, evt.val)
                } else {
                    warn!("[{ts:012}] Received current task event while current task is not known ({:?}).", evt);
                    t.error_evts.push(ts, TraceErrMarker::no_current_task(core_id));
                }
            }
        }
    }

    fn convert_invalid_evt(&self, t: &mut Trace, core_id: usize, e: &RawInvalidEvt) {
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
            core_max_ts: Vec::from_iter(repeat(0).take(core_count)),
        }
    }

    fn add_evts(&mut self, evts: &[RawEvt]) -> anyhow::Result<()> {
        self.evts.reserve_exact(evts.len());

        for evt in evts {
            let ts = evt.ts();

            if let RawEvt::Trace(RawTraceEvt {
                ts,
                kind: RawTraceEvtKind::CoreId(evt),
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
    use crate::decode::evts::{CoreIdEvt, RawTraceEvtKind, TaskSwitchedInEvt};

    use super::*;

    fn dummy_core_id_evt(ts: u64, core_id: u32) -> RawEvt {
        RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::CoreId(CoreIdEvt { core_id }),
        })
    }

    fn dummy_raw_evt(ts: u64) -> RawEvt {
        RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::TaskSwitchedIn(TaskSwitchedInEvt { task_id: 1 }),
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
