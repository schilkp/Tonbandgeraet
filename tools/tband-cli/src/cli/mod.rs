mod cmd_completion;
mod cmd_convert;
mod cmd_dump;
mod cmd_serve;

use clap::Parser;

#[derive(Parser, Debug)]
#[command(version, about)]
#[command(name = "tband-cli")]
pub struct Cli {
    #[arg(long, short, action = clap::ArgAction::Count)]
    pub verbose: u8,

    /// Exit with a non-zero status if any warning is logged
    ///
    /// Causes the tool to fail even if the command otherwise completes successfully.
    /// Mostly intended for development/testing.
    #[arg(long = "Werror")]
    pub fail_on_warning: bool,

    #[command(subcommand)]
    pub cmd: CliCmd,
}

#[derive(Parser, Debug)]
pub enum CliCmd {
    Conv(cmd_convert::Cmd),
    Serve(cmd_serve::Cmd),
    Completion(cmd_completion::Cmd),
    Dump(cmd_dump::Cmd),
}
