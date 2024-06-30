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
use anyhow::{anyhow, Context};
use serde::{Serialize};

use crate::decode::{bytes_left, decode_s64, decode_string, decode_u32, decode_u64, decode_u8};

