use crate::cli::cmd_convert::read_file;
use log::{info, warn};
use tband_conv::decode::StreamDecoder;

use clap::Parser;

use super::cmd_convert::{InputFile, InputFormat, TraceMode};

#[derive(Parser, Debug)]
#[command(about = "Dump trace recording")]
pub struct Cmd {
    /// Input format
    #[arg(short, long, default_value = "bin")]
    pub format: InputFormat,

    /// TraceMode
    #[arg(short, long)]
    pub mode: TraceMode,

    /// Input file with optional core id.
    ///
    /// For split multi-core recording, append core id to file name as such: filename@core_id
    #[arg(short, long)]
    pub input: InputFile,
}

impl Cmd {
    pub fn run(self) -> anyhow::Result<()> {
        let mode = match self.mode {
            TraceMode::BareMetal => tband_conv::decode::evts::TraceMode::Base,
            TraceMode::FreeRTOS => tband_conv::decode::evts::TraceMode::FreeRTOS,
        };

        info!("Opening {} file \"{}\"..", self.format, self.input.file.to_string_lossy());
        let data = read_file(&self.input.file, self.format)?;

        info!("Decoding events..");
        let mut decode = StreamDecoder::new(mode);
        let evts = decode.process_binary(&data);

        for evt in evts {
            println!("{}", serde_json::to_string(&evt).unwrap());
        }

        let trailing_bytes = decode.get_bytes_in_buffer();
        if trailing_bytes > 0 {
            warn!("{trailing_bytes} trailing bytes.");
        }

        Ok(())
    }
}
