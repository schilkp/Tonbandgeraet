use std::io;

use crate::cli::Cli;
use clap::{CommandFactory, Parser};

#[derive(Parser, Debug)]
#[command(about = "Print completion script for specified shell")]
pub struct Cmd {
    pub shell: clap_complete::Shell,
}

impl Cmd {
    pub fn run(&self) -> anyhow::Result<()> {
        clap_complete::generate(self.shell, &mut Cli::command(), "reginald", &mut io::stdout());
        Ok(())
    }
}
