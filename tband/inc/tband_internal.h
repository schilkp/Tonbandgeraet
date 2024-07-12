/**
 * @file tband_internal.h
 * @brief Internal APIs.
 * @note Copyright (c) 2024 Philipp Schilk. Released under the MIT license.
 * @author Philipp Schilk, 2024
 */
#ifndef TBAND_INTERNAL_H_
#define TBAND_INTERNAL_H_
// clang-format off

#ifndef tbandPROPER_INTERNAL_INCLUDE
  #error "This internal header file is not a public API and should be be included. Include tband.h instead."
#endif /* tbandPROPER_INTERNAL_INCLUDE */

// ===== Internal APIs =========================================================

bool tband_submit_to_backend(uint8_t *buf, size_t len, bool is_metadata);

void handle_trace_evt(uint8_t *buf, size_t len, bool is_metadata, uint64_t ts);

// ===== Port ==================================================================

#include "tband_port.h"

#ifndef tband_portTIMESTAMP_RESOLUTION_NS
  #error "tband_portTIMESTAMP_RESOLUTION_NS is not defined!"
#endif /* tband_portTIMESTAMP_RESOLUTION_NS */

#ifndef tband_portTIMESTAMP
  #error "tband_portTIMESTAMP is not defined!"
#endif /* tband_portTIMESTAMP */

#ifndef tband_portENTER_CRITICAL_FROM_ANY
  #error "tband_portENTER_CRITICAL_FROM_ANY is not defined!"
#endif /* tband_portENTER_CRITICAL_FROM_ANY */

#ifndef tband_portEXIT_CRITICAL_FROM_ANY
  #error "tband_portEXIT_CRITICAL_FROM_ANY is not defined!"
#endif /* tband_portEXIT_CRITICAL_FROM_ANY */

#ifndef tband_portNUMBER_OF_CORES
  #define tband_portNUMBER_OF_CORES (1)
#endif /* tband_portNUMBER_OF_CORES */

#ifndef tband_portGET_CORE_ID
  #if (tband_portNUMBER_OF_CORES == 1)
    #define tband_portGET_CORE_ID() (0)
  #else /* (tband_portNUMBER_OF_CORES == 1) */
    #error "tband_portGET_CORE_ID is not defined!"
  #endif /* (tband_portNUMBER_OF_CORES == 1) */
#endif /* tband_portGET_CORE_ID */

// ===== Encode ================================================================

#define tbandPROPER_INTERNAL_INCLUDE
#include "tband_encode.h"
#undef tbandPROPER_INTERNAL_INCLUDE

// clang-format on
#endif /* TBAND_INTERNAL_H_ */
