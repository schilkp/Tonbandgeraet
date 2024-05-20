//! # Important:
//!
//! ```text
//!         ____   ___    _   _  ___ _____   _____ ____ ___ _____
//!        |  _ \ / _ \  | \ | |/ _ \_   _| | ____|  _ \_ _|_   _|
//!        | | | | | | | |  \| | | | || |   |  _| | | | | |  | |
//!        | |_| | |_| | | |\  | |_| || |   | |___| |_| | |  | |
//!        |____/ \___/  |_| \_|\___/ |_|   |_____|____/___| |_|
//!
//! ```
//!
//! This file is generated automatically. See `code_gen` folder in repo.
use std::rc::Rc;

use anyhow::{anyhow, Context};

use crate::decode::{bytes_left, decode_string, decode_u32, decode_u64, decode_u8};

#[derive(Debug, Clone)]
pub enum RawEvt {
    Metadata(RawMetadataEvt),
    Trace(RawTraceEvt),
    Invalid(RawInvalidEvt),
}

impl RawEvt {
    pub fn ts(&self) -> Option<u64> {
        match self {
            RawEvt::Metadata(_) => None,
            RawEvt::Trace(e) => Some(e.ts),
            RawEvt::Invalid(e) => e.ts,
        }
    }
}

#[derive(Debug, Clone)]
pub struct RawInvalidEvt {
    pub ts: Option<u64>,
    pub err: Option<Rc<anyhow::Error>>,
}

#[derive(Debug, Clone)]
pub struct RawTraceEvt {
    pub ts: u64,
    pub kind: RawTraceEvtKind,
}
