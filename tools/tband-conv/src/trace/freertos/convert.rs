use log::warn;

use crate::{
    convert::TraceConverter,
    decode::evts::{FreeRTOSEvt, FreeRTOSEvtKind, FreeRTOSMetadataEvt},
    Trace, TraceErrMarker, UserEvtMarker,
};

use super::{QueueKind, QueueState, TaskBlockingReason, TaskKind, TaskState};

impl TraceConverter {
    pub(crate) fn convert_freertos_evt(&self, t: &mut Trace, core_id: usize, e: &FreeRTOSEvt) {
        let ts = e.ts;

        match &e.kind {
            FreeRTOSEvtKind::TaskSwitchedIn(evt) => {
                let task_id = evt.task_id as usize;
                t.freertos.tasks.ensure_exists(task_id);

                // Switch-out previous task (if any):
                if let Some(previous_task_id) = t.core(core_id).freertos.current_task_id {
                    let previous_task = t.freertos.tasks.get_mut_or_create(previous_task_id);
                    previous_task
                        .state
                        .push(ts, previous_task.state_when_switched_out.clone());
                }

                // Switch-in next task:
                t.freertos.tasks.get_mut_or_create(task_id).state_when_switched_out = TaskState::Ready;
                t.freertos
                    .tasks
                    .get_mut_or_create(task_id)
                    .state
                    .push(ts, TaskState::Running { core_id });
                t.core_mut(core_id).freertos.current_task_id = Some(task_id);
            }

            FreeRTOSEvtKind::TaskToRdyState(evt) => {
                let task_id = evt.task_id as usize;
                let task = t.freertos.tasks.get_mut_or_create(task_id);
                if !task.is_running() {
                    task.state.push(ts, TaskState::Ready);
                }
            }

            FreeRTOSEvtKind::TaskResumed(evt) => {
                let task_id = evt.task_id as usize;
                let task = t.freertos.tasks.get_mut_or_create(task_id);
                task.state.push(ts, TaskState::Ready);
            }

            FreeRTOSEvtKind::TaskResumedFromIsr(evt) => {
                let task_id = evt.task_id as usize;
                let task = t.freertos.tasks.get_mut_or_create(task_id);
                task.state.push(ts, TaskState::Ready);
            }

            FreeRTOSEvtKind::TaskSuspended(evt) => {
                let task_id = evt.task_id as usize;
                let current_task_id = t.core(core_id).freertos.current_task_id;
                let task = t.freertos.tasks.get_mut_or_create(task_id);
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

            FreeRTOSEvtKind::CurtaskDelay(evt) => {
                if let Some(current_task_id) = t.core(core_id).freertos.current_task_id {
                    t.freertos
                        .tasks
                        .get_mut_or_create(current_task_id)
                        .state_when_switched_out = TaskState::Blocked(TaskBlockingReason::Delay { ticks: evt.ticks })
                } else {
                    warn!("[{ts:012}] Received current task event while current task is not known ({:?}).", evt);
                    t.error_evts.push(ts, TraceErrMarker::no_current_task(core_id));
                }
            }

            FreeRTOSEvtKind::CurtaskDelayUntil(evt) => {
                if let Some(current_task_id) = t.core(core_id).freertos.current_task_id {
                    t.freertos
                        .tasks
                        .get_mut_or_create(current_task_id)
                        .state_when_switched_out = TaskState::Blocked(TaskBlockingReason::DelayUntil {
                        time_to_wake: evt.time_to_wake,
                    })
                } else {
                    warn!("[{ts:012}] Received current task event while current task is not known ({:?}).", evt);
                    t.error_evts.push(ts, TraceErrMarker::no_current_task(core_id));
                }
            }

            FreeRTOSEvtKind::TaskPrioritySet(evt) => {
                let task_id = evt.task_id as usize;
                t.freertos
                    .tasks
                    .get_mut_or_create(task_id)
                    .priority
                    .push(ts, evt.priority);
            }

            FreeRTOSEvtKind::TaskPriorityInherit(evt) => {
                let task_id = evt.task_id as usize;
                t.freertos
                    .tasks
                    .get_mut_or_create(task_id)
                    .priority
                    .push(ts, evt.priority);
            }

            FreeRTOSEvtKind::TaskPriorityDisinherit(evt) => {
                let task_id = evt.task_id as usize;
                t.freertos
                    .tasks
                    .get_mut_or_create(task_id)
                    .priority
                    .push(ts, evt.priority);
            }

            FreeRTOSEvtKind::TaskCreated(evt) => {
                let task_id = evt.task_id as usize;
                t.freertos.tasks.ensure_exists(task_id);
            }

            FreeRTOSEvtKind::TaskDeleted(evt) => {
                let task_id = evt.task_id as usize;
                let current_task_id = t.core(core_id).freertos.current_task_id;
                let task = t.freertos.tasks.get_mut_or_create(task_id);
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

            FreeRTOSEvtKind::QueueCreated(evt) => {
                let queue_id = evt.queue_id as usize;
                t.freertos.queues.ensure_exists(queue_id);
            }

            FreeRTOSEvtKind::QueueSend(evt) => {
                let current_task = t.core(core_id).freertos.current_task_id;
                let queue_id = evt.queue_id as usize;
                let queue = t.freertos.queues.get_mut_or_create(queue_id);
                queue.state.push(
                    ts,
                    QueueState {
                        fill: evt.len_after,
                        by_task: current_task,
                    },
                );
            }

            FreeRTOSEvtKind::QueueSendFromIsr(evt) => {
                let queue_id = evt.queue_id as usize;
                let queue = t.freertos.queues.get_mut_or_create(queue_id);
                queue.state.push(
                    ts,
                    QueueState {
                        fill: evt.len_after,
                        by_task: None,
                    },
                );
            }

            FreeRTOSEvtKind::QueueOverwrite(evt) => {
                let current_task = t.core(core_id).freertos.current_task_id;
                let queue_id = evt.queue_id as usize;
                let queue = t.freertos.queues.get_mut_or_create(queue_id);
                queue.state.push(
                    ts,
                    QueueState {
                        fill: evt.len_after,
                        by_task: current_task,
                    },
                );
            }

            FreeRTOSEvtKind::QueueOverwriteFromIsr(evt) => {
                let queue_id = evt.queue_id as usize;
                let queue = t.freertos.queues.get_mut_or_create(queue_id);
                queue.state.push(
                    ts,
                    QueueState {
                        fill: evt.len_after,
                        by_task: None,
                    },
                );
            }

            FreeRTOSEvtKind::QueueReceive(evt) => {
                let current_task = t.core(core_id).freertos.current_task_id;
                let queue_id = evt.queue_id as usize;
                let queue = t.freertos.queues.get_mut_or_create(queue_id);
                queue.state.push(
                    ts,
                    QueueState {
                        fill: evt.len_after,
                        by_task: current_task,
                    },
                );
            }

            FreeRTOSEvtKind::QueueReceiveFromIsr(evt) => {
                let current_task = t.core(core_id).freertos.current_task_id;
                let queue_id = evt.queue_id as usize;
                let queue = t.freertos.queues.get_mut_or_create(queue_id);
                queue.state.push(
                    ts,
                    QueueState {
                        fill: evt.len_after,
                        by_task: current_task,
                    },
                );
            }

            FreeRTOSEvtKind::QueueReset(evt) => {
                let current_task = t.core(core_id).freertos.current_task_id;
                let queue_id = evt.queue_id as usize;
                let queue = t.freertos.queues.get_mut_or_create(queue_id);
                queue.state.push(
                    ts,
                    QueueState {
                        fill: 0,
                        by_task: current_task,
                    },
                );
            }

            FreeRTOSEvtKind::QueueCurLength(evt) => {
                let current_task = t.core(core_id).freertos.current_task_id;
                let queue_id = evt.queue_id as usize;
                let queue = t.freertos.queues.get_mut_or_create(queue_id);
                queue.state.push(
                    ts,
                    QueueState {
                        fill: evt.length,
                        by_task: current_task,
                    },
                );
            }

            FreeRTOSEvtKind::CurtaskBlockOnQueuePeek(evt) => {
                let queue_id = evt.queue_id as usize;
                t.freertos.queues.ensure_exists(queue_id);
                if let Some(current_task_id) = t.core(core_id).freertos.current_task_id {
                    t.freertos
                        .tasks
                        .get_mut_or_create(current_task_id)
                        .state_when_switched_out = TaskState::Blocked(TaskBlockingReason::QueuePeek { queue_id })
                } else {
                    warn!("[{ts:012}] Received current task event while current task is not known ({:?}).", evt);
                    t.error_evts.push(ts, TraceErrMarker::no_current_task(core_id));
                }
            }

            FreeRTOSEvtKind::CurtaskBlockOnQueueSend(evt) => {
                let queue_id = evt.queue_id as usize;
                t.freertos.queues.ensure_exists(queue_id);
                if let Some(current_task_id) = t.core(core_id).freertos.current_task_id {
                    t.freertos
                        .tasks
                        .get_mut_or_create(current_task_id)
                        .state_when_switched_out = TaskState::Blocked(TaskBlockingReason::QueueSend { queue_id })
                } else {
                    warn!("[{ts:012}] Received current task event while current task is not known ({:?}).", evt);
                    t.error_evts.push(ts, TraceErrMarker::no_current_task(core_id));
                }
            }

            FreeRTOSEvtKind::CurtaskBlockOnQueueReceive(evt) => {
                let queue_id = evt.queue_id as usize;
                t.freertos.queues.ensure_exists(queue_id);

                if let Some(current_task_id) = t.core(core_id).freertos.current_task_id {
                    t.freertos
                        .tasks
                        .get_mut_or_create(current_task_id)
                        .state_when_switched_out = TaskState::Blocked(TaskBlockingReason::QueueReceive { queue_id })
                } else {
                    warn!("[{ts:012}] Received current task event while current task is not known ({:?}).", evt);
                    t.error_evts.push(ts, TraceErrMarker::no_current_task(core_id));
                }
            }

            FreeRTOSEvtKind::TaskEvtmarker(evt) => {
                let evtmarker_id = evt.evtmarker_id as usize;
                if let Some(current_task_id) = t.core(core_id).freertos.current_task_id {
                    let evtmarker = t
                        .freertos
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

            FreeRTOSEvtKind::TaskEvtmarkerBegin(evt) => {
                let evtmarker_id = evt.evtmarker_id as usize;
                if let Some(current_task_id) = t.core(core_id).freertos.current_task_id {
                    let evtmarker = t
                        .freertos
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

            FreeRTOSEvtKind::TaskEvtmarkerEnd(evt) => {
                let evtmarker_id = evt.evtmarker_id as usize;
                if let Some(current_task_id) = t.core(core_id).freertos.current_task_id {
                    let evtmarker = t
                        .freertos
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

            FreeRTOSEvtKind::TaskValmarker(evt) => {
                let valmarker_id = evt.valmarker_id as usize;
                if let Some(current_task_id) = t.core(core_id).freertos.current_task_id {
                    let valmarker = t
                        .freertos
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

    pub(crate) fn convert_freertos_metadata_evt(&self, t: &mut Trace, _core_id: usize, e: &FreeRTOSMetadataEvt) {
        match e {
            FreeRTOSMetadataEvt::TaskName(evt) => {
                let task_id = evt.task_id as usize;
                let task = t.freertos.tasks.get_mut_or_create(task_id);
                if let Some(previous_name) = &mut task.name {
                    if *previous_name != evt.name {
                        warn!(
                            "[--METADATA--] Overwriting task #{task_id} name from '{previous_name}' to '{}'.",
                            evt.name
                        );
                    }
                }
                task.name = Some(evt.name.clone());
            }

            FreeRTOSMetadataEvt::TaskIsIdleTask(evt) => {
                let task_id = evt.task_id as usize;
                let task = t.freertos.tasks.get_mut_or_create(task_id);
                let new_kind = TaskKind::Idle {
                    core_id: evt.core_id as usize,
                };
                if !matches!(task.kind, TaskKind::Normal) && new_kind != task.kind {
                    warn!("[--METADATA--] Overwriting task #{task_id} type from '{}' to '{}'.", task.kind, new_kind);
                }
                task.kind = new_kind;
            }

            FreeRTOSMetadataEvt::TaskIsTimerTask(evt) => {
                let task_id = evt.task_id as usize;
                let task = t.freertos.tasks.get_mut_or_create(task_id);
                let new_kind = TaskKind::TimerSvc;
                if !matches!(task.kind, TaskKind::Normal) && new_kind != task.kind {
                    warn!("[--METADATA--] Overwriting task #{task_id} type from '{}' to '{}'.", task.kind, new_kind);
                }
                task.kind = new_kind;
            }

            FreeRTOSMetadataEvt::QueueName(evt) => {
                let queue_id = evt.queue_id as usize;
                let queue = t.freertos.queues.get_mut_or_create(queue_id);
                if let Some(previous_name) = &mut queue.name {
                    if *previous_name != evt.name {
                        warn!(
                            "[--METADATA--] Overwriting queue #{queue_id} name from '{previous_name}' to '{}'.",
                            evt.name
                        );
                    }
                }
                queue.name = Some(evt.name.clone());
            }

            FreeRTOSMetadataEvt::QueueKind(evt) => {
                let queue_id = evt.queue_id as usize;
                let queue = t.freertos.queues.get_mut_or_create(queue_id);
                let new_kind: QueueKind = evt.kind.into();

                if queue.kind == new_kind && !matches!(new_kind, QueueKind::Queue) {
                    warn!("[--METADATA--] Overwriting queue #{queue_id} type from '{}' to '{}'.", queue.kind, new_kind);
                }
                queue.kind = new_kind;
            }

            FreeRTOSMetadataEvt::TaskEvtmarkerName(evt) => {
                let task_id = evt.task_id as usize;
                let evtmarker_id = evt.evtmarker_id as usize;
                let evtmarker = t
                    .freertos
                    .tasks
                    .get_mut_or_create(task_id)
                    .user_evt_markers
                    .get_mut_or_create(evtmarker_id);
                if let Some(previous_name) = &evtmarker.name {
                    if *previous_name != evt.name {
                        warn!(
                            "[--METADATA--] Overwriting Task #{} Marker #{evtmarker_id} name from '{previous_name}' to '{}'.",
                            task_id,
                            evt.name
                        );
                    }
                }
                evtmarker.name = Some(evt.name.clone());
            }

            FreeRTOSMetadataEvt::TaskValmarkerName(evt) => {
                let task_id = evt.task_id as usize;
                let valmarker_id = evt.valmarker_id as usize;
                let valmarker = t
                    .freertos
                    .tasks
                    .get_mut_or_create(task_id)
                    .user_val_markers
                    .get_mut_or_create(valmarker_id);
                if let Some(previous_name) = &valmarker.name {
                    if *previous_name != evt.name {
                        warn!(
                            "[--METADATA--] Overwriting Value Marker #{valmarker_id} name from '{previous_name}' to '{}'.",
                            evt.name
                        );
                    }
                }
                valmarker.name = Some(evt.name.clone());
            }
        }
    }
}
