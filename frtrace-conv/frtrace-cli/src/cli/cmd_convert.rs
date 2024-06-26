use crate::open::{open_trace, serve_trace};
use anyhow::anyhow;
use frtrace_conv_core::convert::TraceConverter;
use lazy_static::lazy_static;
use log::info;
use regex::Regex;
use std::{fmt::Display, path::PathBuf, str::FromStr};

use clap::{Parser, ValueEnum};

#[derive(ValueEnum, Clone, Debug, Copy)]
pub enum InputFormat {
    Hex,
    Bin,
}

impl Display for InputFormat {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            InputFormat::Hex => write!(f, "hex"),
            InputFormat::Bin => write!(f, "binary"),
        }
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct InputFile {
    pub file: PathBuf,
    pub core_id: Option<u32>,
}

lazy_static! {
    static ref PATH_CORE_ID_RE: Regex = Regex::new(r"(?<path>.*)@(?<core_id>\d+)$").unwrap();
}

impl FromStr for InputFile {
    type Err = anyhow::Error;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        match PATH_CORE_ID_RE.captures(s) {
            Some(caps) => {
                let path = &caps[1];
                let core_id: u32 = caps[2].parse().unwrap();
                Ok(Self {
                    file: PathBuf::from_str(path).unwrap(),
                    core_id: Some(core_id),
                })
            }
            None => Ok(Self {
                file: PathBuf::from_str(s).unwrap(),
                core_id: None,
            }),
        }
    }
}

#[derive(Parser, Debug)]
#[command(about = "Convert trace recording")]
pub struct Cmd {
    /// Input format
    #[arg(short, long, default_value = "bin")]
    pub format: InputFormat,

    /// Number of cores of target
    #[arg(short, long, default_value = "1")]
    pub core_count: usize,

    /// Location to store converted trace
    #[arg(short, long)]
    pub output: Option<PathBuf>,

    /// Open converted trace in perfetto
    #[arg(long, action = clap::ArgAction::SetTrue)]
    pub open: bool,

    /// Serve converted trace for perfetto
    #[arg(long, action = clap::ArgAction::SetTrue)]
    pub serve: bool,

    /// Input files with optional core id.
    ///
    /// For split multi-core recording, append core id to file name as such: filename@core_id
    #[arg(action = clap::ArgAction::Append)]
    pub input: Vec<InputFile>,
}

impl Cmd {
    pub fn run(self) -> anyhow::Result<()> {
        if self.core_count == 0 {
            return Err(anyhow!("Core count cannot be zero."));
        }

        if self.input.is_empty() {
            return Err(anyhow!("Require at least one input file."));
        }

        for file in &self.input {
            if let Some(core_id) = file.core_id {
                if core_id as usize >= self.core_count {
                    return Err(anyhow!(
                        "Core id {core_id} for file '{}' invalid for {}-core trace.",
                        file.file.to_string_lossy(),
                        self.core_count
                    ));
                }
            }
        }

        let mut tc = TraceConverter::new(self.core_count)?;

        for inp in self.input {
            info!("Opening {} file \"{}\"..", self.format, inp.file.to_string_lossy());
            let data = read_file(&inp.file, self.format)?;
            info!("Decoding events..");
            if let Some(core_id) = inp.core_id {
                info!("Adding events to core {core_id} trace sequence..");
                tc.add_binary_to_core(&data, core_id)?;
            } else {
                info!("Adding events to trace sequence..");
                tc.add_binary(&data)?;
            }
        }

        info!("Converting..");
        let trace = tc.convert()?;
        info!("Genertating Perfetto Trace..");
        let trace = trace.generate_perfetto_trace();
        info!("Conversion finished.");

        if let Some(output) = self.output {
            info!("Saving to '{}'.", output.to_string_lossy());
            std::fs::write(output.clone(), &trace)?;
        }

        if self.open {
            open_trace(&trace)?;
        }

        if self.serve {
            serve_trace(&trace)?;
        }

        Ok(())
    }
}

fn read_file(f: &PathBuf, format: InputFormat) -> anyhow::Result<Vec<u8>> {
    match format {
        InputFormat::Hex => {
            let mut bytes: Vec<u8> = Vec::with_capacity(2048);
            let content = std::fs::read_to_string(f)?;
            let mut string = String::new();
            for c in content.chars() {
                if c.is_whitespace() {
                    continue;
                }
                if !c.is_ascii_hexdigit() {
                    return Err(anyhow!("Invalid hex input file."));
                }

                string.push(c);
                if string.len() == 2 {
                    bytes.push(u8::from_str_radix(&string, 16)?);
                    string.clear();
                }
            }
            Ok(bytes)
        }
        InputFormat::Bin => Ok(std::fs::read(f)?),
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn input_file(file: &str, core_id: Option<u32>) -> InputFile {
        InputFile {
            file: PathBuf::from_str(file).unwrap(),
            core_id,
        }
    }

    #[test]
    fn input_file_parsing() {
        assert_eq!(InputFile::from_str("asdfasdfas").unwrap(), input_file("asdfasdfas", None));
        assert_eq!(InputFile::from_str("../../tes  t.hex").unwrap(), input_file("../../tes  t.hex", None));
        assert_eq!(InputFile::from_str("@1").unwrap(), input_file("", Some(1)));
        assert_eq!(InputFile::from_str("asdf@1").unwrap(), input_file("asdf", Some(1)));
        assert_eq!(InputFile::from_str("asdf@").unwrap(), input_file("asdf@", None));
        assert_eq!(InputFile::from_str("asdf@asdasda").unwrap(), input_file("asdf@asdasda", None));
        assert_eq!(InputFile::from_str("asdf@1@1").unwrap(), input_file("asdf@1", Some(1)));
    }
}
