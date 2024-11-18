mod cobs;
pub mod evts;

use anyhow::anyhow;
use log::warn;

use crate::decode::evts::InvalidEvt;

use self::evts::{RawEvt, TraceMode};

pub fn decode_frame(input: &[u8], mode: TraceMode) -> anyhow::Result<RawEvt> {
    RawEvt::decode(&cobs::cobs_decode_frame(input)?, mode)
}

fn bytes_left(evt_buf: &[u8], current_idx: usize) -> bool {
    evt_buf.len() > current_idx
}

fn decode_u8(evt_buf: &[u8], current_idx: &mut usize) -> anyhow::Result<u8> {
    if !bytes_left(evt_buf, *current_idx) {
        return Err(anyhow!("Insufficient bytes left in event to decode a u8"));
    }
    let result = evt_buf[*current_idx];
    *current_idx += 1;
    Ok(result)
}

fn decode_u32(evt_buf: &[u8], current_idx: &mut usize) -> anyhow::Result<u32> {
    if !bytes_left(evt_buf, *current_idx) {
        return Err(anyhow!("Insufficient bytes left in event to decode a u32"));
    }

    let mut result = 0;

    for i in 0..5 {
        if !bytes_left(evt_buf, *current_idx) {
            return Err(anyhow!("Unterminated u32"));
        }
        let byte = evt_buf[*current_idx];
        result |= ((byte as u32) & 0x7F) << (7 * i);
        *current_idx += 1;

        if i == 4 && (byte & !0xF != 0) {
            return Err(anyhow!("u32 too long!"));
        }

        if byte & 0x80 == 0 {
            break;
        }
    }
    Ok(result)
}

fn decode_u64(evt_buf: &[u8], current_idx: &mut usize) -> anyhow::Result<u64> {
    if !bytes_left(evt_buf, *current_idx) {
        return Err(anyhow!("Insufficient bytes left in event to decode a u64"));
    }
    let mut result = 0;

    for i in 0..10 {
        if !bytes_left(evt_buf, *current_idx) {
            return Err(anyhow!("Unterminated u64"));
        }
        let byte = evt_buf[*current_idx];
        result |= ((byte as u64) & 0x7F) << (7 * i);
        *current_idx += 1;

        if i == 9 && (byte & !0x1 != 0) {
            return Err(anyhow!("u64 too long!"));
        }

        if byte & 0x80 == 0 {
            break;
        }
    }
    Ok(result)
}

fn decode_s64(evt_buf: &[u8], current_idx: &mut usize) -> anyhow::Result<i64> {
    if !bytes_left(evt_buf, *current_idx) {
        return Err(anyhow!("Insufficient bytes left in event to decode a s64"));
    }
    let mut bin_result: u64 = 0;

    for i in 0..10 {
        if !bytes_left(evt_buf, *current_idx) {
            return Err(anyhow!("Unterminated s64"));
        }
        let byte = evt_buf[*current_idx];
        bin_result |= ((byte as u64) & 0x7F) << (7 * i);
        *current_idx += 1;

        if i == 9 && (byte & !0x1 != 0) {
            return Err(anyhow!("s64 too long!"));
        }

        if byte & 0x80 == 0 {
            break;
        }
    }

    if bin_result == 0x01 {
        Ok(i64::MIN)
    } else {
        let sign: bool = (bin_result & 0x1) != 0;
        let magn: u64 = bin_result >> 1;

        if sign {
            Ok(-(magn as i64))
        } else {
            Ok(magn as i64)
        }
    }
}

fn decode_string(evt_buf: &[u8], current_idx: &mut usize) -> anyhow::Result<String> {
    if !bytes_left(evt_buf, *current_idx) {
        Ok(String::new())
    } else {
        let result = String::from_utf8(evt_buf[*current_idx..].to_vec())?;
        *current_idx = evt_buf.len();
        Ok(result)
    }
}

#[derive(Debug, Clone)]
pub struct StreamDecoder {
    mode: TraceMode,
    frame_buf: Vec<u8>,
    last_ts: Option<u64>,
}

impl StreamDecoder {
    pub fn new(mode: TraceMode) -> Self {
        StreamDecoder {
            mode,
            frame_buf: vec![],
            last_ts: None,
        }
    }

    pub fn process_binary(&mut self, input: &[u8]) -> Vec<RawEvt> {
        let mut result = vec![];

        for byte in input {
            self.frame_buf.push(*byte);
            if *byte == 0 {
                result.push(self.process_full_frame());
                self.frame_buf.clear();
            }
        }

        result
    }

    fn process_full_frame(&mut self) -> RawEvt {
        if self.frame_buf.len() == 1 && self.frame_buf[0] == 0 {
            warn!("Empty COBS frame. Ignoring.");
            return RawEvt::Invalid(InvalidEvt {
                ts: self.last_ts,
                err: Some("Empty COBS frame".to_string()),
            });
        }

        match decode_frame(&self.frame_buf, self.mode) {
            Ok(evt) => {
                if let Some(new_ts) = evt.ts() {
                    self.last_ts = Some(new_ts);
                }
                evt
            }
            Err(err) => {
                warn!("Could not decode event: {err}.");
                RawEvt::Invalid(InvalidEvt {
                    ts: self.last_ts,
                    err: Some(err.to_string()),
                })
            }
        }
    }

    pub fn get_bytes_in_buffer(&self) -> usize {
        self.frame_buf.len()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_bytes_left() {
        assert!(bytes_left(&[0, 1, 2], 0));
        assert!(bytes_left(&[0, 1, 2], 1));
        assert!(bytes_left(&[0, 1, 2], 2));
        assert!(!bytes_left(&[0, 1, 2], 3));
        assert!(!bytes_left(&[0, 1, 2], 4));

        assert!(!bytes_left(&[], 0));
    }

    #[test]
    fn test_decode_u8() {
        let mut idx: usize = 0;
        assert_eq!(decode_u8(&[0, 1, 2], &mut idx).unwrap(), 0);
        assert_eq!(decode_u8(&[0, 1, 2], &mut idx).unwrap(), 1);
        assert_eq!(decode_u8(&[0, 1, 2], &mut idx).unwrap(), 2);
        assert_eq!(idx, 3);
        decode_u8(&[0, 1, 2], &mut idx).unwrap_err();
    }

    #[test]
    fn test_decode_u32() {
        let mut idx: usize = 0;
        assert_eq!(decode_u32(&[0, 1, 2], &mut idx).unwrap(), 0);
        assert_eq!(decode_u32(&[0, 1, 2], &mut idx).unwrap(), 1);
        assert_eq!(decode_u32(&[0, 1, 2], &mut idx).unwrap(), 2);
        assert_eq!(idx, 3);
        decode_u32(&[0, 1, 2], &mut idx).unwrap_err();

        assert_eq!(decode_u32(&[0xFF, 0xFF, 0xFF, 0xFF, 0xF], &mut 0).unwrap(), 0xFFFFFFFF);

        // Too long
        decode_u32(&[0xFF, 0xFF, 0xFF, 0xFF, 0x1F], &mut 0).unwrap_err();
        decode_u32(&[0xFF, 0xFF, 0xFF, 0xFF, 0x8F], &mut 0).unwrap_err();

        // Unterminated
        decode_u32(&[0xFF, 0xFF, 0xFF, 0xFF], &mut 0).unwrap_err();
    }

    #[test]
    fn test_decode_u64() {
        let mut idx: usize = 0;
        assert_eq!(decode_u64(&[0, 1, 2], &mut idx).unwrap(), 0);
        assert_eq!(decode_u64(&[0, 1, 2], &mut idx).unwrap(), 1);
        assert_eq!(decode_u64(&[0, 1, 2], &mut idx).unwrap(), 2);
        assert_eq!(idx, 3);
        decode_u64(&[0, 1, 2], &mut idx).unwrap_err();

        assert_eq!(
            decode_u64(&[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1], &mut 0).unwrap(),
            0xFFFFFFFFFFFFFFFF
        );

        // Too long
        decode_u64(&[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3], &mut 0).unwrap_err();
        decode_u64(&[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x81], &mut 0).unwrap_err();

        // Unterminated
        decode_u64(&[0xFF, 0xFF, 0xFF, 0xFF], &mut 0).unwrap_err();
    }

    #[test]
    fn test_decode_s64() {
        let mut idx: usize = 0;
        let buf = [0, 1, 2, 3, 4, 5];
        assert_eq!(decode_s64(&buf, &mut idx).unwrap(), 0);
        assert_eq!(decode_s64(&buf, &mut idx).unwrap(), i64::MIN);
        assert_eq!(decode_s64(&buf, &mut idx).unwrap(), 1);
        assert_eq!(decode_s64(&buf, &mut idx).unwrap(), -1);
        assert_eq!(decode_s64(&buf, &mut idx).unwrap(), 2);
        assert_eq!(decode_s64(&buf, &mut idx).unwrap(), -2);
        assert_eq!(idx, 6);
        decode_u64(&buf, &mut idx).unwrap_err();

        assert_eq!(
            decode_s64(&[0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01], &mut 0).unwrap(),
            i64::MAX,
        );

        assert_eq!(
            decode_s64(&[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01], &mut 0).unwrap(),
            i64::MIN + 1,
        );

        // Too long
        decode_s64(&[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3], &mut 0).unwrap_err();
        decode_s64(&[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x81], &mut 0).unwrap_err();

        // Unterminated
        decode_s64(&[0xFF, 0xFF, 0xFF, 0xFF], &mut 0).unwrap_err();
    }

    #[test]
    fn test_decode_freertos_task_to_ready() {
        use crate::decode::evts::*;

        let buf = [0x55, 0xa3, 0x8d, 0xe3, 0x04, 0xab, 0x04];
        let evt = RawEvt::decode(&buf, TraceMode::FreeRTOS).unwrap();

        let RawEvt::FreeRTOS(evt) = evt else {
            panic!("Wrong event class.");
        };
        assert_eq!(evt.ts, 10012323);
        let FreeRTOSEvtKind::TaskToRdyState(evt) = evt.kind else {
            panic!("Wrong event.");
        };
        assert_eq!(evt.task_id, 555);
    }

    #[test]
    fn test_decode_freertos_priority_disinherit() {
        use crate::decode::evts::*;

        let buf = [0x5d, 0xfc, 0x80, 0xfe, 0xc1, 0x04, 0x65, 0x2a];
        let evt = RawEvt::decode(&buf, TraceMode::FreeRTOS).unwrap();

        let RawEvt::FreeRTOS(evt) = evt else {
            panic!("Wrong event class.");
        };
        assert_eq!(evt.ts, 1212121212);
        let FreeRTOSEvtKind::TaskPriorityDisinherit(evt) = evt.kind else {
            panic!("Wrong event.");
        };
        assert_eq!(evt.task_id, 101);
        assert_eq!(evt.priority, 42);
    }

    #[test]
    fn test_decode_freertos_queue_name() {
        use crate::decode::evts::*;

        let buf = [0x64, 0x65, 0x74, 0x65, 0x73, 0x74, 0x31, 0x32, 0x31, 0x32];
        let evt = RawEvt::decode(&buf, TraceMode::FreeRTOS).unwrap();

        let RawEvt::FreeRTOSMetadata(evt) = evt else {
            panic!("Wrong event class.");
        };
        let FreeRTOSMetadataEvt::QueueName(evt) = evt else {
            panic!("Wrong event.");
        };
        assert_eq!(evt.queue_id, 101);
        assert_eq!(evt.name, String::from("test1212"));
    }
}
