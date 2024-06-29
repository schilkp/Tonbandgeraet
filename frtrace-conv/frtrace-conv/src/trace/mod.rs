pub mod base;
pub mod convert;
pub mod freertos;
pub mod generate_perfetto;

use std::{collections::BTreeMap, rc::Rc};

use crate::{
    decode::evts::{InvalidEvt, RawEvt, TraceMode},
    NewWithId, ObjectMap, Timeseries,
};

use self::freertos::{FreeRTOSCoreTrace, FreeRTOSTrace};

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

    fn invalid(core_id: usize, e: &InvalidEvt) -> Self {
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

    pub freertos: FreeRTOSCoreTrace,
}

impl NewWithId for CoreTrace {
    fn new(id: usize) -> Self {
        CoreTrace {
            id,
            isrs: ObjectMap::new(),
            evts: Timeseries::new(),
            freertos: FreeRTOSCoreTrace::new(),
        }
    }
}

// == Trace ====================================================================

pub struct Trace {
    // Cores:
    pub cores: BTreeMap<usize, CoreTrace>,
    pub mode: TraceMode,

    // Metadata:
    pub core_count: usize,
    pub ts_resolution_ns: Option<u64>,

    // Errors:
    pub error_evts: Timeseries<TraceErrMarker>,

    // Global markers:
    pub user_evt_markers: ObjectMap<UserEvtMarkerTrace>,
    pub user_val_markers: ObjectMap<UserValMarkerTrace>,

    // FreeRTOS trace:
    pub freertos: FreeRTOSTrace,

    // Conversion state:
    dropped_evt_cnt: u32,
}

impl Trace {
    fn new(core_count: usize, mode: TraceMode) -> Self {
        let mut s = Self {
            mode,
            core_count,
            ts_resolution_ns: None,
            error_evts: Timeseries::new(),
            cores: BTreeMap::new(),
            freertos: FreeRTOSTrace::new(),
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

    fn name_isr(&self, core_id: usize, id: usize) -> String {
        if let Some(isr) = self.cores[&core_id].isrs.get(id) {
            if let Some(name) = &isr.name {
                return format!("ISR {name} (#{id})");
            }
        }
        format!("ISR #{id}")
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
