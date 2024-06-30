#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Test framework:
#include "unity.h"
#include "unity_internals.h"

#define configUSE_PREEMPTION                    1
#define configSUPPORT_STATIC_ALLOCATION         1
#define configSUPPORT_DYNAMIC_ALLOCATION        0
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configCPU_CLOCK_HZ                      (SystemCoreClock)
#define configTICK_RATE_HZ                      ((TickType_t)1000)
#define configMAX_PRIORITIES                    (56)
#define configMINIMAL_STACK_SIZE                ((uint16_t)128)
#define configTOTAL_HEAP_SIZE                   ((size_t)3000)
#define configMAX_TASK_NAME_LEN                 (10)
#define configUSE_TRACE_FACILITY                1
#define configUSE_16_BIT_TICKS                  0
#define configUSE_MUTEXES                       1
#define configQUEUE_REGISTRY_SIZE               8
#define configUSE_QUEUE_SETS                    1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configMESSAGE_BUFFER_LENGTH_TYPE size_t

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES           0
#define configMAX_CO_ROUTINE_PRIORITIES (2)

/* Software timer definitions. */
#define configUSE_TIMERS             1
#define configTIMER_TASK_PRIORITY    (2)
#define configTIMER_QUEUE_LENGTH     10
#define configTIMER_TASK_STACK_DEPTH 256

#define INCLUDE_vTaskPrioritySet            1
#define INCLUDE_uxTaskPriorityGet           1
#define INCLUDE_vTaskDelete                 1
#define INCLUDE_vTaskCleanUpResources       0
#define INCLUDE_vTaskSuspend                1
#define INCLUDE_vTaskDelayUntil             1
#define INCLUDE_vTaskDelay                  1
#define INCLUDE_xTaskGetSchedulerState      1
#define INCLUDE_xTimerPendFunctionCall      1
#define INCLUDE_xQueueGetMutexHolder        1
#define INCLUDE_uxTaskGetStackHighWaterMark 1
#define INCLUDE_xTaskGetCurrentTaskHandle   1
#define INCLUDE_eTaskGetState               1
#define INCLUDE_xTaskGetIdleTaskHandle      1

/* The lowest interrupt priority that can be used in a call to a "set priority"
function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15

/* The highest interrupt priority that can be used by any interrupt service
routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
PRIORITY THAN THIS! (higher priorities are lower numeric values. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

#define configASSERT(x) TEST_ASSERT(x)

// === Tracing =================================================================

#define tband_configENABLE (1)

#include "tband_test_config.h"

// Include tracing hooks:
#include "tband.h"

#endif /* FREERTOS_CONFIG_H */
