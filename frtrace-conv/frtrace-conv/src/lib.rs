pub mod convert;
pub mod decode;

use std::{
    collections::{btree_map, BTreeMap},
    fmt::Display,
    rc::Rc,
};

use decode::evts::{self, RawEvt, RawInvalidEvt};

// == Object Registry ==========================================================

pub trait NewWithId {
    fn new(id: usize) -> Self;
}

pub struct ObjectMap<T>(pub BTreeMap<usize, T>)
where
    T: NewWithId;

impl<T> Default for ObjectMap<T>
where
    T: NewWithId,
 {
    fn default() -> Self {
        Self::new()
    }
}

impl<T> ObjectMap<T>
where
    T: NewWithId,
{
    pub fn new() -> Self {
        ObjectMap(BTreeMap::new())
    }

    pub fn get(&self, id: usize) -> Option<&T> {
        self.0.get(&id)
    }

    pub fn get_mut(&mut self, id: usize) -> Option<&mut T> {
        self.0.get_mut(&id)
    }

    pub fn get_or_create(&mut self, id: usize) -> &T {
        self.0.entry(id).or_insert_with(|| T::new(id))
    }

    pub fn get_mut_or_create(&mut self, id: usize) -> &mut T {
        self.0.entry(id).or_insert_with(|| T::new(id))
    }

    pub fn ensure_exists(&mut self, id: usize) {
        self.0.entry(id).or_insert_with(|| T::new(id));
    }
}

impl<'a, T> IntoIterator for &'a ObjectMap<T>
where
    T: NewWithId,
{
    type Item = (&'a usize, &'a T);
    type IntoIter = btree_map::Iter<'a, usize, T>;

    fn into_iter(self) -> Self::IntoIter {
        self.0.iter()
    }
}

impl<'a, T> IntoIterator for &'a mut ObjectMap<T>
where
    T: NewWithId,
{
    type Item = (&'a usize, &'a mut T);
    type IntoIter = btree_map::IterMut<'a, usize, T>;

    fn into_iter(self) -> Self::IntoIter {
        self.0.iter_mut()
    }
}

impl<T> IntoIterator for ObjectMap<T>
where
    T: NewWithId,
{
    type Item = (usize, T);
    type IntoIter = btree_map::IntoIter<usize, T>;

    fn into_iter(self) -> Self::IntoIter {
        self.0.into_iter()
    }
}

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

    // TODO: Trace evnt that?'
    // TODO: Refactor this in converter?
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

// == User Markers =============================================================

pub enum UserEvtMarker {
    Instant { msg: String },
    SliceBegin { msg: String },
    SliceEnd,
}

pub struct UserEvtMarkerTrace {
    #[allow(unused)]
    id: usize, 
    name: Option<String>,
    markers: Timeseries<UserEvtMarker>,
}

impl NewWithId for UserEvtMarkerTrace {
    fn new(id: usize) -> Self {
        Self {
            id,
            name: None,
            markers: Timeseries::new(),
        }
    }
}

pub struct UserValMarkerTrace {
    #[allow(unused)]
    id: usize,
    name: Option<String>,
    vals: Timeseries<i64>,
}

impl NewWithId for UserValMarkerTrace {
    fn new(id: usize) -> Self {
        Self {
            id,
            name: None,
            vals: Timeseries::new(),
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

impl NewWithId for ISRTrace {
    fn new(id: usize) -> Self {
        ISRTrace {
            id,
            name: None,
            state: Timeseries::new(),
            current_state: ISRState::NotActive,
        }
    }
}

pub struct CoreTrace {
    pub id: usize,
    pub isrs: ObjectMap<ISRTrace>,
    pub evts: Timeseries<TraceEvtMarker>,

    // Conversion state:
    current_task_id: Option<usize>,
}

impl NewWithId for CoreTrace {
    fn new(id: usize) -> Self {
        CoreTrace {
            id,
            isrs: ObjectMap::new(),
            evts: Timeseries::new(),
            current_task_id: None,
        }
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

pub struct Trace {
    // Metadata:
    pub core_count: usize,
    pub ts_resolution_ns: Option<u64>,

    // Errors:
    pub error_evts: Timeseries<TraceErrMarker>,

    // Cores:
    pub cores: BTreeMap<usize, CoreTrace>,

    // Tasks:
    pub tasks: ObjectMap<TaskTrace>,

    // Resources:
    pub queues: ObjectMap<QueueTrace>,
    // pub stream_buffers: ..

    // Global markers:
    pub user_evt_markers: ObjectMap<UserEvtMarkerTrace>,
    pub user_val_markers: ObjectMap<UserValMarkerTrace>,

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
            tasks: ObjectMap::new(),
            queues: ObjectMap::new(),
            user_evt_markers: ObjectMap::new(),
            user_val_markers: ObjectMap::new(),
            dropped_evt_cnt: 0,
        };

        for i in 0..core_count {
            s.cores.insert(i, CoreTrace::new(i));
        }

        s
    }

    fn convert_ts(&self, ts: u64) -> u64 {
        let ts_resolution_ns = self.ts_resolution_ns.unwrap_or(1);
        ts * ts_resolution_ns
    }

    fn name_task(&self, id: usize) -> String {
        if let Some(task) = self.tasks.get(id) {
            if let Some(name) = &task.name {
                return format!("Task {name} (#{id})");
            }
        }
        format!("Task #{id}")
    }

    fn name_isr(&self, core_id: usize, id: usize) -> String {
        if let Some(isr) = self.cores[&core_id].isrs.get(id) {
            if let Some(name) = &isr.name {
                return format!("ISR {name} (#{id})");
            }
        }
        format!("ISR #{id}")
    }

    fn name_queue(&self, id: usize) -> String {
        if let Some(queue) = self.queues.get(id) {
            if let Some(name) = &queue.name {
                return format!("{} {name} (#{id})", queue.kind);
            } else {
                return format!("{} #{id}", queue.kind);
            }
        }
        format!("Queue #{id}")
    }

    fn name_user_evtmarker(&self, id: usize) -> String {
        if let Some(marker) = self.user_evt_markers.get(id) {
            if let Some(name) = &marker.name {
                return format!("Marker {name} (#{id})");
            }
        }
        format!("Marker #{id}")
    }

    fn name_user_valmarker(&self, id: usize) -> String {
        if let Some(marker) = self.user_val_markers.get(id) {
            if let Some(name) = &marker.name {
                return format!("Value {name} (#{id})");
            }
        }
        format!("Value #{id}")
    }

    fn core(&self, id: usize) -> &CoreTrace {
        assert!(id < self.core_count);
        &self.cores[&id]
    }

    fn core_mut(&mut self, id: usize) -> &mut CoreTrace {
        self.cores.get_mut(&id).expect("Invalid core id")
    }
}
