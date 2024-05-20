mod cli;
mod convert;

use std::io::Write;

use colored::*;

use log::{error, Level};
use std::process::ExitCode;

use clap::Parser;
use cli::{Cli, CliCmd};

fn level_str(l: Level) -> String {
    match l {
        Level::Error => "ERR".red().bold().to_string(),
        Level::Warn => "WRN".yellow().bold().to_string(),
        Level::Info => "INF".green().bold().to_string(),
        Level::Debug => "DBG".blue().bold().to_string(),
        Level::Trace => "TRC".purple().bold().to_string(),
    }
}

fn main() -> ExitCode {
    let cli = Cli::parse();

    let mut log_setup = env_logger::builder();
    log_setup.format(|f, record| {
        writeln!(f, "{}{}{} {}", "[".bright_black(), level_str(record.level()), "]".bright_black(), record.args())
    });
    match cli.verbose {
        0 => log_setup.filter_level(log::LevelFilter::Info),
        1 => log_setup.filter_level(log::LevelFilter::Debug),
        _ => log_setup.filter_level(log::LevelFilter::Trace),
    };
    log_setup.init();

    let rst = match cli.cmd {
        CliCmd::Conv(cmd) => cmd.run(),
        CliCmd::Completion(cmd) => cmd.run(),
    };

    match rst {
        Ok(()) => ExitCode::SUCCESS,
        Err(e) => {
            error!("{}", e);
            ExitCode::FAILURE
        }
    }
}
