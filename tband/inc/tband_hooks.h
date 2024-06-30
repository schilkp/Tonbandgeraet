/**
 * @file tband_hooks.h
 * @brief FreeRTOS tracer hooks
 * @author Philipp Schilk, 2024
 */
// clang-format off
#ifndef TBAND_HOOKS_H_
#define TBAND_HOOKS_H_

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

#if (tband_configENABLE == 1)

  // == Scheduler Metadata =====================================================

  // Manual implementation if traceSCHEDULER_STARTED does not exist:
  // FIXME: Add version toggle!
  // FIXME: Move this check to source file?
  #if (INCLUDE_xTaskGetIdleTaskHandle != 1 )
    #error "INCLUDE_xTaskGetIdleTaskHandle is not enabled!"
  #endif /* INCLUDE_xTaskGetIdleTaskHandle */

  // FIXME: Documentation (Called after Scheduler started if any freertos tracing,
  //        automatically called in later versions).
  void impl_tband_gather_system_metadata();
  #define tband_gather_system_metadata() impl_tband_gather_system_metadata()

  // == Task Switched In =======================================================

  #if (tband_configTASK_TRACE_ENABLE == 1)
    // Note: When called, `pxCurrentTCB` contains handle of task about to run.
    void impl_tband_freertos_task_switched_in(uint32_t task_id);
    #define traceTASK_SWITCHED_IN() impl_tband_freertos_task_switched_in(                                                     \
        (uint32_t)(pxCurrentTCB)->uxTaskNumber /* task id */                                                           \
      )
  #endif /* (tband_configTASK_TRACE_ENABLE == 1) */

  // == Task Switched Out ======================================================

  #if (tband_configTASK_TRACE_ENABLE == 1)
    // Note: When called, `pxCurrentTCB` contains handle of task about to run.
    #define traceTASK_SWITCHED_OUT()
  #endif /* (tband_configTASK_TRACE_ENABLE == 1) */

  // == Task to Ready State ====================================================

  #if (tband_configTASK_TRACE_ENABLE == 1)
    void impl_tband_freertos_moved_task_to_ready_state(uint32_t task_id);
    #define traceMOVED_TASK_TO_READY_STATE(pxTask) impl_tband_freertos_moved_task_to_ready_state(                             \
        (uint32_t)(pxTask)->uxTaskNumber /* task id */                                                                 \
      )
  #endif /* (tband_configTASK_TRACE_ENABLE == 1) */

  // == Task Resume ============================================================

  #if (tband_configTASK_TRACE_ENABLE == 1)
    void impl_tband_freertos_task_resumed(uint32_t task_id);
    #define traceTASK_RESUME(pxTask) impl_tband_freertos_task_resumed(                                                        \
        (uint32_t)(pxTask)->uxTaskNumber /* task id */                                                                 \
      )
  #endif /* (tband_configTASK_TRACE_ENABLE == 1) */

  #if (tband_configTASK_TRACE_ENABLE == 1)
    void impl_tband_freertos_task_resumed_from_isr(uint32_t task_id);
    #define traceTASK_RESUME_FROM_ISR(pxTask) impl_tband_freertos_task_resumed_from_isr(                                      \
        (uint32_t)(pxTask)->uxTaskNumber /* task id */                                                                 \
      )
  #endif /* (tband_configTASK_TRACE_ENABLE == 1) */

  // == Task Suspended =========================================================

  #if (tband_configTASK_TRACE_ENABLE == 1)
    void impl_tband_freertos_task_suspended(uint32_t task_id);
    #define traceTASK_SUSPEND(pxTask) impl_tband_freertos_task_suspended(                                                     \
        (uint32_t)(pxTask)->uxTaskNumber /* task id */                                                                 \
      )
  #endif /* (tband_configTASK_TRACE_ENABLE == 1) */

  // == Task Delayed ===========================================================

  // FIXME: Add version toggle!

  #if (tband_configTASK_TRACE_ENABLE == 1)
    void impl_tband_freertos_task_delay(uint32_t ticks);
    #define traceTASK_DELAY() impl_tband_freertos_task_delay((uint32_t) xTicksToDelay)
  #endif /* (tband_configTASK_TRACE_ENABLE == 1) */

  #if (tband_configTASK_TRACE_ENABLE == 1)
    void impl_tband_freertos_task_delay_until(uint32_t time_to_wake);
    #define traceTASK_DELAY_UNTIL(xTimeToWake) impl_tband_freertos_task_delay_until(xTimeToWake)
  #endif /* (tband_configTASK_TRACE_ENABLE == 1) */

  // == Task Priority Set ======================================================

  #if (tband_configTASK_TRACE_ENABLE == 1)
    void impl_tband_freertos_task_priority_set(uint32_t task_id, uint32_t priority);
    #define traceTASK_PRIORITY_SET(pxTask, uxNewPriority) impl_tband_freertos_task_priority_set(                              \
        (uint32_t)(pxTask)->uxTaskNumber /* task id */,                                                                \
        (uint32_t)(uxNewPriority) /* priority */                                                                       \
      )
  #endif /* (tband_configTASK_TRACE_ENABLE == 1) */

  // == Task Priority Inherited ================================================

  #if (tband_configTASK_TRACE_ENABLE == 1)
    void impl_tband_freertos_task_priority_inherit(uint32_t task_id, uint32_t priority);
    #define traceTASK_PRIORITY_INHERIT(pxTCBOfMutexHolder, uxInheritedPriority) impl_tband_freertos_task_priority_inherit(    \
        (uint32_t)(pxTCBOfMutexHolder)->uxTaskNumber /* task id */,                                                    \
        (uint32_t)(uxInheritedPriority) /* priority */                                                                 \
      )
  #endif /* (tband_configTASK_TRACE_ENABLE == 1) */

  // == Task Priority Disinherited =============================================

  #if (tband_configTASK_TRACE_ENABLE == 1)
    void impl_tband_freertos_task_priority_disinherit(uint32_t task_id, uint32_t priority);
    #define traceTASK_PRIORITY_DISINHERIT(pxTCBOfMutexHolder, uxOriginalPriority) impl_tband_freertos_task_priority_disinherit( \
        (uint32_t)(pxTCBOfMutexHolder)->uxTaskNumber /* task id */,                                                      \
        (uint32_t)(uxOriginalPriority) /* priority */                                                                    \
      )
  #endif /* (tband_configTASK_TRACE_ENABLE == 1) */

  // == Task Created ===========================================================

  #if (tband_configFREERTOS_TRACE_ENABLE == 1)
    void impl_tband_freertos_task_create(void * task_handle, uint32_t priority, char *name);
    #define traceTASK_CREATE(pxTask) impl_tband_freertos_task_create(                                                         \
        (void *)(pxTask) /* task handle */,                                                                            \
        (uint32_t)(pxTask)->uxPriority /* priority */,                                                                 \
        (pxTask)->pcTaskName /* name */                                                                                \
      )
  #endif /* (tband_configFREERTOS_TRACE_ENABLE == 1) */


  // == Task Deleted ===========================================================

  #if (tband_configTASK_TRACE_ENABLE == 1)
    void impl_tband_freertos_task_deleted(uint32_t task_id);
    #define traceTASK_DELETE(pxTask) impl_tband_freertos_task_deleted(                                                        \
        (pxTask)->uxTaskNumber  /* task id */                                                                          \
      )
  #endif /* (tband_configTASK_TRACE_ENABLE == 1) */

  // == ISRs ===================================================================

  #if (tband_configISR_TRACE_ENABLE == 1)
    void impl_tband_isr_name(uint32_t isr_id, char *name);
    #define tband_isr_name(isr_id, name) impl_tband_isr_name(isr_id, name)
  #endif /* (tband_configISR_TRACE_ENABLE == 1) */

  #if (tband_configISR_TRACE_ENABLE == 1)
    void impl_tband_isr_enter(uint32_t isr_id);
    #define tband_isr_enter(isr_id) impl_tband_isr_enter(isr_id)
  #endif /* (tband_configISR_TRACE_ENABLE == 1) */

  #if (tband_configISR_TRACE_ENABLE == 1)
    void impl_tband_isr_exit(uint32_t isr_id);
    #define tband_isr_exit(isr_id) impl_tband_isr_exit(isr_id)
  #endif /* (tband_configISR_TRACE_ENABLE == 1) */

  // == Queue Created ==========================================================

  #if (tband_configFREERTOS_TRACE_ENABLE == 1)
    void impl_tband_freertos_queue_created(void *handle, uint8_t type_val);
    #define traceQUEUE_CREATE(pxNewQueue) impl_tband_freertos_queue_created(                                                  \
        (void *)(pxNewQueue),               /* queue handle  */                                                        \
        (uint8_t)(pxNewQueue)->ucQueueType  /* type */                                                                 \
      )
  #endif /* (tband_configFREERTOS_TRACE_ENABLE == 1) */

  // == Queue Name =============================================================

  #if (tband_configFREERTOS_TRACE_ENABLE == 1)
  void impl_tband_freertos_queue_name(void *queue_handle, char *name);
    #define tband_freertos_queue_name(handle, name) impl_tband_freertos_queue_name((void *)(handle), (name))
    #define tband_freertos_binary_semaphore_name(handle, name) tband_freertos_queue_name((handle), (name))
    #define tband_freertos_counting_semaphore_name(handle, name) tband_freertos_queue_name((handle), (name))
    #define tband_freertos_mutex_name(handle, name) tband_freertos_queue_name((handle), (name))
    #define tband_freertos_recursive_mutex_name(handle, name) tband_freertos_queue_name((handle), (name))
  #endif /* (tband_configFREERTOS_TRACE_ENABLE == 1) */

  // == Queue Send =============================================================

  // FIXME: Add version toggle!
  #if (tband_configQUEUE_TRACE_ENABLE == 1)
    void impl_tband_freertos_queue_send(uint32_t id, uint32_t copy_position, uint32_t size_before);
    #define traceQUEUE_SEND(pxQueue) impl_tband_freertos_queue_send(                                                          \
        (uint32_t)(pxQueue)->uxQueueNumber /* queue id */,                                                             \
        queueSEND_TO_BACK /* copy position */,                                                                         \
        (uint32_t)(pxQueue)->uxMessagesWaiting /* size before */                                                       \
      )
  #endif /* (tband_configQUEUE_TRACE_ENABLE == 1) */

  // FIXME: Add version toggle! Should this be a seperate event?
  // Re-route set send to queue send:
  #if (tband_configQUEUE_TRACE_ENABLE == 1)
    #define traceQUEUE_SET_SEND(pxQueue) impl_tband_freertos_queue_send(                                                      \
        (uint32_t)(pxQueue)->uxQueueNumber /* queue id */,                                                             \
        queueSEND_TO_BACK /* copy position */,                                                                         \
        (uint32_t)(pxQueue)->uxMessagesWaiting /* size before */                                                       \
      )
  #endif /* (tband_configQUEUE_TRACE_ENABLE == 1) */

  // FIXME: Add version toggle! queueSEND_TO_BACK is not fixed!
  #if (tband_configQUEUE_TRACE_ENABLE == 1)
  void impl_tband_freertos_queue_send_from_isr(uint32_t id, uint32_t copy_position, uint32_t size_before);
    #define traceQUEUE_SEND_FROM_ISR(pxQueue) impl_tband_freertos_queue_send_from_isr(                                        \
        (uint32_t)(pxQueue)->uxQueueNumber /* queue id */,                                                             \
        queueSEND_TO_BACK /* copy position */,                                                                         \
        (uint32_t)(pxQueue)->uxMessagesWaiting /* size before */                                                       \
      )
  #endif /* (tband_configQUEUE_TRACE_ENABLE == 1) */

  // FIXME: Add version toggle!
  #if (tband_configTASK_TRACE_ENABLE == 1)
    void impl_tband_freertos_blocking_on_queue_send(uint32_t queue_id, uint32_t ticks_to_wait);
    #define traceBLOCKING_ON_QUEUE_SEND(pxQueue) impl_tband_freertos_blocking_on_queue_send(                                  \
        (uint32_t)(pxQueue)->uxQueueNumber /* queue id */,                                                             \
        (uint32_t)(xTicksToWait) /* ticks to wait */                                                                   \
      )
  #endif /* (tband_configTASK_TRACE_ENABLE == 1) */

  // == Queue Receive ==========================================================

  #if (tband_configQUEUE_TRACE_ENABLE == 1)
    void impl_tband_freertos_queue_receive(uint32_t id, uint32_t size_before);
    #define traceQUEUE_RECEIVE(pxQueue) impl_tband_freertos_queue_receive(                                                    \
        (uint32_t)(pxQueue)->uxQueueNumber /* queue id */,                                                             \
        (uint32_t)(pxQueue)->uxMessagesWaiting /* size before */                                                       \
      )
  #endif /* (tband_configQUEUE_TRACE_ENABLE == 1) */

  #if (tband_configQUEUE_TRACE_ENABLE == 1)
    void impl_tband_freertos_queue_receive_from_isr(uint32_t id, uint32_t size_before);
      #define traceQUEUE_RECEIVE_FROM_ISR(pxQueue) impl_tband_freertos_queue_receive_from_isr(                                \
          (uint32_t)(pxQueue)->uxQueueNumber /* queue id */,                                                           \
          (uint32_t)(pxQueue)->uxMessagesWaiting /* size before */                                                     \
        )
  #endif /* (tband_configQUEUE_TRACE_ENABLE == 1) */

  // FIXME: Add version toggle!
  #if (tband_configTASK_TRACE_ENABLE == 1)
    void impl_tband_freertos_blocking_on_queue_receive(uint32_t queue_id, uint32_t ticks_to_wait);
    #define traceBLOCKING_ON_QUEUE_RECEIVE(pxQueue) impl_tband_freertos_blocking_on_queue_receive(                            \
        (uint32_t)((pxQueue)->uxQueueNumber) /* queue id */,                                                           \
        (uint32_t)(xTicksToWait) /* ticks to wait */                                                                   \
      )
  #endif /* (tband_configTASK_TRACE_ENABLE == 1) */

  // == Queue Reset ============================================================

  // FIXME: Add version toggle!
  #if (tband_configQUEUE_TRACE_ENABLE == 1)
    void impl_tband_freertos_queue_reset(uint32_t id);
    #define traceQUEUE_RESET(pxQueue, xNewQueue) impl_tband_freertos_queue_reset(                                             \
        (uint32_t)(pxQueue)->uxQueueNumber /* queue id */                                                              \
      )
  #endif /* (tband_configQUEUE_TRACE_ENABLE == 1) */

  // == Queue Peek ============================================================

  // FIXME: Add version toggle!
  #if (tband_configTASK_TRACE_ENABLE == 1)
    void impl_tband_freertos_blocking_on_queue_peek(uint32_t queue_id, uint32_t ticks_to_wait);
    #define traceBLOCKING_ON_QUEUE_PEEK(pxQueue) impl_tband_freertos_blocking_on_queue_peek(                                  \
        (uint32_t)((pxQueue)->uxQueueNumber) /* queue id */,                                                           \
        (uint32_t)(xTicksToWait) /* ticks to wait */                                                                   \
      )
  #endif /* (tband_configTASK_TRACE_ENABLE == 1) */

  // == Markers ================================================================

  #if (tband_configMARKER_TRACE_ENABLE == 1)
  
  void impl_tband_evtmarker_name(uint32_t id, const char* name);
  #define tband_evtmarker_name(id, name) impl_tband_evtmarker_name(id, name)

  void impl_tband_evtmarker(uint32_t id, const char* msg);
  #define tband_evtmarker(id, msg) impl_tband_evtmarker(id, msg)

  void impl_tband_evtmarker_begin(uint32_t id, const char* msg);
  #define tband_evtmarker_begin(id, msg) impl_tband_evtmarker_begin(id, msg)

  void impl_tband_evtmarker_end(uint32_t id);
  #define tband_evtmarker_end(id) impl_tband_evtmarker_end(id)

  void impl_tband_valmarker_name(uint32_t id, const char* name);
  #define tband_valmarker_name(id, name) impl_tband_valmarker_name(id, name)

  void impl_tband_valmarker(uint32_t id, int64_t val);
  #define tband_valmarker(id, val) impl_tband_valmarker(id, val)

  #endif /* (tband_configMARKER_TRACE_ENABLE == 1) */

  // == Task-Local Markers =====================================================

  #if ((tband_configMARKER_TRACE_ENABLE == 1) && (tband_configFREERTOS_TRACE_ENABLE == 1))

  void impl_tband_freertos_task_evtmarker_name(uint32_t id, const char* name);
  #define tband_freertos_task_evtmarker_name(id, name) impl_tband_freertos_task_evtmarker_name(id, name)

  void impl_tband_freertos_task_evtmarker(uint32_t id, const char* msg);
  #define tband_freertos_task_evtmarker(id, msg) impl_tband_freertos_task_evtmarker(id, msg)

  void impl_tband_freertos_task_evtmarker_begin(uint32_t id, const char* msg);
  #define tband_freertos_task_evtmarker_begin(id, msg) impl_tband_freertos_task_evtmarker_begin(id, msg)

  void impl_tband_freertos_task_evtmarker_end(uint32_t id);
  #define tband_freertos_task_evtmarker_end(id) impl_tband_freertos_task_evtmarker_end(id)

  void impl_tband_freertos_task_valmarker_name(uint32_t id, const char* name);
  #define tband_freertos_task_valmarker_name(id, name) impl_tband_freertos_task_valmarker_name(id, name)

  void impl_tband_freertos_task_valmarker(uint32_t id, int64_t val);
  #define tband_freertos_task_valmarker(id, val) impl_tband_freertos_task_valmarker(id, val)

  #endif /* ((tband_configMARKER_TRACE_ENABLE == 1) && (tband_configFREERTOS_TRACE_ENABLE == 1)) */

#endif /* tband_configENABLE == 1*/

// Provide shims for tracing hooks that are called manually if they are disabled:

#ifndef tband_gather_system_metadata
  #define tband_gather_system_metadata()
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

#ifndef tband_freertos_queue_name
  #define tband_freertos_queue_name(handle, name)
#endif /* tband_freertos_queue_name */

#ifndef tband_freertos_binary_semaphore_name
  #define tband_freertos_binary_semaphore_name(handle, name)
#endif /* tband_freertos_binary_semaphore_name */

#ifndef tband_freertos_counting_semaphore_name
  #define tband_freertos_counting_semaphore_name(handle, name)
#endif /* tband_freertos_counting_semaphore_name */

#ifndef tband_freertos_mutex_name
  #define tband_freertos_mutex_name(handle, name)
#endif /* tband_freertos_mutex_name */

#ifndef tband_freertos_recursive_mutex_name
  #define tband_freertos_recursive_mutex_name(handle, name)
#endif /* tband_freertos_recursive_mutex_name */

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
  #define tband_evtmarker_end(id, msg)
#endif /* tband_evtmarker_end */

#ifndef tband_freertos_task_evtmarker_name
  #define tband_freertos_task_evtmarker_name(id, name)
#endif /* tband_freertos_task_evtmarker_name */

#ifndef tband_freertos_task_evtmarker
  #define tband_freertos_task_evtmarker(id, msg)
#endif /* tband_freertos_task_evtmarker */

#ifndef tband_freertos_task_evtmarker_begin
  #define tband_freertos_task_evtmarker_begin(id, msg)
#endif /* tband_freertos_task_evtmarker_begin */

#ifndef tband_freertos_task_evtmarker_end
  #define tband_freertos_task_evtmarker_end(id)
#endif /* tband_freertos_task_evtmarker_end */

#ifndef tband_valmarker_name
  #define tband_valmarker_name(id, name)
#endif /* tband_valmarker_name */

#ifndef tband_valmarker
  #define tband_valmarker(id, val)
#endif /* tband_valmarker */

#ifndef tband_freertos_task_valmarker_name
  #define tband_freertos_task_valmarker_name(id, name)
#endif /* tband_freertos_task_valmarker_name */

#ifndef tband_freertos_task_valmarker
  #define tband_freertos_task_valmarker(id, val)
#endif /* tband_freertos_task_valmarker */

#if defined(__cplusplus)
}
#endif

#endif /* TBAND_HOOKS_H_ */
// clang-format on
