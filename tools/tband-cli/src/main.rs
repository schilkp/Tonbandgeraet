mod cli;
mod log_cnt;
mod open;

use std::io::Write;

use colored::*;

use log::{error, Level};
use std::process::ExitCode;

use clap::Parser;
use cli::{Cli, CliCmd};

use crate::log_cnt::CountingLogger;

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

    let mut env_log_setup = env_logger::builder();
    env_log_setup.format(|f, record| {
        writeln!(f, "{}{}{} {}", "[".bright_black(), level_str(record.level()), "]".bright_black(), record.args())
    });
    match cli.verbose {
        0 => env_log_setup.filter_level(log::LevelFilter::Info),
        1 => env_log_setup.filter_level(log::LevelFilter::Debug),
        _ => env_log_setup.filter_level(log::LevelFilter::Trace),
    };
    let inner_logger = env_log_setup.build();
    log::set_max_level(inner_logger.filter());
    log::set_boxed_logger(Box::new(CountingLogger { inner: inner_logger })).expect("logger already set");

    let rst = match cli.cmd {
        CliCmd::Conv(cmd) => cmd.run(),
        CliCmd::Dump(cmd) => cmd.run(),
        CliCmd::Serve(cmd) => cmd.run(),
        CliCmd::Completion(cmd) => cmd.run(),
    };

    let exit_code = match rst {
        Ok(()) => ExitCode::SUCCESS,
        Err(e) => {
            error!("{}", e);
            ExitCode::FAILURE
        }
    };

    if cli.fail_on_warning && log_cnt::get_warn_count() > 0 {
        error!("Error: {} warning(s) issued and `--Werror` set.", log_cnt::get_warn_count());
        return ExitCode::FAILURE;
    }

    exit_code
}
