/**
 * @file tband.h
 * @brief Tonbandgeraet embedded tracer.
 * @author Philipp Schilk, 2024
 * @note Copyright (c) 2024 Philipp Schilk. Released under the MIT license.
 *
 * https://github.com/schilkp/Tonbandgeraet
 */
// clang-format off

#ifndef TBAND_H_
#define TBAND_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

// FIXME: Add C extern guards in all headers!

//                       ____ ___  _   _ _____ ___ ____
//                      / ___/ _ \| \ | |  ___|_ _/ ___|
// =================== | |  | | | |  \| | |_   | | |  _  =======================
// =================== | |__| |_| | |\  |  _|  | | |_| | =======================
//                      \____\___/|_| \_|_|   |___\____|

#include "tband_config.h"

// == CONFIG: GENERAL ==========================================================

#ifndef tband_configENABLE
  #define tband_configENABLE 0
#endif /* tband_configENABLE */

#ifndef tband_configMAX_STR_LEN
  #define tband_configMAX_STR_LEN (20)
#endif /* tband_configMAX_STR_LEN */

#ifndef tband_configTRACE_DROP_CNT_EVERY
  #define tband_configTRACE_DROP_CNT_EVERY (50)
#endif /* tband_configTRACE_DROP_CNT_EVERY */

#ifndef tband_configMARKER_TRACE_ENABLE
  #define tband_configMARKER_TRACE_ENABLE 1
#endif /* tband_configMARKER_TRACE_ENABLE */

#ifndef tband_configISR_TRACE_ENABLE
  #define tband_configISR_TRACE_ENABLE 1
#endif /* tband_configISR_TRACE_ENABLE */

#ifndef tband_configFREERTOS_TRACE_ENABLE
  #define tband_configFREERTOS_TRACE_ENABLE 0
#endif /* tband_configFREERTOS_TRACE_ENABLE */

// == CONFIG: METADATA BUF =====================================================

#ifndef tband_configUSE_METADATA_BUF
  #define tband_configUSE_METADATA_BUF 1
#endif /* tband_configUSE_METADATA_BUF */

#ifndef tband_configMETADATA_BUF_SIZE
  #define tband_configMETADATA_BUF_SIZE 256
#endif /* tband_configMETADATA_BUF_SIZE */

// == CONFIG: STREAMING BACKEND ================================================

#ifndef tband_configUSE_BACKEND_STREAMING
  #define tband_configUSE_BACKEND_STREAMING 0
#endif /* tband_configUSE_BACKEND_STREAMING */

// == CONFIG: SNAPSHOT BACKEND =================================================

#ifndef tband_configUSE_BACKEND_SNAPSHOT
  #define tband_configUSE_BACKEND_SNAPSHOT 0
#endif /* tband_configUSE_BACKEND_SNAPSHOT */

#ifndef tband_configBACKEND_SNAPSHOT_BUF_SIZE
  #define tband_configBACKEND_SNAPSHOT_BUF_SIZE 32768
#endif /* tband_configBACKEND_SNAPSHOT_BUF_SIZE */

// == CONFIG: POST-MORTEM BACKEND ==============================================

#ifndef tband_configUSE_BACKEND_POST_MORTEM
  #define tband_configUSE_BACKEND_POST_MORTEM 0
#endif /* tband_configUSE_BACKEND_POST_MORTEM */

// == CONFIG: BACKENDS =========================================================

#ifndef tband_configUSE_BACKEND_EXTERNAL
  #define tband_configUSE_BACKEND_EXTERNAL 0
#endif /* tband_configUSE_BACKEND_EXTERNAL */

#if ((tband_configUSE_BACKEND_POST_MORTEM +                                    \
      tband_configUSE_BACKEND_SNAPSHOT +                                       \
      tband_configUSE_BACKEND_STREAMING +                                      \
      tband_configUSE_BACKEND_EXTERNAL) != 1)
  #error "Exactly one backend must be enabled!"
#endif /* only one backend selected */

//                               _    ____ ___
//                              / \  |  _ \_ _|
// =========================   / _ \ | |_) | |  ================================
// =========================  / ___ \|  __/| |  ================================
//                           /_/   \_\_|  |___|

// == API: GENERAL =============================================================

#if (tband_configENABLE == 1)
  bool tband_tracing_enabled(void);
  bool tband_tracing_finished(void);
  bool tband_tracing_backend_finished(unsigned int core_id);
#else /* tband_configENABLE == 1 */
  #define tband_tracing_enabled(void)
  #define tband_tracing_finished(void)
  #define tband_tracing_backend_finished(core_id) (void)(core_id)
#endif /* tband_configENABLE == 1 */

#if (tband_configUSE_METADATA_BUF == 1)
  const volatile uint8_t* tband_get_metadata_buf(unsigned int core_id);
  size_t tband_get_metadata_buf_amnt(unsigned int core_id);
#endif /* (tband_configUSE_METADATA_BUF == 1) */

// == API: STREAMING BACKEND ===================================================

#if (tband_configUSE_BACKEND_STREAMING == 1)
  int tband_start_streaming(void);
  int tband_stop_streaming(void);
#endif /* tband_configUSE_BACKEND_STREAMING == 1 */

// == API: SNAPSHOT BACKEND ====================================================

#if (tband_configUSE_BACKEND_SNAPSHOT == 1)
  int tband_trigger_snapshot(void);
  int tband_stop_snapshot(void);
  int tband_reset_snapshot(void);
  const volatile uint8_t* tband_get_core_snapshot_buf(unsigned int core_id);
  size_t tband_get_core_snapshot_buf_amnt(unsigned int core_id);
#endif /* tband_configUSE_BACKEND_SNAPSHOT == 1 */

// == API: POST-MORTEM BACKEND =================================================

#if (tband_configUSE_BACKEND_POST_MORTEM == 1)
#error TODO
#endif /* tband_configUSE_BACKEND_POST_MORTEM == 1*/

//                _____ ____      _    ____ ___ _   _  ____
//               |_   _|  _ \    / \  / ___|_ _| \ | |/ ___|
// ==============  | | | |_) |  / _ \| |    | ||  \| | |  _  ===================
// ==============  | | |  _ <  / ___ \ |___ | || |\  | |_| | ===================
//                 |_| |_| \_\/_/   \_\____|___|_| \_|\____|

#if (tband_configENABLE == 1)

  // System Metadata:
  void impl_tband_gather_system_metadata(void);
  #define tband_gather_system_metadata(void) impl_tband_gather_system_metadata(void)

  // ISRs:
  #if (tband_configISR_TRACE_ENABLE == 1)
    void impl_tband_isr_name(uint32_t isr_id, const char *name);
    void impl_tband_isr_enter(uint32_t isr_id);
    void impl_tband_isr_exit(uint32_t isr_id);

    /**
     * @brief Name an interrupt.
     * @note This is a metadata event that is added to the metadata buffer, if
     *       it is enabled.
     *
     * @param id interrupt id.
     * @param name name to be given.
     */
    #define tband_isr_name(isr_id, name) impl_tband_isr_name(isr_id, name)

    /**
     * @brief Trace that an interrupt handler has started executing.
     *
     * @param id interrupt id.
     */
    #define tband_isr_enter(isr_id) impl_tband_isr_enter(isr_id)

    /**
     * @brief Trace that an interrupt handler has finished executing.
     *
     * @param id interrupt id.
     */
    #define tband_isr_exit(isr_id) impl_tband_isr_exit(isr_id)
  #endif /* (tband_configISR_TRACE_ENABLE == 1) */

  // Markers:
  #if (tband_configMARKER_TRACE_ENABLE == 1)
    void impl_tband_evtmarker_name(uint32_t id, const char* name);
    void impl_tband_evtmarker(uint32_t id, const char* msg);
    void impl_tband_evtmarker_begin(uint32_t id, const char* msg);
    void impl_tband_evtmarker_end(uint32_t id);
    void impl_tband_valmarker_name(uint32_t id, const char* name);
    void impl_tband_valmarker(uint32_t id, int64_t val);

    /**
     * @brief Name an event marker.
     * @note This is a metadata event that is added to the metadata buffer, if
     *       it is enabled.
     *
     * @param id event marker id.
     * @param name name to be given.
     */
    #define tband_evtmarker_name(id, name) impl_tband_evtmarker_name(id, name)

    /**
     * @brief Trace an instant event.
     *
     * @param id event marker id.
     * @param msg trace event message (optional).
     */
    #define tband_evtmarker(id, msg) impl_tband_evtmarker(id, msg)

    /**
     * @brief Trace beginning of a slice event marker.
     *
     * @param id event marker id.
     * @param msg trace event message (optional).
     */
    #define tband_evtmarker_begin(id, msg) impl_tband_evtmarker_begin(id, msg)

    /**
     * @brief Trace end of a slice event marker.
     *
     * @param msg trace event message (optional).
     */
    #define tband_evtmarker_end(id) impl_tband_evtmarker_end(id)

    /**
     * @brief Name a value marker.
     * @note This is a metadata event that is added to the metadata buffer, if
     *       it is enabled.
     *
     * @param id value marker id.
     * @param name name to be given.
     */
    #define tband_valmarker_name(id, name) impl_tband_valmarker_name(id, name)

    /**
     * @brief Trace a numeric value.
     *
     * @param id value marker id.
     * @param val value to be traced.
     */
    #define tband_valmarker(id, val) impl_tband_valmarker(id, val)

    /**
     * @brief Trace function event marker.
     *
     * @param id value marker id.
     */
    #define tband_fn(id)                                                       \
      static int tband_fn_trace_is_named = 0;                                  \
      static uint32_t tband_fn_trace_id = id;                                  \
      if (!tband_fn_trace_is_named) {                                          \
        tband_evtmarker_name(tband_fn_trace_id, __func__);                     \
        tband_fn_trace_is_named = 1;                                           \
      }                                                                        \
      tband_evtmarker(tband_fn_trace_id, ""); 
  
    /**
     * @brief Trace beginning of a function slice event marker.
     *
     * @param id value marker id.
     */
    #define tband_fn_begin(id)                                                 \
      static int tband_fn_trace_is_named = 0;                                  \
      static uint32_t tband_fn_trace_id = id;                                  \
      if (!tband_fn_trace_is_named) {                                          \
        tband_evtmarker_name(tband_fn_trace_id, __func__);                     \
        tband_fn_trace_is_named = 1;                                           \
      }                                                                        \
      tband_evtmarker_begin(tband_fn_trace_id, ""); 
  
    /**
     * @brief Trace end of a function slice event marker.
     *
     * @param id value marker id.
     */
    #define tband_fn_end() tband_evtmarker_end(tband_fn_trace_id)
  #endif /* (tband_configMARKER_TRACE_ENABLE == 1) */

#endif /* tband_configENABLE == 1*/

// Tracing Modules:
#if (tband_configFREERTOS_TRACE_ENABLE == 1)
  #define tbandPROPER_MODULE_INCLUDE
  #include "tband_freertos.h"
  #undef tbandPROPER_MODULE_INCLUDE
#endif /* (tband_configFREERTOS_TRACE_ENABLE == 1) */

// Provide shims for tracing hooks that are called manually if they are disabled:
#ifndef tband_gather_system_metadata
  #define tband_gather_system_metadata(void)
#endif /* tband_gather_system_metadata */

#ifndef tband_isr_name
  #define tband_isr_name(isr_id, name)
#endif /* tband_isr_exit */

#ifndef tband_isr_enter
  #define tband_isr_enter(isr_id)
#endif /* tband_isr_enter */

#ifndef tband_isr_exit
  #define tband_isr_exit(isr_id)
#endif /* tband_isr_exit */

#ifndef tband_evtmarker_name
  #define tband_evtmarker_name(id, name)
#endif /* tband_evtmarker_name */

#ifndef tband_evtmarker
  #define tband_evtmarker(id, msg)
#endif /* tband_evtmarker */

#ifndef tband_evtmarker_begin
  #define tband_evtmarker_begin(id, msg)
#endif /* tband_evtmarker_begin */

#ifndef tband_evtmarker_end
  #define tband_evtmarker_end(id)
#endif /* tband_evtmarker_end */

#ifndef tband_valmarker_name
  #define tband_valmarker_name(id, name)
#endif /* tband_valmarker_name */

#ifndef tband_valmarker
  #define tband_valmarker(id, val)
#endif /* tband_valmarker */

#ifndef tband_fn
  #define tband_fn(id)
#endif /* tband_fn */

#ifndef tband_fn_begin
  #define tband_fn_begin(id)
#endif /* tband_fn_begin */

#ifndef tband_fn_end
  #define tband_fn_end()
#endif /* tband_fn_end */

#endif /* TBAND_H_ */
// clang-format on
