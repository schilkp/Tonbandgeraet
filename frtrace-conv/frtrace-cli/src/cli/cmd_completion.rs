use std::io;

use crate::cli::Cli;
use clap::{CommandFactory, Parser};

#[derive(Parser, Debug)]
#[command(about = "Print completion script for specified shell")]
pub struct Cmd {
    /// Style of completion script to generate
    pub shell: clap_complete::Shell,
}

impl Cmd {
    pub fn run(&self) -> anyhow::Result<()> {
        clap_complete::generate(self.shell, &mut Cli::command(), "frtrace-cli", &mut io::stdout());
        Ok(())
    }
}
