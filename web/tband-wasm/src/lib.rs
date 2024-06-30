mod utils;

use log::{info, Level, Log};
use serde::Serialize;
use utils::set_panic_hook;
use wasm_bindgen::prelude::*;

use tband_conv::{convert::TraceConverter, decode::evts};
use web_sys::console;

#[wasm_bindgen]
#[derive(Debug, Clone)]
pub struct TraceData {
    pub core_id: u32,
    #[wasm_bindgen(getter_with_clone)]
    pub data: Vec<u8>,
}

#[wasm_bindgen]
impl TraceData {
    pub fn new(core_id: u32, data: Vec<u8>) -> Self {
        TraceData { core_id, data }
    }
}

#[wasm_bindgen]
pub enum TraceMode {
    BareMetal,
    FreeRTOS,
}

#[wasm_bindgen]
pub fn convert(
    core_count: usize,
    data: Vec<TraceData>,
    mode: TraceMode,
) -> Result<Vec<u8>, String> {
    let mode = match mode {
        TraceMode::BareMetal => evts::TraceMode::Base,
        TraceMode::FreeRTOS => evts::TraceMode::FreeRTOS,
    };

    let mut tr = TraceConverter::new(core_count, mode).map_err(|x| x.to_string())?;
    for d in &data {
        info!("Decoding trace for core #{}...", d.core_id);
        tr.add_binary_to_core(&d.data, d.core_id)
            .map_err(|x| x.to_string())?;
    }
    let trace = tr.convert().map_err(|x| x.to_string())?;
    Ok(trace.generate_perfetto_trace())
}

// =======================================================================================

#[wasm_bindgen]
struct JsLog {}

impl Log for JsLog {
    fn enabled(&self, _metadata: &log::Metadata) -> bool {
        true
    }

    fn log(&self, record: &log::Record) {
        to_log_evt(&record.args().to_string(), record.level());
        let msg_console = format!("TBAND: [{}] - {}", record.level(), record.args());
        to_console_log(&msg_console, record.level());
    }

    fn flush(&self) {}
}

static LOGGER: JsLog = JsLog {};

#[derive(Debug, Clone, Serialize)]
pub enum LogMsgLevel {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
}

#[derive(Serialize)]
pub struct LogMsg {
    pub msg: String,
    pub lvl: LogMsgLevel,
}

impl LogMsg {
    pub fn new(lvl: Level, msg: &str) -> Self {
        let lvl = match lvl {
            Level::Error => LogMsgLevel::Error,
            Level::Warn => LogMsgLevel::Warn,
            Level::Info => LogMsgLevel::Info,
            Level::Debug => LogMsgLevel::Debug,
            Level::Trace => LogMsgLevel::Trace,
        };
        LogMsg {
            lvl,
            msg: msg.to_string(),
        }
    }
}

pub fn to_log_evt(msg: &str, level: Level) {
    let Some(window) = web_sys::window() else {
        to_console_log("TBAND LOG ERR: could not get window", Level::Error);
        return;
    };
    let Some(document) = window.document() else {
        to_console_log("TBAND LOG ERR: could not get document", Level::Error);
        return;
    };
    let Ok(new_event) = web_sys::CustomEvent::new("tband_log") else {
        to_console_log("TBAND LOG ERR: not create event", Level::Error);
        return;
    };
    let msg = LogMsg::new(level, msg);
    new_event.init_custom_event_with_can_bubble_and_cancelable_and_detail(
        "tband_log",
        false,
        false,
        &serde_wasm_bindgen::to_value(&msg).unwrap(),
    );
    match document.dispatch_event(&new_event) {
        Ok(_) => (),
        Err(e) => to_console_log(
            &format!("TBAND LOG ERR: Failed to send log event: {e:?}"),
            Level::Warn,
        ),
    }
}

pub fn to_console_log(msg: &str, level: Level) {
    match level {
        log::Level::Error => console::error_1(&JsValue::from_str(msg)),
        log::Level::Warn => console::warn_1(&JsValue::from_str(msg)),
        log::Level::Info => console::info_1(&JsValue::from_str(msg)),
        log::Level::Debug => console::debug_1(&JsValue::from_str(msg)),
        log::Level::Trace => console::trace_1(&JsValue::from_str(msg)),
    }
}

#[wasm_bindgen]
pub fn setup_log() {
    set_panic_hook();
    log::set_logger(&LOGGER)
        .map(|()| log::set_max_level(log::LevelFilter::Info))
        .expect("Failed to setup logger");
}
