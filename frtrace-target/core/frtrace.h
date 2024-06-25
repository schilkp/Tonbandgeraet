/** @file frtrace.h
 * @brief FreeRTOS tracer.
 * @author Philipp Schilk, 2024
 *
 * This file should be included at the end of of FreeRTOSConfig.h
 */
// clang-format off

#ifndef FRTRACE_H_
#define FRTRACE_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#ifndef frtrace_configENABLE
  #define frtrace_configENABLE 0
#endif /* frtrace_configENABLE */

#if (frtrace_configENABLE == 1)

  #include "frtrace_port.h"

  // == CONFIG: GENERAL ========================================================

  #ifndef frtrace_configTRACE_DROP_CNT_EVERY
    #define frtrace_configTRACE_DROP_CNT_EVERY (50)
  #endif /* frtrace_configTRACE_DROP_CNT_EVERY */

  #ifndef frtrace_configMAX_STR_LEN
    #define frtrace_configMAX_STR_LEN (20)
  #endif /* frtrace_configMAX_STR_LEN */

  #ifndef frtrace_configMARKER_TRACE_ENABLE
    #define frtrace_configMARKER_TRACE_ENABLE 1
  #endif /* frtrace_configMARKER_TRACE_ENABLE */

  #ifndef frtrace_configISR_TRACE_ENABLE
    #define frtrace_configISR_TRACE_ENABLE 1
  #endif /* frtrace_configISR_TRACE_ENABLE */

  #ifndef frtrace_configTASK_TRACE_ENABLE
    #define frtrace_configTASK_TRACE_ENABLE 1
  #endif /* frtrace_configTASK_TRACE_ENABLE */

  #ifndef frtrace_configQUEUE_TRACE_ENABLE
    #define frtrace_configQUEUE_TRACE_ENABLE 1
  #endif /* frtrace_configQUEUE_TRACE_ENABLE */

  #ifndef frtrace_configSTREAM_BUFFER_TRACE_ENABLE
    #define frtrace_configSTREAM_BUFFER_TRACE_ENABLE 1
  #endif /* frtrace_configSTREAM_BUFFER_TRACE_ENABLE */

  #ifndef frtrace_configFREERTOS_TRACE_ENABLE
    // Config not manually set. Enable FreeRTOS tracing if any FreeRTOS resource is being traced.
    #if ((frtrace_configTASK_TRACE_ENABLE == 1) || (frtrace_configQUEUE_TRACE_ENABLE == 1) || (frtrace_configSTREAM_BUFFER_TRACE_ENABLE == 1))
      #define frtrace_configFREERTOS_TRACE_ENABLE 1
    #else
      #define frtrace_configFREERTOS_TRACE_ENABLE 0
    #endif

  #else /* frtrace_configFREERTOS_TRACE_ENABLE */
    // Config manually set. Ensure FreeRTOS tracing is enabled if any FreeRTOS resource is being traced.
    #if ((frtrace_configTASK_TRACE_ENABLE == 1) || (frtrace_configQUEUE_TRACE_ENABLE == 1) || (frtrace_configSTREAM_BUFFER_TRACE_ENABLE == 1))
      #if (frtrace_configFREERTOS_TRACE_ENABLE != 1)
        #error "frtrace_configFREERTOS_TRACE_ENABLE must be enabled if any FreeRTOS resource is being traced!"
      #endif
    #endif

  #endif /* frtrace_configFREERTOS_TRACE_ENABLE */

  // == CONFIG: METADATA BUF ===================================================

  #ifndef frtrace_configUSE_METADATA_BUF
    #define frtrace_configUSE_METADATA_BUF 1
  #endif /* frtrace_configUSE_METADATA_BUF */

  #ifndef frtrace_configMETADATA_BUF_SIZE
    #define frtrace_configMETADATA_BUF_SIZE 256
  #endif /* frtrace_configMETADATA_BUF_SIZE */

  #if (frtrace_configUSE_METADATA_BUF == 1)
    const volatile uint8_t* frtrace_get_metadata_buf(unsigned int core_id);
    size_t frtrace_get_metadata_buf_amnt(unsigned int core_id);
  #endif /* (frtrace_configUSE_METADATA_BUF == 1) */

  // == CONFIG: STREAMING BACKEND ===============================================

  #ifndef frtrace_configUSE_BACKEND_STREAMING
    #define frtrace_configUSE_BACKEND_STREAMING 0
  #endif /* frtrace_configUSE_BACKEND_STREAMING */

  // == CONFIG: SNAPSHOT BACKEND ===============================================

  #ifndef frtrace_configUSE_BACKEND_SNAPSHOT
    #define frtrace_configUSE_BACKEND_SNAPSHOT 0
  #endif /* frtrace_configUSE_BACKEND_SNAPSHOT */

  #ifndef frtrace_configBACKEND_SNAPSHOT_BUF_SIZE
    #define frtrace_configBACKEND_SNAPSHOT_BUF_SIZE 32768
  #endif /* frtrace_configBACKEND_SNAPSHOT_BUF_SIZE */

  // == CONFIG: POST-MORTEM BACKEND ============================================

  #ifndef frtrace_configUSE_BACKEND_POST_MORTEM
    #define frtrace_configUSE_BACKEND_POST_MORTEM 0
  #endif /* frtrace_configUSE_BACKEND_POST_MORTEM */

  // == CONFIG: BACKENDS =======================================================

  #ifndef frtrace_configUSE_BACKEND_EXTERNAL
    #define frtrace_configUSE_BACKEND_EXTERNAL 0
  #endif /* frtrace_configUSE_BACKEND_EXTERNAL */

  #if ((frtrace_configUSE_BACKEND_POST_MORTEM +                                \
        frtrace_configUSE_BACKEND_SNAPSHOT +                                   \
        frtrace_configUSE_BACKEND_STREAMING +                                  \
        frtrace_configUSE_BACKEND_EXTERNAL) != 1)
    #error "Exactly one backend must be enabled!"
  #endif /* only one backend selected */

  // == PORT VALIDATION ========================================================

  #ifndef frtrace_portTIMESTAMP_RESOLUTION_NS
    #error "frtrace_portTIMESTAMP_RESOLUTION_NS is not defined!"
  #endif /* frtrace_portTIMESTAMP_RESOLUTION_NS */

  #ifndef frtrace_portTIMESTAMP
    #error "frtrace_portTIMESTAMP is not defined!"
  #endif /* frtrace_portTIMESTAMP */

  #ifndef frtrace_portKERNEL_ENTER_CRITICAL_FROM_ANY
    #error "frtrace_portKERNEL_ENTER_CRITICAL_FROM_ANY is not defined!"
  #endif /* frtrace_portKERNEL_ENTER_CRITICAL_FROM_ANY */

  #ifndef frtrace_portKERNEL_EXIT_CRITICAL_FROM_ANY
    #error "frtrace_portKERNEL_EXIT_CRITICAL_FROM_ANY is not defined!"
  #endif /* frtrace_portKERNEL_EXIT_CRITICAL_FROM_ANY */

  // == API: GENERAL ===========================================================

  bool frtrace_tracing_enabled();
  bool frtrace_tracing_finished();
  bool frtrace_tracing_backend_finished(unsigned int core_id);

  // == API: STREAMING BACKEND =================================================

  #if (frtrace_configUSE_BACKEND_STREAMING == 1)

    int frtrace_start_streaming();

    int frtrace_stop_streaming();

  #endif /* frtrace_configUSE_BACKEND_STREAMING == 1 */

  // == API: STREAMING BACKEND =================================================

  #if (frtrace_configUSE_BACKEND_SNAPSHOT == 1)

    int frtrace_trigger_snapshot();

    int frtrace_stop_snapshot();

    int frtrace_reset_snapshot();

    const volatile uint8_t* frtrace_get_core_snapshot_buf(unsigned int core_id);

    size_t frtrace_get_core_snapshot_buf_amnt(unsigned int core_id);

  #endif /* frtrace_configUSE_BACKEND_SNAPSHOT == 1 */

  // == API: POST-MORTEM BACKEND ===============================================

  #if (frtrace_configUSE_BACKEND_POST_MORTEM == 1)

  #endif /* frtrace_configUSE_BACKEND_POST_MORTEM == 1*/

#endif /* frtrace_configENABLE == 1 */

// =============================================================================

#if (frtrace_configENABLE == 1)
  #include "frtrace_hooks.h"
#endif /* frtrace_configENABLE == 1 */

#endif /* FRTRACE_H_ */
// clang-format on
