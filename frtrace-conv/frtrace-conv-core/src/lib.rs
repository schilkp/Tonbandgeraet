pub mod convert;
pub mod decode;

use std::{collections::BTreeMap, fmt::Display, rc::Rc};

use decode::evts::{self, RawEvt, RawInvalidEvt};

// == Timestamped Value ========================================================

pub struct Ts<T> {
    pub ts: u64,
    pub inner: T,
}

impl<T> Ts<T> {
    pub fn new(ts: u64, i: T) -> Ts<T> {
        Ts { ts, inner: i }
    }
}

pub struct Timeseries<T>(pub Vec<Ts<T>>);

impl<T> Default for Timeseries<T> {
    fn default() -> Self {
        Self::new()
    }
}

impl<T> Timeseries<T> {
    pub fn new() -> Self {
        Timeseries(vec![])
    }

    pub fn push(&mut self, ts: u64, t: T) {
        self.0.push(Ts::new(ts, t));
    }
}

// == Trace Event Marker =======================================================

#[derive(Debug)]
pub struct TraceEvtMarker(pub RawEvt);

#[derive(Debug)]
pub enum ErrMarkerKind {
    DroppedEvts { dropped: u32, total: u32 },
    NoCurrentTask,
    InvalidEvent(Option<Rc<anyhow::Error>>),
}

#[derive(Debug)]
pub struct TraceErrMarker {
    pub core_id: Option<usize>,
    pub kind: ErrMarkerKind,
}

impl TraceErrMarker {
    fn dropped(dropped: u32, total: u32, core_id: usize) -> Self {
        TraceErrMarker {
            core_id: Some(core_id),
            kind: ErrMarkerKind::DroppedEvts { dropped, total },
        }
    }

    fn no_current_task(core_id: usize) -> Self {
        TraceErrMarker {
            core_id: Some(core_id),
            kind: ErrMarkerKind::NoCurrentTask,
        }
    }

    fn invalid(core_id: usize, e: &RawInvalidEvt) -> Self {
        TraceErrMarker {
            core_id: Some(core_id),
            kind: ErrMarkerKind::InvalidEvent(e.err.clone()),
        }
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
    pub fn rich_name(&self, t: &Trace) -> String {
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
    pub fn rich_name(&self, t: &Trace) -> String {
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

    // Conversion state:
    state_when_switched_out: TaskState,
}

impl TaskTrace {
    fn new(id: usize) -> Self {
        Self {
            id,
            name: None,
            kind: TaskKind::Normal,
            state: Timeseries::new(),
            priority: Timeseries::new(),
            state_when_switched_out: TaskState::Ready,
        }
    }

    fn is_running(&self) -> bool {
        self.state
            .0
            .last()
            .map(|x| matches!(x.inner, TaskState::Running { .. }))
            .unwrap_or(false)
    }
}

// == Core =====================================================================

pub enum ISRState {
    Active,
    NotActive,
}

pub struct ISRTrace {
    pub id: usize,
    pub name: Option<String>,
    pub state: Timeseries<ISRState>,

    // Conversion state:
    current_state: ISRState,
}

pub struct CoreTrace {
    pub id: usize,
    pub isrs: BTreeMap<usize, ISRTrace>,
    pub evts: Timeseries<TraceEvtMarker>,

    // Conversion state:
    current_task_id: Option<usize>,
}

impl CoreTrace {
    fn new(id: usize) -> Self {
        CoreTrace {
            id,
            isrs: BTreeMap::new(),
            evts: Timeseries::new(),
            current_task_id: None,
        }
    }

    fn ensure_isr_exists(&mut self, id: usize) {
        self.isrs.entry(id).or_insert_with(|| ISRTrace {
            id,
            name: None,
            state: Timeseries::new(),

            current_state: ISRState::NotActive,
        });
    }
}

impl<'a> CoreTrace {
    fn isr(&'a mut self, isr_id: usize) -> &'a ISRTrace {
        self.ensure_isr_exists(isr_id);
        self.isrs.get(&isr_id).unwrap()
    }

    fn isr_mut(&'a mut self, isr_id: usize) -> &'a mut ISRTrace {
        self.ensure_isr_exists(isr_id);
        self.isrs.get_mut(&isr_id).unwrap()
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

impl From<evts::QueueKind> for QueueKind {
    fn from(value: evts::QueueKind) -> Self {
        match value {
            evts::QueueKind::QkQueue => Self::Queue,
            evts::QueueKind::QkCountingSemphr => Self::CountingSemphr,
            evts::QueueKind::QkBinarySemphr => Self::BinarySemphr,
            evts::QueueKind::QkMutex => Self::Mutex,
            evts::QueueKind::QkRecursiveMutex => Self::RecursiveMutex,
            evts::QueueKind::QkQueueSet => Self::QueueSet,
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

impl QueueTrace {
    pub fn new(id: usize) -> Self {
        Self {
            id,
            name: None,
            kind: QueueKind::Queue,
            state: Timeseries::new(),
        }
    }
}

// == Trace ====================================================================

pub struct Trace {
    // Metadata:
    pub core_count: usize,
    pub ts_resolution_ns: Option<u64>,

    // Errors:
    pub error_evts: Timeseries<TraceErrMarker>,

    // Cores:
    pub cores: BTreeMap<usize, CoreTrace>,

    // Tasks:
    pub tasks: BTreeMap<usize, TaskTrace>,

    // Resources:
    pub queues: BTreeMap<usize, QueueTrace>,
    // pub stream_buffers: ..

    // Conversion state:
    dropped_evt_cnt: u32,
}

impl Trace {
    fn new(core_count: usize) -> Self {
        let mut s = Self {
            core_count,
            ts_resolution_ns: None,
            error_evts: Timeseries::new(),
            cores: BTreeMap::new(),
            tasks: BTreeMap::new(),
            queues: BTreeMap::new(),
            dropped_evt_cnt: 0,
        };

        for i in 0..core_count {
            s.cores.insert(i, CoreTrace::new(i));
        }

        s
    }

    fn ensure_task_exists(&mut self, id: usize) {
        self.tasks.entry(id).or_insert_with(|| TaskTrace::new(id));
    }

    fn ensure_queue_exists(&mut self, id: usize) {
        self.queues.entry(id).or_insert_with(|| QueueTrace::new(id));
    }

    fn convert_ts(&self, ts: u64) -> u64 {
        let ts_resolution_ns = self.ts_resolution_ns.unwrap_or(1);
        ts * ts_resolution_ns
    }

    fn name_task(&self, id: usize) -> String {
        if let Some(task) = self.tasks.get(&id) {
            if let Some(name) = &task.name {
                return format!("Task {name} (#{id})");
            }
        }
        return format!("Task #{id}");
    }

    fn name_isr(&self, core_id: usize, id: usize) -> String {
        if let Some(isr) = self.core(core_id).isrs.get(&id) {
            if let Some(name) = &isr.name {
                return format!("ISR {name} (#{id})");
            }
        }
        return format!("ISR #{id}");
    }

    fn name_queue(&self, id: usize) -> String {
        if let Some(queue) = self.queues.get(&id) {
            if let Some(name) = &queue.name {
                return format!("{} {name} (#{id})", queue.kind);
            } else {
                return format!("{} #{id}", queue.kind);
            }
        }
        return format!("Queue #{id}");
    }
}

impl<'a> Trace {
    fn task(&'a mut self, task_id: usize) -> &'a TaskTrace {
        self.ensure_task_exists(task_id);
        self.tasks.get(&task_id).unwrap()
    }

    fn task_mut(&'a mut self, task_id: usize) -> &'a mut TaskTrace {
        self.ensure_task_exists(task_id);
        self.tasks.get_mut(&task_id).unwrap()
    }

    fn core(&'a self, core_id: usize) -> &'a CoreTrace {
        self.cores.get(&core_id).unwrap()
    }

    fn core_mut(&'a mut self, core_id: usize) -> &'a mut CoreTrace {
        self.cores.get_mut(&core_id).unwrap()
    }

    fn queue(&'a mut self, queue_id: usize) -> &'a QueueTrace {
        self.ensure_queue_exists(queue_id);
        self.queues.get(&queue_id).unwrap()
    }

    fn queue_mut(&'a mut self, queue_id: usize) -> &'a mut QueueTrace {
        self.ensure_queue_exists(queue_id);
        self.queues.get_mut(&queue_id).unwrap()
    }
}
