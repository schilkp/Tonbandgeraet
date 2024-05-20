mod cmd_completion;
pub(crate) mod cmd_convert;

use clap::Parser;

#[derive(Parser, Debug)]
#[command(version, about)]
#[command(name = "frtrace-conv")]
pub struct Cli {
    #[arg(long, short, action = clap::ArgAction::Count)]
    pub verbose: u8,

    #[command(subcommand)]
    pub cmd: CliCmd,
}

#[derive(Parser, Debug)]
pub enum CliCmd {
    Conv(cmd_convert::Cmd),
    Completion(cmd_completion::Cmd),
}
