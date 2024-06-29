mod convert;
mod generate_perfetto;

use std::fmt::Display;

use crate::{decode::evts, NewWithId, ObjectMap, Timeseries, UserEvtMarkerTrace, UserValMarkerTrace};

// == Core =====================================================================

pub struct FreeRTOSCoreTrace {
    // conversion state:
    pub current_task_id: Option<usize>,
}

impl FreeRTOSCoreTrace {
    pub(crate) fn new() -> FreeRTOSCoreTrace {
        FreeRTOSCoreTrace { current_task_id: None }
    }
}

// == Task =====================================================================

#[derive(Debug, Clone)]
pub enum TaskBlockingReason {
    Delay { ticks: u32 },
    DelayUntil { time_to_wake: u32 },
    QueuePeek { queue_id: usize },
    QueueSend { queue_id: usize },
    QueueReceive { queue_id: usize },
}

impl TaskBlockingReason {
    pub fn rich_name(&self, t: &FreeRTOSTrace) -> String {
        match self {
            TaskBlockingReason::Delay { ticks } => format!("Delay for {ticks} ticks"),
            TaskBlockingReason::DelayUntil { time_to_wake } => format!("Delay until tick {time_to_wake}"),
            TaskBlockingReason::QueuePeek { queue_id } => format!("Receive {}", t.name_queue(*queue_id)),
            TaskBlockingReason::QueueSend { queue_id } => format!("Send to {}", t.name_queue(*queue_id)),
            TaskBlockingReason::QueueReceive { queue_id } => format!("Receive from {}", t.name_queue(*queue_id)),
        }
    }
}

#[derive(Debug, Clone)]
pub enum TaskState {
    Running { core_id: usize },
    Ready,
    Blocked(TaskBlockingReason),
    Suspended { by_task_id: Option<usize> },
    Deleted { by_task_id: Option<usize> },
}

impl TaskState {
    pub fn rich_name(&self, t: &FreeRTOSTrace) -> String {
        match self {
            TaskState::Running { core_id } => format!("Running (core {core_id})"),
            TaskState::Ready => String::from("Ready"),
            TaskState::Blocked(reason) => format!("Blocked ({})", reason.rich_name(t)),
            TaskState::Suspended {
                by_task_id: Some(by_task_id),
            } => format!("Suspended (by {})", t.name_task(*by_task_id)),
            TaskState::Suspended { by_task_id: None } => String::from("Suspended"),
            TaskState::Deleted {
                by_task_id: Some(by_task_id),
            } => format!("Deleted (by {})", t.name_task(*by_task_id)),
            TaskState::Deleted { by_task_id: None } => String::from("Deleted"),
        }
    }
}

#[derive(Debug, Clone, PartialEq)]
pub enum TaskKind {
    Normal,
    Idle { core_id: usize },
    TimerSvc,
}

impl Display for TaskKind {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            TaskKind::Normal => write!(f, "normal"),
            TaskKind::Idle { core_id } => write!(f, "idle (core {core_id}"),
            TaskKind::TimerSvc => write!(f, "timer svc"),
        }
    }
}

pub struct TaskTrace {
    pub id: usize,
    pub name: Option<String>,

    pub kind: TaskKind,
    pub state: Timeseries<TaskState>,
    pub priority: Timeseries<u32>,

    // User markers:
    pub user_evt_markers: ObjectMap<UserEvtMarkerTrace>,
    pub user_val_markers: ObjectMap<UserValMarkerTrace>,

    // Conversion state:
    state_when_switched_out: TaskState,
}

impl NewWithId for TaskTrace {
    fn new(id: usize) -> Self {
        Self {
            id,
            name: None,
            kind: TaskKind::Normal,
            state: Timeseries::new(),
            priority: Timeseries::new(),
            user_evt_markers: ObjectMap::new(),
            user_val_markers: ObjectMap::new(),
            state_when_switched_out: TaskState::Ready,
        }
    }
}

impl TaskTrace {
    fn is_running(&self) -> bool {
        self.state
            .0
            .last()
            .map(|x| matches!(x.inner, TaskState::Running { .. }))
            .unwrap_or(false)
    }

    fn name_user_evtmarker(&self, id: usize) -> String {
        let task_name = self.name();
        if let Some(marker) = self.user_evt_markers.get(id) {
            if let Some(name) = &marker.name {
                return format!("{task_name} Marker {name} (#{id})");
            }
        }
        format!("{task_name} Marker #{id}")
    }

    fn name_user_valmarker(&self, id: usize) -> String {
        let task_name = self.name();
        if let Some(marker) = self.user_val_markers.get(id) {
            if let Some(name) = &marker.name {
                return format!("{task_name} Value {name} (#{id})");
            }
        }
        format!("{task_name} Value #{id}")
    }

    fn name(&self) -> String {
        let id = self.id;
        if let Some(name) = &self.name {
            return format!("Task {name} (#{id})");
        }
        format!("Task #{id}")
    }
}

// == Queue ====================================================================

#[derive(Debug, Clone, PartialEq)]
pub enum QueueKind {
    Queue,
    CountingSemphr,
    BinarySemphr,
    Mutex,
    RecursiveMutex,
    QueueSet,
}

impl QueueKind {
    fn is_mutex(&self) -> bool {
        match self {
            QueueKind::Queue => false,
            QueueKind::CountingSemphr => false,
            QueueKind::BinarySemphr => false,
            QueueKind::Mutex => true,
            QueueKind::RecursiveMutex => true,
            QueueKind::QueueSet => false,
        }
    }
}

impl Display for QueueKind {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            QueueKind::Queue => write!(f, "Queue"),
            QueueKind::CountingSemphr => write!(f, "Counting Semaphore"),
            QueueKind::BinarySemphr => write!(f, "Binary Semaphore"),
            QueueKind::Mutex => write!(f, "Mutex"),
            QueueKind::RecursiveMutex => write!(f, "Recursive Mutex"),
            QueueKind::QueueSet => write!(f, "Queue Set"),
        }
    }
}

impl From<evts::FrQueueKind> for QueueKind {
    fn from(value: evts::FrQueueKind) -> Self {
        match value {
            evts::FrQueueKind::FrqkQueue => Self::Queue,
            evts::FrQueueKind::FrqkCountingSemphr => Self::CountingSemphr,
            evts::FrQueueKind::FrqkBinarySemphr => Self::BinarySemphr,
            evts::FrQueueKind::FrqkMutex => Self::Mutex,
            evts::FrQueueKind::FrqkRecursiveMutex => Self::RecursiveMutex,
            evts::FrQueueKind::FrqkQueueSet => Self::QueueSet,
        }
    }
}

pub struct QueueState {
    fill: u32,
    // Track the task that put the queue into this state. If this queue is a
    // mutex, this is used to track which task currently holds it.
    by_task: Option<usize>,
}

pub struct QueueTrace {
    pub id: usize,
    pub name: Option<String>,
    pub kind: QueueKind,
    pub state: Timeseries<QueueState>,
}

impl NewWithId for QueueTrace {
    fn new(id: usize) -> Self {
        Self {
            id,
            name: None,
            kind: QueueKind::Queue,
            state: Timeseries::new(),
        }
    }
}

// == Trace ====================================================================

pub struct FreeRTOSTrace {
    // Tasks:
    pub tasks: ObjectMap<TaskTrace>,

    // Resources:
    pub queues: ObjectMap<QueueTrace>,
    // pub stream_buffers: ..
}

impl FreeRTOSTrace {
    pub(crate) fn new() -> Self {
        Self {
            tasks: ObjectMap::new(),
            queues: ObjectMap::new(),
        }
    }

    pub(crate) fn name_task(&self, id: usize) -> String {
        if let Some(task) = self.tasks.get(id) {
            if let Some(name) = &task.name {
                return format!("Task {name} (#{id})");
            }
        }
        format!("Task #{id}")
    }

    pub(crate) fn name_queue(&self, id: usize) -> String {
        if let Some(queue) = self.queues.get(id) {
            if let Some(name) = &queue.name {
                return format!("{} {name} (#{id})", queue.kind);
            } else {
                return format!("{} #{id}", queue.kind);
            }
        }
        format!("Queue #{id}")
    }
}
