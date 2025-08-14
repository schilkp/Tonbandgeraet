/**
 * @file tband_encode.h
 * @brief FreeRTOS tracing event encoder
 * @warning This file is generated. Do not edit. See `code_gen` folder in repo.
 * @note Copyright (c) 2024 Philipp Schilk. Released under the MIT license.
 *
 * https://github.com/schilkp/Tonbandgeraet
 */
/*         ____   ___    _   _  ___ _____   _____ ____ ___ _____
 *        |  _ \ / _ \  | \ | |/ _ \_   _| | ____|  _ \_ _|_   _|
 *        | | | | | | | |  \| | | | || |   |  _| | | | | |  | |
 *        | |_| | |_| | | |\  | |_| || |   | |___| |_| | |  | |
 *        |____/ \___/  |_| \_|\___/ |_|   |_____|____/___| |_|
 *
 */
// clang-format off
#ifndef TRACE_ENCODER_H_
#define TRACE_ENCODER_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef tbandPROPER_INTERNAL_INCLUDE
  #error "This internal header file is not a public API and should not be included. Include tband.h instead."
#endif /* tbandPROPER_INTERNAL_INCLUDE */

// ==== COBS Framing ===========================================================

// COBS framing state
struct cobs_state {
  uint8_t *buf;    // Start of output buffer.
  size_t pos;      // Current location in output buffer.
  uint8_t code;    // Code value.
  size_t code_pos; // Code position.
  bool just_finished_full_block;
};

// Start COBS frame
static inline struct cobs_state cobs_start(uint8_t *out_buf) {
  struct cobs_state cobs = {
      .buf = out_buf,
      .pos = 1,      // Since first byte is COBS code byte, data starts at 1.
      .code = 1,     // First data byte is one away from first code byte.
      .code_pos = 0, // First COBS code byte is at 0.
      .just_finished_full_block = false,
  };
  return cobs;
}

// Add byte to COBS frame
static inline void cobs_add_byte(struct cobs_state *cobs, uint8_t byte) {

  cobs->just_finished_full_block = false;

  if (byte != 0) {
    // If this is a non-zero byte, add it to the output buffer:
    cobs->buf[cobs->pos] = byte;
    cobs->pos++;
    cobs->code++;
  }

  // If a block of 254 non-zero bytes is complete, restart the COBS coding:
  if (byte == 0 || cobs->code == 0xff) {

    if (byte != 0) {
      cobs->just_finished_full_block = true;
    }

    cobs->buf[cobs->code_pos] = cobs->code;
    cobs->code = 1;
    cobs->code_pos = cobs->pos;
    cobs->pos++;
  }
}

// Finish COBS frame. Returns final output size including trailing
// zero termination.
static inline size_t cobs_finish(struct cobs_state *cobs) {
  if (!cobs->just_finished_full_block) {
    cobs->buf[cobs->code_pos] = cobs->code; // Write last code byte
    cobs->buf[cobs->pos] = 0;               // Zero-terminate
    return cobs->pos + 1;                   // Return final buffer content length
  } else {
    cobs->buf[cobs->code_pos] = 0;
    return cobs->pos; // Return final buffer content length
  }
}

// Max length of a COBS frame with N data bytes and trailing delimiting zero
// added:
#define COBS_MAXLEN(N) ( (N) == 0 ? 2 : (1 + (((N) + 254 - 1)/254) + (N)) )

// ==== Field Encoding Functions ===============================================

static inline void encode_u8(struct cobs_state *cobs, uint8_t v) { cobs_add_byte(cobs, v); }

static inline void encode_u32(struct cobs_state *cobs, uint32_t v) {
  for (size_t i = 0; i < 5; i++) {
    uint8_t bits = v & 0x7F;
    v = v >> 7;
    if (v != 0) {
      cobs_add_byte(cobs, bits | 0x80);
    } else {
      cobs_add_byte(cobs, bits | 0x00);
      break;
    }
  }
}

static inline void encode_u64(struct cobs_state *cobs, uint64_t v) {
  for (size_t i = 0; i < 10; i++) {
    uint8_t bits = v & 0x7F;
    v = v >> 7;
    if (v != 0) {
      cobs_add_byte(cobs, bits | 0x80);
    } else {
      cobs_add_byte(cobs, bits | 0x00);
      break;
    }
  }
}

static inline void encode_s64(struct cobs_state *cobs, int64_t v) {
  // Negative max edge case:
  if (v == INT64_MIN) {
      cobs_add_byte(cobs, 0x1);
      return;
  }

  // Seperate sign, convert to positive value:
  uint8_t sign = 0;
  uint64_t bin_repr = 0;
  if (v < 0) {
    sign = 1;
    bin_repr = (uint64_t) -v;
  } else {
    bin_repr = (uint64_t) v;
  }

  // Generate bytes to encode, with sign at LSB
  bin_repr <<= 1;
  bin_repr |= sign;

  // varint encoding:
  for (size_t i = 0; i < 10; i++) {
    uint8_t bits = bin_repr & 0x7F;
    bin_repr = bin_repr >> 7;
    if (bin_repr != 0) {
      cobs_add_byte(cobs, bits | 0x80);
    } else {
      cobs_add_byte(cobs, bits | 0x00);
      break;
    }
  }
}

static inline void encode_str(struct cobs_state *cobs, const char *str) {
  if (str == 0) return;
  for (size_t i = 0; i < tband_configMAX_STR_LEN; i++) {
    if (*str == 0) {
      break;
    }
    cobs_add_byte(cobs, (uint8_t)*str);
    str++;
  }
}

// ==== Base Enums =============================================================

// ==== Base Encoder Functions =================================================

#define EVT_CORE_ID_IS_METADATA (0)
#define EVT_CORE_ID_MAXLEN (COBS_MAXLEN((16)))
static inline size_t encode_core_id(uint8_t buf[EVT_CORE_ID_MAXLEN], uint64_t ts, uint32_t core_id) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x0);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, core_id);
  return cobs_finish(&cobs);
}

#define EVT_DROPPED_EVT_CNT_IS_METADATA (0)
#define EVT_DROPPED_EVT_CNT_MAXLEN (COBS_MAXLEN((16)))
static inline size_t encode_dropped_evt_cnt(uint8_t buf[EVT_DROPPED_EVT_CNT_MAXLEN], uint64_t ts, uint32_t cnt) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x1);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, cnt);
  return cobs_finish(&cobs);
}

#define EVT_TS_RESOLUTION_NS_IS_METADATA (1)
#define EVT_TS_RESOLUTION_NS_MAXLEN (COBS_MAXLEN((11)))
static inline size_t encode_ts_resolution_ns(uint8_t buf[EVT_TS_RESOLUTION_NS_MAXLEN], uint64_t ns_per_ts) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x2);
  encode_u64(&cobs, ns_per_ts);
  return cobs_finish(&cobs);
}

#define EVT_ISR_NAME_IS_METADATA (1)
#define EVT_ISR_NAME_MAXLEN (COBS_MAXLEN((6 + tband_configMAX_STR_LEN)))
static inline size_t encode_isr_name(uint8_t buf[EVT_ISR_NAME_MAXLEN], uint32_t isr_id, const char *name) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x3);
  encode_u32(&cobs, isr_id);
  encode_str(&cobs, name);
  return cobs_finish(&cobs);
}

#define EVT_ISR_ENTER_IS_METADATA (0)
#define EVT_ISR_ENTER_MAXLEN (COBS_MAXLEN((16)))
static inline size_t encode_isr_enter(uint8_t buf[EVT_ISR_ENTER_MAXLEN], uint64_t ts, uint32_t isr_id) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x4);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, isr_id);
  return cobs_finish(&cobs);
}

#define EVT_ISR_EXIT_IS_METADATA (0)
#define EVT_ISR_EXIT_MAXLEN (COBS_MAXLEN((16)))
static inline size_t encode_isr_exit(uint8_t buf[EVT_ISR_EXIT_MAXLEN], uint64_t ts, uint32_t isr_id) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x5);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, isr_id);
  return cobs_finish(&cobs);
}

#define EVT_EVTMARKER_NAME_IS_METADATA (1)
#define EVT_EVTMARKER_NAME_MAXLEN (COBS_MAXLEN((6 + tband_configMAX_STR_LEN)))
static inline size_t encode_evtmarker_name(uint8_t buf[EVT_EVTMARKER_NAME_MAXLEN], uint32_t evtmarker_id, const char *name) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x6);
  encode_u32(&cobs, evtmarker_id);
  encode_str(&cobs, name);
  return cobs_finish(&cobs);
}

#define EVT_EVTMARKER_IS_METADATA (0)
#define EVT_EVTMARKER_MAXLEN (COBS_MAXLEN((16 + tband_configMAX_STR_LEN)))
static inline size_t encode_evtmarker(uint8_t buf[EVT_EVTMARKER_MAXLEN], uint64_t ts, uint32_t evtmarker_id, const char *msg) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x7);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, evtmarker_id);
  encode_str(&cobs, msg);
  return cobs_finish(&cobs);
}

#define EVT_EVTMARKER_BEGIN_IS_METADATA (0)
#define EVT_EVTMARKER_BEGIN_MAXLEN (COBS_MAXLEN((16 + tband_configMAX_STR_LEN)))
static inline size_t encode_evtmarker_begin(uint8_t buf[EVT_EVTMARKER_BEGIN_MAXLEN], uint64_t ts, uint32_t evtmarker_id, const char *msg) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x8);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, evtmarker_id);
  encode_str(&cobs, msg);
  return cobs_finish(&cobs);
}

#define EVT_EVTMARKER_END_IS_METADATA (0)
#define EVT_EVTMARKER_END_MAXLEN (COBS_MAXLEN((16)))
static inline size_t encode_evtmarker_end(uint8_t buf[EVT_EVTMARKER_END_MAXLEN], uint64_t ts, uint32_t evtmarker_id) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x9);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, evtmarker_id);
  return cobs_finish(&cobs);
}

#define EVT_VALMARKER_NAME_IS_METADATA (1)
#define EVT_VALMARKER_NAME_MAXLEN (COBS_MAXLEN((6 + tband_configMAX_STR_LEN)))
static inline size_t encode_valmarker_name(uint8_t buf[EVT_VALMARKER_NAME_MAXLEN], uint32_t valmarker_id, const char *name) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0xA);
  encode_u32(&cobs, valmarker_id);
  encode_str(&cobs, name);
  return cobs_finish(&cobs);
}

#define EVT_VALMARKER_IS_METADATA (0)
#define EVT_VALMARKER_MAXLEN (COBS_MAXLEN((26)))
static inline size_t encode_valmarker(uint8_t buf[EVT_VALMARKER_MAXLEN], uint64_t ts, uint32_t valmarker_id, int64_t val) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0xB);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, valmarker_id);
  encode_s64(&cobs, val);
  return cobs_finish(&cobs);
}

// ==== FreeRTOS Enums =========================================================

enum FrQueueKind {
  FRQK_QUEUE = 0x0,
  FRQK_COUNTING_SEMPHR = 0x1,
  FRQK_BINARY_SEMPHR = 0x2,
  FRQK_MUTEX = 0x3,
  FRQK_RECURSIVE_MUTEX = 0x4,
  FRQK_QUEUE_SET = 0x5,
};

enum FrStreamBufferKind {
  FRSBK_STREAM_BUFFER = 0x0,
  FRSBK_MESSAGE_BUFFER = 0x1,
};

// ==== FreeRTOS Encoder Functions =============================================

#define EVT_FREERTOS_TASK_SWITCHED_IN_IS_METADATA (0)
#define EVT_FREERTOS_TASK_SWITCHED_IN_MAXLEN (COBS_MAXLEN((16)))
static inline size_t encode_freertos_task_switched_in(uint8_t buf[EVT_FREERTOS_TASK_SWITCHED_IN_MAXLEN], uint64_t ts, uint32_t task_id) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x54);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, task_id);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_TASK_TO_RDY_STATE_IS_METADATA (0)
#define EVT_FREERTOS_TASK_TO_RDY_STATE_MAXLEN (COBS_MAXLEN((16)))
static inline size_t encode_freertos_task_to_rdy_state(uint8_t buf[EVT_FREERTOS_TASK_TO_RDY_STATE_MAXLEN], uint64_t ts, uint32_t task_id) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x55);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, task_id);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_TASK_RESUMED_IS_METADATA (0)
#define EVT_FREERTOS_TASK_RESUMED_MAXLEN (COBS_MAXLEN((16)))
static inline size_t encode_freertos_task_resumed(uint8_t buf[EVT_FREERTOS_TASK_RESUMED_MAXLEN], uint64_t ts, uint32_t task_id) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x56);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, task_id);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_TASK_RESUMED_FROM_ISR_IS_METADATA (0)
#define EVT_FREERTOS_TASK_RESUMED_FROM_ISR_MAXLEN (COBS_MAXLEN((16)))
static inline size_t encode_freertos_task_resumed_from_isr(uint8_t buf[EVT_FREERTOS_TASK_RESUMED_FROM_ISR_MAXLEN], uint64_t ts, uint32_t task_id) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x57);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, task_id);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_TASK_SUSPENDED_IS_METADATA (0)
#define EVT_FREERTOS_TASK_SUSPENDED_MAXLEN (COBS_MAXLEN((16)))
static inline size_t encode_freertos_task_suspended(uint8_t buf[EVT_FREERTOS_TASK_SUSPENDED_MAXLEN], uint64_t ts, uint32_t task_id) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x58);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, task_id);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_CURTASK_DELAY_IS_METADATA (0)
#define EVT_FREERTOS_CURTASK_DELAY_MAXLEN (COBS_MAXLEN((16)))
static inline size_t encode_freertos_curtask_delay(uint8_t buf[EVT_FREERTOS_CURTASK_DELAY_MAXLEN], uint64_t ts, uint32_t ticks) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x59);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, ticks);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_CURTASK_DELAY_UNTIL_IS_METADATA (0)
#define EVT_FREERTOS_CURTASK_DELAY_UNTIL_MAXLEN (COBS_MAXLEN((16)))
static inline size_t encode_freertos_curtask_delay_until(uint8_t buf[EVT_FREERTOS_CURTASK_DELAY_UNTIL_MAXLEN], uint64_t ts, uint32_t time_to_wake) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x5A);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, time_to_wake);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_TASK_PRIORITY_SET_IS_METADATA (0)
#define EVT_FREERTOS_TASK_PRIORITY_SET_MAXLEN (COBS_MAXLEN((21)))
static inline size_t encode_freertos_task_priority_set(uint8_t buf[EVT_FREERTOS_TASK_PRIORITY_SET_MAXLEN], uint64_t ts, uint32_t task_id, uint32_t priority) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x5B);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, task_id);
  encode_u32(&cobs, priority);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_TASK_PRIORITY_INHERIT_IS_METADATA (0)
#define EVT_FREERTOS_TASK_PRIORITY_INHERIT_MAXLEN (COBS_MAXLEN((21)))
static inline size_t encode_freertos_task_priority_inherit(uint8_t buf[EVT_FREERTOS_TASK_PRIORITY_INHERIT_MAXLEN], uint64_t ts, uint32_t task_id, uint32_t priority) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x5C);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, task_id);
  encode_u32(&cobs, priority);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_TASK_PRIORITY_DISINHERIT_IS_METADATA (0)
#define EVT_FREERTOS_TASK_PRIORITY_DISINHERIT_MAXLEN (COBS_MAXLEN((21)))
static inline size_t encode_freertos_task_priority_disinherit(uint8_t buf[EVT_FREERTOS_TASK_PRIORITY_DISINHERIT_MAXLEN], uint64_t ts, uint32_t task_id, uint32_t priority) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x5D);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, task_id);
  encode_u32(&cobs, priority);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_TASK_CREATED_IS_METADATA (0)
#define EVT_FREERTOS_TASK_CREATED_MAXLEN (COBS_MAXLEN((16)))
static inline size_t encode_freertos_task_created(uint8_t buf[EVT_FREERTOS_TASK_CREATED_MAXLEN], uint64_t ts, uint32_t task_id) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x5E);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, task_id);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_TASK_NAME_IS_METADATA (1)
#define EVT_FREERTOS_TASK_NAME_MAXLEN (COBS_MAXLEN((6 + tband_configMAX_STR_LEN)))
static inline size_t encode_freertos_task_name(uint8_t buf[EVT_FREERTOS_TASK_NAME_MAXLEN], uint32_t task_id, const char *name) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x5F);
  encode_u32(&cobs, task_id);
  encode_str(&cobs, name);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_TASK_IS_IDLE_TASK_IS_METADATA (1)
#define EVT_FREERTOS_TASK_IS_IDLE_TASK_MAXLEN (COBS_MAXLEN((11)))
static inline size_t encode_freertos_task_is_idle_task(uint8_t buf[EVT_FREERTOS_TASK_IS_IDLE_TASK_MAXLEN], uint32_t task_id, uint32_t core_id) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x60);
  encode_u32(&cobs, task_id);
  encode_u32(&cobs, core_id);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_TASK_IS_TIMER_TASK_IS_METADATA (1)
#define EVT_FREERTOS_TASK_IS_TIMER_TASK_MAXLEN (COBS_MAXLEN((6)))
static inline size_t encode_freertos_task_is_timer_task(uint8_t buf[EVT_FREERTOS_TASK_IS_TIMER_TASK_MAXLEN], uint32_t task_id) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x61);
  encode_u32(&cobs, task_id);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_TASK_DELETED_IS_METADATA (0)
#define EVT_FREERTOS_TASK_DELETED_MAXLEN (COBS_MAXLEN((16)))
static inline size_t encode_freertos_task_deleted(uint8_t buf[EVT_FREERTOS_TASK_DELETED_MAXLEN], uint64_t ts, uint32_t task_id) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x62);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, task_id);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_QUEUE_CREATED_IS_METADATA (0)
#define EVT_FREERTOS_QUEUE_CREATED_MAXLEN (COBS_MAXLEN((16)))
static inline size_t encode_freertos_queue_created(uint8_t buf[EVT_FREERTOS_QUEUE_CREATED_MAXLEN], uint64_t ts, uint32_t queue_id) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x63);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, queue_id);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_QUEUE_NAME_IS_METADATA (1)
#define EVT_FREERTOS_QUEUE_NAME_MAXLEN (COBS_MAXLEN((6 + tband_configMAX_STR_LEN)))
static inline size_t encode_freertos_queue_name(uint8_t buf[EVT_FREERTOS_QUEUE_NAME_MAXLEN], uint32_t queue_id, const char *name) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x64);
  encode_u32(&cobs, queue_id);
  encode_str(&cobs, name);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_QUEUE_KIND_IS_METADATA (1)
#define EVT_FREERTOS_QUEUE_KIND_MAXLEN (COBS_MAXLEN((7)))
static inline size_t encode_freertos_queue_kind(uint8_t buf[EVT_FREERTOS_QUEUE_KIND_MAXLEN], uint32_t queue_id, enum FrQueueKind kind) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x65);
  encode_u32(&cobs, queue_id);
  encode_u8(&cobs, (uint8_t)kind);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_QUEUE_SEND_IS_METADATA (0)
#define EVT_FREERTOS_QUEUE_SEND_MAXLEN (COBS_MAXLEN((21)))
static inline size_t encode_freertos_queue_send(uint8_t buf[EVT_FREERTOS_QUEUE_SEND_MAXLEN], uint64_t ts, uint32_t queue_id, uint32_t len_after) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x66);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, queue_id);
  encode_u32(&cobs, len_after);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_QUEUE_SEND_FROM_ISR_IS_METADATA (0)
#define EVT_FREERTOS_QUEUE_SEND_FROM_ISR_MAXLEN (COBS_MAXLEN((21)))
static inline size_t encode_freertos_queue_send_from_isr(uint8_t buf[EVT_FREERTOS_QUEUE_SEND_FROM_ISR_MAXLEN], uint64_t ts, uint32_t queue_id, uint32_t len_after) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x67);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, queue_id);
  encode_u32(&cobs, len_after);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_QUEUE_OVERWRITE_IS_METADATA (0)
#define EVT_FREERTOS_QUEUE_OVERWRITE_MAXLEN (COBS_MAXLEN((21)))
static inline size_t encode_freertos_queue_overwrite(uint8_t buf[EVT_FREERTOS_QUEUE_OVERWRITE_MAXLEN], uint64_t ts, uint32_t queue_id, uint32_t len_after) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x68);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, queue_id);
  encode_u32(&cobs, len_after);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_QUEUE_OVERWRITE_FROM_ISR_IS_METADATA (0)
#define EVT_FREERTOS_QUEUE_OVERWRITE_FROM_ISR_MAXLEN (COBS_MAXLEN((21)))
static inline size_t encode_freertos_queue_overwrite_from_isr(uint8_t buf[EVT_FREERTOS_QUEUE_OVERWRITE_FROM_ISR_MAXLEN], uint64_t ts, uint32_t queue_id, uint32_t len_after) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x69);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, queue_id);
  encode_u32(&cobs, len_after);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_QUEUE_RECEIVE_IS_METADATA (0)
#define EVT_FREERTOS_QUEUE_RECEIVE_MAXLEN (COBS_MAXLEN((21)))
static inline size_t encode_freertos_queue_receive(uint8_t buf[EVT_FREERTOS_QUEUE_RECEIVE_MAXLEN], uint64_t ts, uint32_t queue_id, uint32_t len_after) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x6A);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, queue_id);
  encode_u32(&cobs, len_after);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_QUEUE_RECEIVE_FROM_ISR_IS_METADATA (0)
#define EVT_FREERTOS_QUEUE_RECEIVE_FROM_ISR_MAXLEN (COBS_MAXLEN((21)))
static inline size_t encode_freertos_queue_receive_from_isr(uint8_t buf[EVT_FREERTOS_QUEUE_RECEIVE_FROM_ISR_MAXLEN], uint64_t ts, uint32_t queue_id, uint32_t len_after) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x6B);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, queue_id);
  encode_u32(&cobs, len_after);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_QUEUE_RESET_IS_METADATA (0)
#define EVT_FREERTOS_QUEUE_RESET_MAXLEN (COBS_MAXLEN((16)))
static inline size_t encode_freertos_queue_reset(uint8_t buf[EVT_FREERTOS_QUEUE_RESET_MAXLEN], uint64_t ts, uint32_t queue_id) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x6C);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, queue_id);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_CURTASK_BLOCK_ON_QUEUE_PEEK_IS_METADATA (0)
#define EVT_FREERTOS_CURTASK_BLOCK_ON_QUEUE_PEEK_MAXLEN (COBS_MAXLEN((21)))
static inline size_t encode_freertos_curtask_block_on_queue_peek(uint8_t buf[EVT_FREERTOS_CURTASK_BLOCK_ON_QUEUE_PEEK_MAXLEN], uint64_t ts, uint32_t queue_id, uint32_t ticks_to_wait) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x6D);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, queue_id);
  encode_u32(&cobs, ticks_to_wait);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_CURTASK_BLOCK_ON_QUEUE_SEND_IS_METADATA (0)
#define EVT_FREERTOS_CURTASK_BLOCK_ON_QUEUE_SEND_MAXLEN (COBS_MAXLEN((21)))
static inline size_t encode_freertos_curtask_block_on_queue_send(uint8_t buf[EVT_FREERTOS_CURTASK_BLOCK_ON_QUEUE_SEND_MAXLEN], uint64_t ts, uint32_t queue_id, uint32_t ticks_to_wait) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x6E);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, queue_id);
  encode_u32(&cobs, ticks_to_wait);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_CURTASK_BLOCK_ON_QUEUE_RECEIVE_IS_METADATA (0)
#define EVT_FREERTOS_CURTASK_BLOCK_ON_QUEUE_RECEIVE_MAXLEN (COBS_MAXLEN((21)))
static inline size_t encode_freertos_curtask_block_on_queue_receive(uint8_t buf[EVT_FREERTOS_CURTASK_BLOCK_ON_QUEUE_RECEIVE_MAXLEN], uint64_t ts, uint32_t queue_id, uint32_t ticks_to_wait) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x6F);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, queue_id);
  encode_u32(&cobs, ticks_to_wait);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_QUEUE_CUR_LENGTH_IS_METADATA (0)
#define EVT_FREERTOS_QUEUE_CUR_LENGTH_MAXLEN (COBS_MAXLEN((21)))
static inline size_t encode_freertos_queue_cur_length(uint8_t buf[EVT_FREERTOS_QUEUE_CUR_LENGTH_MAXLEN], uint64_t ts, uint32_t queue_id, uint32_t length) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x70);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, queue_id);
  encode_u32(&cobs, length);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_TASK_EVTMARKER_NAME_IS_METADATA (1)
#define EVT_FREERTOS_TASK_EVTMARKER_NAME_MAXLEN (COBS_MAXLEN((11 + tband_configMAX_STR_LEN)))
static inline size_t encode_freertos_task_evtmarker_name(uint8_t buf[EVT_FREERTOS_TASK_EVTMARKER_NAME_MAXLEN], uint32_t evtmarker_id, uint32_t task_id, const char *name) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x7A);
  encode_u32(&cobs, evtmarker_id);
  encode_u32(&cobs, task_id);
  encode_str(&cobs, name);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_TASK_EVTMARKER_IS_METADATA (0)
#define EVT_FREERTOS_TASK_EVTMARKER_MAXLEN (COBS_MAXLEN((16 + tband_configMAX_STR_LEN)))
static inline size_t encode_freertos_task_evtmarker(uint8_t buf[EVT_FREERTOS_TASK_EVTMARKER_MAXLEN], uint64_t ts, uint32_t evtmarker_id, const char *msg) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x7B);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, evtmarker_id);
  encode_str(&cobs, msg);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_TASK_EVTMARKER_BEGIN_IS_METADATA (0)
#define EVT_FREERTOS_TASK_EVTMARKER_BEGIN_MAXLEN (COBS_MAXLEN((16 + tband_configMAX_STR_LEN)))
static inline size_t encode_freertos_task_evtmarker_begin(uint8_t buf[EVT_FREERTOS_TASK_EVTMARKER_BEGIN_MAXLEN], uint64_t ts, uint32_t evtmarker_id, const char *msg) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x7C);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, evtmarker_id);
  encode_str(&cobs, msg);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_TASK_EVTMARKER_END_IS_METADATA (0)
#define EVT_FREERTOS_TASK_EVTMARKER_END_MAXLEN (COBS_MAXLEN((16)))
static inline size_t encode_freertos_task_evtmarker_end(uint8_t buf[EVT_FREERTOS_TASK_EVTMARKER_END_MAXLEN], uint64_t ts, uint32_t evtmarker_id) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x7D);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, evtmarker_id);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_TASK_VALMARKER_NAME_IS_METADATA (1)
#define EVT_FREERTOS_TASK_VALMARKER_NAME_MAXLEN (COBS_MAXLEN((11 + tband_configMAX_STR_LEN)))
static inline size_t encode_freertos_task_valmarker_name(uint8_t buf[EVT_FREERTOS_TASK_VALMARKER_NAME_MAXLEN], uint32_t valmarker_id, uint32_t task_id, const char *name) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x7E);
  encode_u32(&cobs, valmarker_id);
  encode_u32(&cobs, task_id);
  encode_str(&cobs, name);
  return cobs_finish(&cobs);
}

#define EVT_FREERTOS_TASK_VALMARKER_IS_METADATA (0)
#define EVT_FREERTOS_TASK_VALMARKER_MAXLEN (COBS_MAXLEN((26)))
static inline size_t encode_freertos_task_valmarker(uint8_t buf[EVT_FREERTOS_TASK_VALMARKER_MAXLEN], uint64_t ts, uint32_t valmarker_id, int64_t val) {
  struct cobs_state cobs = cobs_start(buf);
  encode_u8(&cobs, 0x7F);
  encode_u64(&cobs, ts);
  encode_u32(&cobs, valmarker_id);
  encode_s64(&cobs, val);
  return cobs_finish(&cobs);
}

#endif /* TRACE_ENCODER_H_ */
// clang-format on
