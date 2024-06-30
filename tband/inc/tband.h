/** @file tband.h
 * @brief FreeRTOS tracer.
 * @author Philipp Schilk, 2024
 *
 * This file should be included at the end of of FreeRTOSConfig.h
 */
// clang-format off

#ifndef TBAND_H_
#define TBAND_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#ifndef tband_configENABLE
  #define tband_configENABLE 0
#endif /* tband_configENABLE */

#if (tband_configENABLE == 1)

  #include "tband_port.h"

  // == CONFIG: GENERAL ========================================================

  #ifndef tband_configTRACE_DROP_CNT_EVERY
    #define tband_configTRACE_DROP_CNT_EVERY (50)
  #endif /* tband_configTRACE_DROP_CNT_EVERY */

  #ifndef tband_configMAX_STR_LEN
    #define tband_configMAX_STR_LEN (20)
  #endif /* tband_configMAX_STR_LEN */

  #ifndef tband_configMARKER_TRACE_ENABLE
    #define tband_configMARKER_TRACE_ENABLE 1
  #endif /* tband_configMARKER_TRACE_ENABLE */

  #ifndef tband_configISR_TRACE_ENABLE
    #define tband_configISR_TRACE_ENABLE 1
  #endif /* tband_configISR_TRACE_ENABLE */

  #ifndef tband_configTASK_TRACE_ENABLE
    #define tband_configTASK_TRACE_ENABLE 1
  #endif /* tband_configTASK_TRACE_ENABLE */

  #ifndef tband_configQUEUE_TRACE_ENABLE
    #define tband_configQUEUE_TRACE_ENABLE 1
  #endif /* tband_configQUEUE_TRACE_ENABLE */

  #ifndef tband_configSTREAM_BUFFER_TRACE_ENABLE
    #define tband_configSTREAM_BUFFER_TRACE_ENABLE 1
  #endif /* tband_configSTREAM_BUFFER_TRACE_ENABLE */

  #ifndef tband_configFREERTOS_TRACE_ENABLE
    // Config not manually set. Enable FreeRTOS tracing if any FreeRTOS resource is being traced.
    #if ((tband_configTASK_TRACE_ENABLE == 1) || (tband_configQUEUE_TRACE_ENABLE == 1) || (tband_configSTREAM_BUFFER_TRACE_ENABLE == 1))
      #define tband_configFREERTOS_TRACE_ENABLE 1
    #else
      #define tband_configFREERTOS_TRACE_ENABLE 0
    #endif

  #else /* tband_configFREERTOS_TRACE_ENABLE */
    // Config manually set. Ensure FreeRTOS tracing is enabled if any FreeRTOS resource is being traced.
    #if ((tband_configTASK_TRACE_ENABLE == 1) || (tband_configQUEUE_TRACE_ENABLE == 1) || (tband_configSTREAM_BUFFER_TRACE_ENABLE == 1))
      #if (tband_configFREERTOS_TRACE_ENABLE != 1)
        #error "tband_configFREERTOS_TRACE_ENABLE must be enabled if any FreeRTOS resource is being traced!"
      #endif
    #endif

  #endif /* tband_configFREERTOS_TRACE_ENABLE */

  // == CONFIG: METADATA BUF ===================================================

  #ifndef tband_configUSE_METADATA_BUF
    #define tband_configUSE_METADATA_BUF 1
  #endif /* tband_configUSE_METADATA_BUF */

  #ifndef tband_configMETADATA_BUF_SIZE
    #define tband_configMETADATA_BUF_SIZE 256
  #endif /* tband_configMETADATA_BUF_SIZE */

  #if (tband_configUSE_METADATA_BUF == 1)
    const volatile uint8_t* tband_get_metadata_buf(unsigned int core_id);
    size_t tband_get_metadata_buf_amnt(unsigned int core_id);
  #endif /* (tband_configUSE_METADATA_BUF == 1) */

  // == CONFIG: STREAMING BACKEND ===============================================

  #ifndef tband_configUSE_BACKEND_STREAMING
    #define tband_configUSE_BACKEND_STREAMING 0
  #endif /* tband_configUSE_BACKEND_STREAMING */

  // == CONFIG: SNAPSHOT BACKEND ===============================================

  #ifndef tband_configUSE_BACKEND_SNAPSHOT
    #define tband_configUSE_BACKEND_SNAPSHOT 0
  #endif /* tband_configUSE_BACKEND_SNAPSHOT */

  #ifndef tband_configBACKEND_SNAPSHOT_BUF_SIZE
    #define tband_configBACKEND_SNAPSHOT_BUF_SIZE 32768
  #endif /* tband_configBACKEND_SNAPSHOT_BUF_SIZE */

  // == CONFIG: POST-MORTEM BACKEND ============================================

  #ifndef tband_configUSE_BACKEND_POST_MORTEM
    #define tband_configUSE_BACKEND_POST_MORTEM 0
  #endif /* tband_configUSE_BACKEND_POST_MORTEM */

  // == CONFIG: BACKENDS =======================================================

  #ifndef tband_configUSE_BACKEND_EXTERNAL
    #define tband_configUSE_BACKEND_EXTERNAL 0
  #endif /* tband_configUSE_BACKEND_EXTERNAL */

  #if ((tband_configUSE_BACKEND_POST_MORTEM +                                \
        tband_configUSE_BACKEND_SNAPSHOT +                                   \
        tband_configUSE_BACKEND_STREAMING +                                  \
        tband_configUSE_BACKEND_EXTERNAL) != 1)
    #error "Exactly one backend must be enabled!"
  #endif /* only one backend selected */

  // == PORT VALIDATION ========================================================

  #ifndef tband_portTIMESTAMP_RESOLUTION_NS
    #error "tband_portTIMESTAMP_RESOLUTION_NS is not defined!"
  #endif /* tband_portTIMESTAMP_RESOLUTION_NS */

  #ifndef tband_portTIMESTAMP
    #error "tband_portTIMESTAMP is not defined!"
  #endif /* tband_portTIMESTAMP */

  #ifndef tband_portKERNEL_ENTER_CRITICAL_FROM_ANY
    #error "tband_portKERNEL_ENTER_CRITICAL_FROM_ANY is not defined!"
  #endif /* tband_portKERNEL_ENTER_CRITICAL_FROM_ANY */

  #ifndef tband_portKERNEL_EXIT_CRITICAL_FROM_ANY
    #error "tband_portKERNEL_EXIT_CRITICAL_FROM_ANY is not defined!"
  #endif /* tband_portKERNEL_EXIT_CRITICAL_FROM_ANY */

  // == API: GENERAL ===========================================================

  bool tband_tracing_enabled();
  bool tband_tracing_finished();
  bool tband_tracing_backend_finished(unsigned int core_id);

  // == API: STREAMING BACKEND =================================================

  #if (tband_configUSE_BACKEND_STREAMING == 1)

    int tband_start_streaming();

    int tband_stop_streaming();

  #endif /* tband_configUSE_BACKEND_STREAMING == 1 */

  // == API: STREAMING BACKEND =================================================

  #if (tband_configUSE_BACKEND_SNAPSHOT == 1)

    int tband_trigger_snapshot();

    int tband_stop_snapshot();

    int tband_reset_snapshot();

    const volatile uint8_t* tband_get_core_snapshot_buf(unsigned int core_id);

    size_t tband_get_core_snapshot_buf_amnt(unsigned int core_id);

  #endif /* tband_configUSE_BACKEND_SNAPSHOT == 1 */

  // == API: POST-MORTEM BACKEND ===============================================

  #if (tband_configUSE_BACKEND_POST_MORTEM == 1)

  #endif /* tband_configUSE_BACKEND_POST_MORTEM == 1*/

#endif /* tband_configENABLE == 1 */

// =============================================================================

#if (tband_configENABLE == 1)
  #include "tband_hooks.h"
#endif /* tband_configENABLE == 1 */

#endif /* TBAND_H_ */
// clang-format on
