/**
 * @file tband.c
 * @brief Tonbandgeraet embedded tracer: Basic events.
 * @author Philipp Schilk, 2024
 * @note Copyright (c) 2024 Philipp Schilk. Released under the MIT license.
 *
 * https://github.com/schilkp/Tonbandgeraet
 */

#include "tband.h"
#if (tband_configENABLE == 1)

// std:
#include <stdatomic.h>
#include <stdbool.h>

#define tbandPROPER_INTERNAL_INCLUDE
#include "tband_internal.h"
#undef tbandPROPER_INTERNAL_INCLUDE

// ===== TRACE HOOKS ===========================================================

void impl_tband_gather_system_metadata(void) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_TS_RESOLUTION_NS_MAXLEN];
  size_t len = encode_ts_resolution_ns(buf, tband_portTIMESTAMP_RESOLUTION_NS);
  handle_trace_evt(buf, len, EVT_TS_RESOLUTION_NS_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}

#if (tband_configISR_TRACE_ENABLE == 1)
void impl_tband_isr_name(uint32_t isr_id, const char *name) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_ISR_NAME_MAXLEN];
  size_t len = encode_isr_name(buf, isr_id, name);
  handle_trace_evt(buf, len, EVT_ISR_NAME_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* *tband_configISR_TRACE_ENABLE == 1) */

#if (tband_configISR_TRACE_ENABLE == 1)
void impl_tband_isr_enter(uint32_t isr_id) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_ISR_ENTER_MAXLEN];
  size_t len = encode_isr_enter(buf, ts, isr_id);
  handle_trace_evt(buf, len, EVT_ISR_ENTER_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* *tband_configISR_TRACE_ENABLE == 1) */

#if (tband_configISR_TRACE_ENABLE == 1)
void impl_tband_isr_exit(uint32_t isr_id) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_ISR_EXIT_MAXLEN];
  size_t len = encode_isr_exit(buf, ts, isr_id);
  handle_trace_evt(buf, len, EVT_ISR_EXIT_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* tband_configISR_TRACE_ENABLE == 1 */

#if (tband_configMARKER_TRACE_ENABLE == 1)
void impl_tband_evtmarker_name(uint32_t id, const char *name) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_EVTMARKER_NAME_MAXLEN];
  size_t len = encode_evtmarker_name(buf, id, name);
  handle_trace_evt(buf, len, EVT_EVTMARKER_NAME_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* (tband_configMARKER_TRACE_ENABLE == 1) */

#if (tband_configMARKER_TRACE_ENABLE == 1)
void impl_tband_evtmarker(uint32_t id, const char *msg) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_EVTMARKER_MAXLEN];
  size_t len = encode_evtmarker(buf, ts, id, msg);
  handle_trace_evt(buf, len, EVT_EVTMARKER_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* (tband_configMARKER_TRACE_ENABLE == 1) */

#if (tband_configMARKER_TRACE_ENABLE == 1)
void impl_tband_evtmarker_begin(uint32_t id, const char *msg) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_EVTMARKER_BEGIN_MAXLEN];
  size_t len = encode_evtmarker_begin(buf, ts, id, msg);
  handle_trace_evt(buf, len, EVT_EVTMARKER_BEGIN_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* (tband_configMARKER_TRACE_ENABLE == 1) */

#if (tband_configMARKER_TRACE_ENABLE == 1)
void impl_tband_evtmarker_end(uint32_t id) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_EVTMARKER_END_MAXLEN];
  size_t len = encode_evtmarker_end(buf, ts, id);
  handle_trace_evt(buf, len, EVT_EVTMARKER_END_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* (tband_configMARKER_TRACE_ENABLE == 1) */

#if (tband_configMARKER_TRACE_ENABLE == 1)
void impl_tband_valmarker_name(uint32_t id, const char *name) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_VALMARKER_NAME_MAXLEN];
  size_t len = encode_valmarker_name(buf, id, name);
  handle_trace_evt(buf, len, EVT_VALMARKER_NAME_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* (tband_configMARKER_TRACE_ENABLE == 1) */

#if (tband_configMARKER_TRACE_ENABLE == 1)
void impl_tband_valmarker(uint32_t id, int64_t val) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_VALMARKER_MAXLEN];
  size_t len = encode_valmarker(buf, ts, id, val);
  handle_trace_evt(buf, len, EVT_VALMARKER_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* (tband_configMARKER_TRACE_ENABLE == 1) */

// ===== Trace Handling ========================================================

// Number of dropped events (shared between all cores)
static volatile atomic_ulong dropped_evt_cnt = 0;

// Last submitted dropped event count (local to core)
static volatile unsigned long last_traced_dropped_evt_cnts[tband_portNUMBER_OF_CORES] = {0};

#if tband_configTRACE_DROP_CNT_EVERY > 0
// Dropped event tracing event inclusion state counter (local to core)
static volatile uint32_t dropped_evt_trace_periodic_cnts[tband_portNUMBER_OF_CORES] = {0};
#endif

void handle_trace_evt(uint8_t *buf, size_t len, bool is_metadata, uint64_t ts) {
#if tband_configTRACE_DROP_CNT_EVERY > 0
  uint32_t dropped_evt_trace_period_cnt = dropped_evt_trace_periodic_cnts[tband_portGET_CORE_ID()];
#else
  uint32_t dropped_evt_trace_period_cnt = 0;
#endif
  unsigned long last_traced_dropped_evt_cnt = last_traced_dropped_evt_cnts[tband_portGET_CORE_ID()];
  uint32_t current_dropped_evt_cnt = atomic_load(&dropped_evt_cnt);

  // Submit 'dropped evt count' marker if the value of dropped event count has changed
  // since we last traced it, or tband_configTRACE_DROP_CNT_EVERY events have passed.
  if (((tband_configTRACE_DROP_CNT_EVERY > 0) && dropped_evt_trace_period_cnt == 0) ||
      last_traced_dropped_evt_cnt != current_dropped_evt_cnt) {
    uint8_t buf_dropped[EVT_DROPPED_EVT_CNT_MAXLEN];
    size_t len = encode_dropped_evt_cnt(buf_dropped, ts, atomic_load(&dropped_evt_cnt));
    bool did_drop_evt = tband_submit_to_backend(buf_dropped, len, EVT_DROPPED_EVT_CNT_IS_METADATA);

    if (did_drop_evt) {
      // Increase dropped event count and try again to submit a dropped event count evt
      // next time. Abort.
      (void)atomic_fetch_add(&dropped_evt_cnt, 1);
      return;
    } else {
#if tband_configTRACE_DROP_CNT_EVERY > 0
      // Successfully submitted dropped event count. Submit next one in
      // tband_configTRACE_DROP_CNT_EVERY events.
      dropped_evt_trace_periodic_cnts[tband_portGET_CORE_ID()] = tband_configTRACE_DROP_CNT_EVERY;
#endif
      (void)0; // don't warn on empty else.
    }
  } else {
    // No dropped event count has to be traced on this call. If periodic dropped
    // event count inclusions are enabled decrease the counter:
#if tband_configTRACE_DROP_CNT_EVERY > 0
    dropped_evt_trace_periodic_cnts[tband_portGET_CORE_ID()]--;
#endif
    (void)0; // don't warn on empty else.
  }

  // Submit to port:
  bool did_drop_evt = tband_submit_to_backend(buf, len, is_metadata);

  if (did_drop_evt) {
    (void)atomic_fetch_add(&dropped_evt_cnt, 1);
  }
}

#endif /* tband_configENABLE == 1*/
