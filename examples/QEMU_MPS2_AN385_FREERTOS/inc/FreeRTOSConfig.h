/**
 * @file FreeRTOSConfig.h
 * @brief FreeRTOS Configuration for the QEMU "mps2-an385" (Cortex-M3) example.
 * @author Philipp Schilk, 2024-2026
 */
// clang-format off
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdint.h>

// QEMU's mps2-an385 SysTick runs off the 25MHz reference clock used by the
// official FreeRTOS MPS2/QEMU demo.
#define configCPU_CLOCK_HZ                         ( ( unsigned long ) 25000000 )

#define configUSE_PREEMPTION                       1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION    1
#define configUSE_IDLE_HOOK                        0
#define configUSE_TICK_HOOK                        0
#define configUSE_DAEMON_TASK_STARTUP_HOOK         0
#define configTICK_RATE_HZ                         ( 1000 )
#define configMINIMAL_STACK_SIZE                   ( 128 )
#define configTOTAL_HEAP_SIZE                      ( ( size_t ) ( 8 * 1024 ) )
#define configIDLE_SHOULD_YIELD                    1
#define configCHECK_FOR_STACK_OVERFLOW             0
#define configSUPPORT_STATIC_ALLOCATION            1
#define configSUPPORT_DYNAMIC_ALLOCATION           0
#define configMAX_PRIORITIES                       (8)
#define configMAX_TASK_NAME_LEN                    (10)
#define configUSE_TRACE_FACILITY                   1
#define configUSE_16_BIT_TICKS                     0
#define configUSE_MUTEXES                          1
#define configQUEUE_REGISTRY_SIZE                  8
#define configUSE_QUEUE_SETS                       1
#define configUSE_RECURSIVE_MUTEXES                1
#define configUSE_COUNTING_SEMAPHORES              1
#define configMESSAGE_BUFFER_LENGTH_TYPE           size_t

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES           0
#define configMAX_CO_ROUTINE_PRIORITIES (2)

/* Software timer definitions. */
#define configUSE_TIMERS             1
#define configTIMER_TASK_PRIORITY    (2)
#define configTIMER_QUEUE_LENGTH     10
#define configTIMER_TASK_STACK_DEPTH 256

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
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

/* Cortex-M3 interrupt priority configuration. QEMU's mps2-an385 model does not
 * implement all priority bits, so use the widest possible values (as done by
 * the official FreeRTOS MPS2/QEMU demo) rather than a fixed configPRIO_BITS -
 * the port detects the number of implemented bits at runtime. */
#define configKERNEL_INTERRUPT_PRIORITY     ( 255 )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY ( 4 )

#include "semihosting.h"
#define configASSERT( x )    if (!(x)) semihosting_exit(1)

// === Tracing =====================================================================================

// Include tracer:
#ifndef __ASSEMBLER__
#include "tband.h"
#endif

#endif /* FREERTOS_CONFIG_H */
// clang-format on
