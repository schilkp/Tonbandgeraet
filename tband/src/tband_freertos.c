/**
 * @file tband_freertos.c
 * @brief Tonbandgeraet FreeRTOS tracer.
 * @author Philipp Schilk, 2024
 * @note Copyright (c) 2024 Philipp Schilk. Released under the MIT license.
 *
 * https://github.com/schilkp/Tonbandgeraet
 */

#include "tband.h"
#if ((tband_configENABLE == 1) && (tband_configFREERTOS_TRACE_ENABLE == 1))

// std:
#include <stdatomic.h>
#include <stdbool.h>

// FreeRTOS:
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#if (configUSE_TIMERS == 1)
#include "timers.h"
#endif /* (configUSE_TIMERS == 1) */

#define tbandPROPER_INTERNAL_INCLUDE
#include "tband_internal.h"
#undef tbandPROPER_INTERNAL_INCLUDE

// == FreeRTOS CONFIG VALIDATION ===============================================

#if (configUSE_TRACE_FACILITY == 0)
#error "configUSE_TRACE_FACILITY is not enabled!"
#endif /* configUSE_TRACE_FACILITY == 0 */

// ===== STATE =================================================================

// Unique ID counters (shared between all cores)
// Note: ID 0 is reserved.
static volatile atomic_ulong next_task_id = 1;
static volatile atomic_ulong next_queue_id = 1;

#if (configUSE_PREEMPTION == 0)
// Track last running task_id per-core
// This is required when preemption is disabled to detect and prevent continuous
// generation of task-switched-in events by the IDLE task.
static volatile uint32_t core_last_task[tband_portNUMBER_OF_CORES] = {0};
#endif /* configUSE_PREEMPTION */

// ===== TRACE HOOKS ===========================================================

// FIXME add version toggle
#if (INCLUDE_xTaskGetIdleTaskHandle != 1)
#error "INCLUDE_xTaskGetIdleTaskHandle is not enabled!"
#endif /* INCLUDE_xTaskGetIdleTaskHandle */

void impl_tband_freertos_scheduler_started_manual(void) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
#if (tband_portNUMBER_OF_CORES > 1)
  {
    for (size_t core_id = 0; core_id < tband_portNUMBER_OF_CORES; core_id++) {
      TaskHandle_t idle_task = xTaskGetIdleTaskHandleForCore(core_id);
      uint32_t task_id = (uint32_t)uxTaskGetTaskNumber(idle_task);
      uint8_t buf[EVT_FREERTOS_TASK_IS_IDLE_TASK_MAXLEN];
      size_t len = encode_freertos_task_is_idle_task(buf, task_id, core_id);
      handle_trace_evt(buf, len, EVT_FREERTOS_TASK_IS_IDLE_TASK_IS_METADATA, ts);
    }
  }
#else  /* tband_portNUMBER_OF_CORES == 1 */
  {
    TaskHandle_t idle_task = xTaskGetIdleTaskHandle();
    uint32_t task_id = (uint32_t)uxTaskGetTaskNumber(idle_task);
    uint8_t buf[EVT_FREERTOS_TASK_IS_IDLE_TASK_MAXLEN];
    size_t len = encode_freertos_task_is_idle_task(buf, task_id, 0);
    handle_trace_evt(buf, len, EVT_FREERTOS_TASK_IS_IDLE_TASK_IS_METADATA, ts);
  }
#endif /* tband_portNUMBER_OF_CORES */
#if (configUSE_TIMERS == 1)
  {
    TaskHandle_t timer_svc = xTimerGetTimerDaemonTaskHandle();
    uint32_t task_id = (uint32_t)uxTaskGetTaskNumber(timer_svc);
    uint8_t buf[EVT_FREERTOS_TASK_IS_TIMER_TASK_MAXLEN];
    size_t len = encode_freertos_task_is_timer_task(buf, task_id);
    handle_trace_evt(buf, len, EVT_FREERTOS_TASK_IS_TIMER_TASK_IS_METADATA, ts);
  }
#endif /* (configUSE_TIMERS == 1) */
  tband_portEXIT_CRITICAL_FROM_ANY();
}

#if (tband_configFREERTOS_TASK_TRACE_ENABLE == 1)
void impl_tband_freertos_task_switched_in(uint32_t task_id) {
  tband_portENTER_CRITICAL_FROM_ANY();

#if (configUSE_PREEMPTION == 0)
  // With preemption disabled, the IDLE task will continuously generate
  // "task-switched-in" events while it is running. Skip repeated
  // task-switched-in events:
  if (core_last_task[tband_portGET_CORE_ID()] == task_id) {
    goto done;
  }
#endif /* configUSE_PREEMPTION */

  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_FREERTOS_TASK_SWITCHED_IN_MAXLEN];
  size_t len = encode_freertos_task_switched_in(buf, ts, task_id);
  handle_trace_evt(buf, len, EVT_FREERTOS_TASK_SWITCHED_IN_IS_METADATA, ts);

#if (configUSE_PREEMPTION == 0)
  core_last_task[tband_portGET_CORE_ID()] = task_id;
done:
#endif /* configUSE_PREEMPTION */

  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* (tband_configFREERTOS_TASK_TRACE_ENABLE == 1) */

#if (tband_configFREERTOS_TASK_TRACE_ENABLE == 1)
void impl_tband_freertos_moved_task_to_ready_state(uint32_t task_id) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_FREERTOS_TASK_TO_RDY_STATE_MAXLEN];
  size_t len = encode_freertos_task_to_rdy_state(buf, ts, task_id);
  handle_trace_evt(buf, len, EVT_FREERTOS_TASK_TO_RDY_STATE_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* (tband_configFREERTOS_TASK_TRACE_ENABLE == 1) */

#if (tband_configFREERTOS_TASK_TRACE_ENABLE == 1)
void impl_tband_freertos_task_resumed(uint32_t task_id) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_FREERTOS_TASK_RESUMED_MAXLEN];
  size_t len = encode_freertos_task_resumed(buf, ts, task_id);
  handle_trace_evt(buf, len, EVT_FREERTOS_TASK_RESUMED_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* (tband_configFREERTOS_TASK_TRACE_ENABLE == 1) */

#if (tband_configFREERTOS_TASK_TRACE_ENABLE == 1)
void impl_tband_freertos_task_resumed_from_isr(uint32_t task_id) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_FREERTOS_TASK_RESUMED_FROM_ISR_MAXLEN];
  size_t len = encode_freertos_task_resumed_from_isr(buf, ts, task_id);
  handle_trace_evt(buf, len, EVT_FREERTOS_TASK_RESUMED_FROM_ISR_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* (tband_configFREERTOS_TASK_TRACE_ENABLE == 1) */

#if (tband_configFREERTOS_TASK_TRACE_ENABLE == 1)
void impl_tband_freertos_task_suspended(uint32_t task_id) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_FREERTOS_TASK_SUSPENDED_MAXLEN];
  size_t len = encode_freertos_task_suspended(buf, ts, task_id);
  handle_trace_evt(buf, len, EVT_FREERTOS_TASK_SUSPENDED_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* (tband_configFREERTOS_TASK_TRACE_ENABLE == 1) */

#if (tband_configFREERTOS_TASK_TRACE_ENABLE == 1)
void impl_tband_freertos_task_delay(uint32_t ticks) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_FREERTOS_CURTASK_DELAY_MAXLEN];
  size_t len = encode_freertos_curtask_delay(buf, ts, ticks);
  handle_trace_evt(buf, len, EVT_FREERTOS_CURTASK_DELAY_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* (tband_configFREERTOS_TASK_TRACE_ENABLE == 1) */

#if (tband_configFREERTOS_TASK_TRACE_ENABLE == 1)
void impl_tband_freertos_task_delay_until(uint32_t time_to_wake) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_FREERTOS_CURTASK_DELAY_UNTIL_MAXLEN];
  size_t len = encode_freertos_curtask_delay_until(buf, ts, time_to_wake);
  handle_trace_evt(buf, len, EVT_FREERTOS_CURTASK_DELAY_UNTIL_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* (tband_configFREERTOS_TASK_TRACE_ENABLE == 1) */

#if (tband_configFREERTOS_TASK_TRACE_ENABLE == 1)
void impl_tband_freertos_task_priority_set(uint32_t task_id, uint32_t priority) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_FREERTOS_TASK_PRIORITY_SET_MAXLEN];
  size_t len = encode_freertos_task_priority_set(buf, ts, task_id, priority);
  handle_trace_evt(buf, len, EVT_FREERTOS_TASK_PRIORITY_SET_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* (tband_configFREERTOS_TASK_TRACE_ENABLE == 1) */

#if (tband_configFREERTOS_TASK_TRACE_ENABLE == 1)
void impl_tband_freertos_task_priority_inherit(uint32_t task_id, uint32_t priority) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_FREERTOS_TASK_PRIORITY_INHERIT_MAXLEN];
  size_t len = encode_freertos_task_priority_inherit(buf, ts, task_id, priority);
  handle_trace_evt(buf, len, EVT_FREERTOS_TASK_PRIORITY_INHERIT_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* (tband_configFREERTOS_TASK_TRACE_ENABLE == 1) */

#if (tband_configFREERTOS_TASK_TRACE_ENABLE == 1)
void impl_tband_freertos_task_priority_disinherit(uint32_t task_id, uint32_t priority) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_FREERTOS_TASK_PRIORITY_DISINHERIT_MAXLEN];
  size_t len = encode_freertos_task_priority_disinherit(buf, ts, task_id, priority);
  handle_trace_evt(buf, len, EVT_FREERTOS_TASK_PRIORITY_DISINHERIT_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* (tband_configFREERTOS_TASK_TRACE_ENABLE == 1) */

#if (tband_configFREERTOS_TRACE_ENABLE == 1)

void impl_tband_freertos_task_create(void *task_handle, uint32_t priority, char *name) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();

  TaskHandle_t task = (TaskHandle_t)task_handle;
  uint32_t task_id = (uint32_t)atomic_fetch_add(&next_task_id, 1);
  vTaskSetTaskNumber(task, (UBaseType_t)task_id);

#if (tband_configFREERTOS_TASK_TRACE_ENABLE == 1)
  {
    uint8_t buf[EVT_FREERTOS_TASK_CREATED_MAXLEN];
    size_t len = encode_freertos_task_created(buf, ts, task_id);
    handle_trace_evt(buf, len, EVT_FREERTOS_TASK_CREATED_IS_METADATA, ts);
  }
  {
    uint8_t buf[EVT_FREERTOS_TASK_PRIORITY_SET_MAXLEN];
    size_t len = encode_freertos_task_priority_set(buf, ts, task_id, priority);
    handle_trace_evt(buf, len, EVT_FREERTOS_TASK_PRIORITY_SET_IS_METADATA, ts);
  }
#endif /* (tband_configFREERTOS_TASK_TRACE_ENABLE == 1) */

  {
    uint8_t buf[EVT_FREERTOS_TASK_NAME_MAXLEN];
    size_t len = encode_freertos_task_name(buf, task_id, name);
    handle_trace_evt(buf, len, EVT_FREERTOS_TASK_NAME_IS_METADATA, ts);
  }
  tband_portEXIT_CRITICAL_FROM_ANY();
}

#endif /* (tband_configFREERTOS_TRACE_ENABLE == 1) */

#if (tband_configFREERTOS_TASK_TRACE_ENABLE == 1)
void impl_tband_freertos_task_deleted(uint32_t task_id) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_FREERTOS_TASK_DELETED_MAXLEN];
  size_t len = encode_freertos_task_deleted(buf, ts, task_id);
  handle_trace_evt(buf, len, EVT_FREERTOS_TASK_DELETED_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* (tband_configFREERTOS_TASK_TRACE_ENABLE == 1) */

#if (tband_configFREERTOS_TRACE_ENABLE == 1)
void impl_tband_freertos_queue_created(void *handle, uint8_t type_val) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();

  QueueHandle_t queue = (QueueHandle_t)handle;
  uint32_t id = (uint32_t)atomic_fetch_add(&next_queue_id, 1);
  vQueueSetQueueNumber(queue, (UBaseType_t)id);

  enum FrQueueKind kind;
  switch (type_val) {
    case queueQUEUE_TYPE_BASE:
      kind = FRQK_QUEUE;
      break;
    case queueQUEUE_TYPE_MUTEX:
      kind = FRQK_MUTEX;
      break;
    case queueQUEUE_TYPE_COUNTING_SEMAPHORE:
      kind = FRQK_COUNTING_SEMPHR;
      break;
    case queueQUEUE_TYPE_BINARY_SEMAPHORE:
      kind = FRQK_BINARY_SEMPHR;
      break;
    case queueQUEUE_TYPE_RECURSIVE_MUTEX:
      kind = FRQK_RECURSIVE_MUTEX;
      break;
      // FIXME add version check
      // #if ( (queueQUEUE_TYPE_SET) != (queueQUEUE_TYPE_BASE))
      //     case queueQUEUE_TYPE_SET:
      //       kind = FRQK_QUEUE_SET;
      //       break;
      // #endif /*(queueQUEUE_TYPE_SET != queueQUEUE_TYPE_BASE) */
    default:
      kind = FRQK_QUEUE;
      break;
  }

  {
    uint8_t buf[EVT_FREERTOS_QUEUE_CREATED_MAXLEN];
    size_t len = encode_freertos_queue_created(buf, ts, id);
    handle_trace_evt(buf, len, EVT_FREERTOS_QUEUE_CREATED_IS_METADATA, ts);
  }

#if (tband_configFREERTOS_QUEUE_TRACE_ENABLE == 1)
  {
    uint8_t buf[EVT_FREERTOS_QUEUE_KIND_MAXLEN];
    size_t len = encode_freertos_queue_kind(buf, id, kind);
    handle_trace_evt(buf, len, EVT_FREERTOS_QUEUE_KIND_IS_METADATA, ts);
  }
  {
    uint8_t buf[EVT_FREERTOS_QUEUE_CUR_LENGTH_MAXLEN];
    size_t len = encode_freertos_queue_cur_length(buf, ts, id, 0);
    handle_trace_evt(buf, len, EVT_FREERTOS_QUEUE_CUR_LENGTH_IS_METADATA, ts);
  }
#endif /* (tband_configFREERTOS_QUEUE_TRACE_ENABLE == 1) */

  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* (tband_configFREERTOS_TRACE_ENABLE == 1) */

#if (tband_configFREERTOS_TRACE_ENABLE == 1)
void impl_tband_freertos_queue_name(void *queue_handle, char *name) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint32_t id = (uint32_t)uxQueueGetQueueNumber((QueueHandle_t)queue_handle);
  uint8_t buf[EVT_FREERTOS_QUEUE_NAME_MAXLEN];
  size_t len = encode_freertos_queue_name(buf, id, name);
  handle_trace_evt(buf, len, EVT_FREERTOS_QUEUE_NAME_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* (tband_configFREERTOS_TRACE_ENABLE == 1) */

#if (tband_configFREERTOS_QUEUE_TRACE_ENABLE == 1)
void impl_tband_freertos_counting_semaphore_create(uint32_t id, uint32_t initial_count) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_FREERTOS_QUEUE_CUR_LENGTH_MAXLEN];
  size_t len = encode_freertos_queue_cur_length(buf, ts, id, initial_count);
  handle_trace_evt(buf, len, EVT_FREERTOS_QUEUE_CUR_LENGTH_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* (tband_configFREERTOS_QUEUE_TRACE_ENABLE == 1) */

#if (tband_configFREERTOS_QUEUE_TRACE_ENABLE == 1)
void impl_tband_freertos_queue_send(uint32_t id, uint32_t copy_position, uint32_t size_before) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  if (copy_position == queueOVERWRITE) {
    uint8_t buf[EVT_FREERTOS_QUEUE_OVERWRITE_MAXLEN];
    size_t len = encode_freertos_queue_overwrite(buf, ts, id, size_before);
    handle_trace_evt(buf, len, EVT_FREERTOS_QUEUE_OVERWRITE_IS_METADATA, ts);
  } else {
    uint8_t buf[EVT_FREERTOS_QUEUE_SEND_MAXLEN];
    size_t len = encode_freertos_queue_send(buf, ts, id, size_before + 1);
    handle_trace_evt(buf, len, EVT_FREERTOS_QUEUE_SEND_IS_METADATA, ts);
  }
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* (tband_configFREERTOS_QUEUE_TRACE_ENABLE == 1) */

#if (tband_configFREERTOS_QUEUE_TRACE_ENABLE == 1)
void impl_tband_freertos_queue_send_from_isr(uint32_t id, uint32_t copy_position,
                                             uint32_t size_before) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  if (copy_position == queueOVERWRITE) {
    uint8_t buf[EVT_FREERTOS_QUEUE_OVERWRITE_FROM_ISR_MAXLEN];
    size_t len = encode_freertos_queue_overwrite_from_isr(buf, ts, id, size_before);
    handle_trace_evt(buf, len, EVT_FREERTOS_QUEUE_OVERWRITE_FROM_ISR_IS_METADATA, ts);
  } else {
    uint8_t buf[EVT_FREERTOS_QUEUE_SEND_FROM_ISR_MAXLEN];
    size_t len = encode_freertos_queue_send_from_isr(buf, ts, id, size_before + 1);
    handle_trace_evt(buf, len, EVT_FREERTOS_QUEUE_SEND_FROM_ISR_IS_METADATA, ts);
  }
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* (tband_configFREERTOS_QUEUE_TRACE_ENABLE == 1) */

#if (tband_configFREERTOS_TASK_TRACE_ENABLE == 1)
void impl_tband_freertos_blocking_on_queue_send(uint32_t queue_id, uint32_t ticks_to_wait) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_FREERTOS_CURTASK_BLOCK_ON_QUEUE_SEND_MAXLEN];
  size_t len = encode_freertos_curtask_block_on_queue_send(buf, ts, queue_id, ticks_to_wait);
  handle_trace_evt(buf, len, EVT_FREERTOS_CURTASK_BLOCK_ON_QUEUE_SEND_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* (tband_configFREERTOS_TASK_TRACE_ENABLE == 1) */

#if (tband_configFREERTOS_QUEUE_TRACE_ENABLE == 1)
void impl_tband_freertos_queue_receive(uint32_t id, uint32_t size_before) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint32_t new_size = (size_before == 0) ? 0 : size_before - 1;
  uint8_t buf[EVT_FREERTOS_QUEUE_RECEIVE_MAXLEN];
  size_t len = encode_freertos_queue_receive(buf, ts, id, new_size);
  handle_trace_evt(buf, len, EVT_FREERTOS_QUEUE_RECEIVE_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* (tband_configFREERTOS_QUEUE_TRACE_ENABLE == 1) */

#if (tband_configFREERTOS_QUEUE_TRACE_ENABLE == 1)
void impl_tband_freertos_queue_receive_from_isr(uint32_t id, uint32_t size_before) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint32_t new_size = (size_before == 0) ? 0 : size_before - 1;
  uint8_t buf[EVT_FREERTOS_QUEUE_RECEIVE_FROM_ISR_MAXLEN];
  size_t len = encode_freertos_queue_receive_from_isr(buf, ts, id, new_size);
  handle_trace_evt(buf, len, EVT_FREERTOS_QUEUE_RECEIVE_FROM_ISR_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* (tband_configFREERTOS_QUEUE_TRACE_ENABLE == 1) */

#if (tband_configFREERTOS_TASK_TRACE_ENABLE == 1)
void impl_tband_freertos_blocking_on_queue_receive(uint32_t queue_id, uint32_t ticks_to_wait) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_FREERTOS_CURTASK_BLOCK_ON_QUEUE_RECEIVE_MAXLEN];
  size_t len = encode_freertos_curtask_block_on_queue_receive(buf, ts, queue_id, ticks_to_wait);
  handle_trace_evt(buf, len, EVT_FREERTOS_CURTASK_BLOCK_ON_QUEUE_RECEIVE_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* (tband_configFREERTOS_TASK_TRACE_ENABLE == 1) */

#if (tband_configFREERTOS_QUEUE_TRACE_ENABLE == 1)
void impl_tband_freertos_queue_reset(uint32_t id) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_FREERTOS_QUEUE_RESET_MAXLEN];
  size_t len = encode_freertos_queue_reset(buf, ts, id);
  handle_trace_evt(buf, len, EVT_FREERTOS_QUEUE_RESET_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* (tband_configFREERTOS_QUEUE_TRACE_ENABLE == 1) */

#if (tband_configFREERTOS_TASK_TRACE_ENABLE == 1)
void impl_tband_freertos_blocking_on_queue_peek(uint32_t queue_id, uint32_t ticks_to_wait) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_FREERTOS_CURTASK_BLOCK_ON_QUEUE_PEEK_MAXLEN];
  size_t len = encode_freertos_curtask_block_on_queue_peek(buf, ts, queue_id, ticks_to_wait);
  handle_trace_evt(buf, len, EVT_FREERTOS_CURTASK_BLOCK_ON_QUEUE_PEEK_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* (tband_configFREERTOS_TASK_TRACE_ENABLE == 1) */

#if ((tband_configMARKER_TRACE_ENABLE == 1) && (tband_configFREERTOS_TRACE_ENABLE == 1))
void impl_tband_freertos_task_evtmarker_name(uint32_t id, const char *name) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint32_t task_id = (uint32_t)uxTaskGetTaskNumber(0);
  uint8_t buf[EVT_FREERTOS_TASK_EVTMARKER_NAME_MAXLEN];
  size_t len = encode_freertos_task_evtmarker_name(buf, id, task_id, name);
  handle_trace_evt(buf, len, EVT_FREERTOS_TASK_EVTMARKER_NAME_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* tband_configMARKER_TRACE_ENABLE == 1 && tband_configFREERTOS_TRACE_ENABLE == 1 */

#if ((tband_configMARKER_TRACE_ENABLE == 1) && (tband_configFREERTOS_TRACE_ENABLE == 1))
void impl_tband_freertos_task_evtmarker(uint32_t id, const char *msg) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_FREERTOS_TASK_EVTMARKER_MAXLEN];
  size_t len = encode_freertos_task_evtmarker(buf, ts, id, msg);
  handle_trace_evt(buf, len, EVT_FREERTOS_TASK_EVTMARKER_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* tband_configMARKER_TRACE_ENABLE == 1 && tband_configFREERTOS_TRACE_ENABLE == 1 */

#if ((tband_configMARKER_TRACE_ENABLE == 1) && (tband_configFREERTOS_TRACE_ENABLE == 1))
void impl_tband_freertos_task_evtmarker_begin(uint32_t id, const char *msg) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_FREERTOS_TASK_EVTMARKER_BEGIN_MAXLEN];
  size_t len = encode_freertos_task_evtmarker_begin(buf, ts, id, msg);
  handle_trace_evt(buf, len, EVT_FREERTOS_TASK_EVTMARKER_BEGIN_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* tband_configMARKER_TRACE_ENABLE == 1 && tband_configFREERTOS_TRACE_ENABLE == 1 */

#if ((tband_configMARKER_TRACE_ENABLE == 1) && (tband_configFREERTOS_TRACE_ENABLE == 1))
void impl_tband_freertos_task_evtmarker_end(uint32_t id) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_FREERTOS_TASK_EVTMARKER_END_MAXLEN];
  size_t len = encode_freertos_task_evtmarker_end(buf, ts, id);
  handle_trace_evt(buf, len, EVT_FREERTOS_TASK_EVTMARKER_END_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* tband_configMARKER_TRACE_ENABLE == 1 && tband_configFREERTOS_TRACE_ENABLE == 1 */

#if ((tband_configMARKER_TRACE_ENABLE == 1) && (tband_configFREERTOS_TRACE_ENABLE == 1))
void impl_tband_freertos_task_valmarker_name(uint32_t id, const char *name) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint32_t task_id = (uint32_t)uxTaskGetTaskNumber(0);
  uint8_t buf[EVT_FREERTOS_TASK_VALMARKER_NAME_MAXLEN];
  size_t len = encode_freertos_task_valmarker_name(buf, id, task_id, name);
  handle_trace_evt(buf, len, EVT_FREERTOS_TASK_VALMARKER_NAME_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* tband_configMARKER_TRACE_ENABLE == 1 && tband_configFREERTOS_TRACE_ENABLE == 1 */

#if ((tband_configMARKER_TRACE_ENABLE == 1) && (tband_configFREERTOS_TRACE_ENABLE == 1))
void impl_tband_freertos_task_valmarker(uint32_t id, int64_t val) {
  tband_portENTER_CRITICAL_FROM_ANY();
  uint64_t ts = tband_portTIMESTAMP();
  uint8_t buf[EVT_FREERTOS_TASK_VALMARKER_MAXLEN];
  size_t len = encode_freertos_task_valmarker(buf, ts, id, val);
  handle_trace_evt(buf, len, EVT_FREERTOS_TASK_VALMARKER_IS_METADATA, ts);
  tband_portEXIT_CRITICAL_FROM_ANY();
}
#endif /* tband_configMARKER_TRACE_ENABLE == 1 && tband_configFREERTOS_TRACE_ENABLE == 1 */

#endif /* ((tband_configENABLE == 1) && (tband_configFREERTOS_TRACE_ENABLE == 1)) */
