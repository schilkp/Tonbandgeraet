use std::path::PathBuf;

use clap::Parser;

use crate::open::{open_trace, serve_trace};

#[derive(Parser, Debug)]
#[command(about = "Serve trace file for perfetto.")]
pub struct Cmd {
    #[arg(short, long)]
    pub input: PathBuf,

    #[arg(long, action = clap::ArgAction::SetTrue)]
    pub open: bool,
}

impl Cmd {
    pub fn run(&self) -> anyhow::Result<()> {
        let file = std::fs::read(&self.input)?;

        if self.open {
            open_trace(&file)?;
        } else {
            serve_trace(&file)?;
        }

        Ok(())
    }
}
