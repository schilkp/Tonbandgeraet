use std::path::PathBuf;

use anyhow::anyhow;
use frtrace_conv_core::{
    convert::TraceConverter,
    decode::{
        decode_frame,
        evts::{RawEvt, RawInvalidEvt},
    },
};
use log::{info, warn};

use crate::cli::{self, cmd_convert::InputFormat};

fn read_file(f: &PathBuf, format: InputFormat) -> anyhow::Result<Vec<u8>> {
    info!("Opening {format} file \"{}\"..", f.to_string_lossy());

    match format {
        InputFormat::Hex => {
            let mut bytes: Vec<u8> = Vec::with_capacity(2048);
            let content = std::fs::read_to_string(f)?;
            let mut string = String::new();
            for c in content.chars() {
                if c.is_whitespace() {
                    continue;
                }
                if !c.is_ascii_hexdigit() {
                    return Err(anyhow!("Invalid hex input file."));
                }

                string.push(c);
                if string.len() == 2 {
                    bytes.push(u8::from_str_radix(&string, 16)?);
                    string.clear();
                }
            }
            Ok(bytes)
        }
        InputFormat::Bin => Ok(std::fs::read(f)?),
    }
}

fn decode_file(bytes: &[u8]) -> anyhow::Result<Vec<RawEvt>> {
    info!("Decoding events..");

    let mut evts: Vec<RawEvt> = vec![];
    let mut frame_start: usize = 0;
    let mut last_ts: Option<u64> = None;

    for idx in 0..bytes.len() {
        if bytes[idx] == 0 {
            if frame_start == idx {
                info!("Empty frame. Ignoring.");
                frame_start = idx + 1;
                continue;
            }
            let evt = match decode_frame(&bytes[frame_start..=idx]) {
                Ok(evt) => {
                    if let Some(new_ts) = evt.ts() {
                        last_ts = Some(new_ts);
                    }
                    evt
                }
                Err(err) => {
                    warn!("Could not decode event: {err}");
                    RawEvt::Invalid(RawInvalidEvt {
                        ts: last_ts,
                        err: Some(err.into()),
                    })
                }
            };
            evts.push(evt);
            frame_start = idx + 1;
        }
    }

    if frame_start != bytes.len() {
        warn!("Trailing bytes! ignoring.");
    }

    Ok(evts)
}

pub fn convert(cmd: cli::cmd_convert::Cmd) -> anyhow::Result<()> {
    let mut tc = TraceConverter::new(cmd.core_count)?;

    for inp in cmd.input {
        let data = read_file(&inp.file, cmd.format)?;
        let evts = decode_file(&data)?;

        if let Some(core_id) = inp.core_id {
            info!("Adding events to core {core_id} trace sequence..");
            tc.add_evts_to_core(&evts, core_id)?;
        } else {
            info!("Adding events to trace sequence..");
            tc.add_evts(&evts)?;
        }
    }

    info!("Converting..");
    let trace = tc.convert()?;
    info!("Genertating Perfetto Trace..");
    let trace = trace.generate_perfetto_trace();
    info!("Conversion finished.");

    if let Some(output_path) = &cmd.output {
        info!("Saving to '{}'.", output_path.to_string_lossy());
        std::fs::write(output_path, trace)?;
    }

    if cmd.open {
        todo!()
    }

    if cmd.link {
        todo!()
    }

    Ok(())
}
