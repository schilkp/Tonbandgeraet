use lazy_static::lazy_static;
use regex::Regex;
use std::{fmt::Display, path::PathBuf, str::FromStr};

use anyhow::anyhow;
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
    #[arg(short, long, default_value = "bin")]
    pub format: InputFormat,

    #[arg(short, long, default_value = "1")]
    pub core_count: usize,
    
    #[arg(short, long)]
    pub output: PathBuf,

    // #[arg(long, action = clap::ArgAction::SetTrue)]
    // pub open: bool,
    // 
    // #[arg(long, action = clap::ArgAction::SetTrue)]
    // pub link: bool,

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

        crate::convert::convert(self)
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
