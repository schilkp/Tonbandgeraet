mod cmd_completion;
mod cmd_convert;
mod cmd_serve;

use clap::Parser;

#[derive(Parser, Debug)]
#[command(version, about)]
#[command(name = "frtrace-cli")]
pub struct Cli {
    #[arg(long, short, action = clap::ArgAction::Count)]
    pub verbose: u8,

    #[command(subcommand)]
    pub cmd: CliCmd,
}

#[derive(Parser, Debug)]
pub enum CliCmd {
    Conv(cmd_convert::Cmd),
    Serve(cmd_serve::Cmd),
    Completion(cmd_completion::Cmd),
}
