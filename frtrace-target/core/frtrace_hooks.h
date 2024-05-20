/**
 * @file frtrace_hooks.h
 * @brief FreeRTOS tracer hooks
 * @author Philipp Schilk, 2024
 */
// clang-format off
#ifndef FRTRACE_HOOKS_H_
#define FRTRACE_HOOKS_H_

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

#if (frtrace_configENABLE == 1)

  // == Scheduler Metadata =====================================================

  // Manual implementation if traceSCHEDULER_STARTED does not exist:
  // FIXME: Add version toggle!
  #if (INCLUDE_xTaskGetIdleTaskHandle != 1 )
    #error "INCLUDE_xTaskGetIdleTaskHandle is not enabled!"
  #endif /* INCLUDE_xTaskGetIdleTaskHandle */
  void impl_frtrace_gather_scheduler_metadata();
  #define frtrace_gather_scheduler_metadata() impl_frtrace_gather_scheduler_metadata()

  // == Task Switched In =======================================================

  // Note: When called, `pxCurrentTCB` contains handle of task about to run.
  void impl_frtrace_task_switched_in(uint32_t task_id);
  #define traceTASK_SWITCHED_IN() impl_frtrace_task_switched_in(                                                       \
      (uint32_t)(pxCurrentTCB)->uxTaskNumber /* task id */                                                             \
    )

  // == Task Switched Out ======================================================

  // Note: When called, `pxCurrentTCB` contains handle of task about to run.
  #define traceTASK_SWITCHED_OUT()

  // == Task to Ready State ====================================================

  void impl_frtrace_moved_task_to_ready_state(uint32_t task_id);
  #define traceMOVED_TASK_TO_READY_STATE(pxTask) impl_frtrace_moved_task_to_ready_state(                               \
      (uint32_t)(pxTask)->uxTaskNumber /* task id */                                                                   \
    )

  // == Task Resume ============================================================

  void impl_frtrace_task_resumed(uint32_t task_id);
  #define traceTASK_RESUME(pxTask) impl_frtrace_task_resumed(                                                          \
      (uint32_t)(pxTask)->uxTaskNumber /* task id */                                                                   \
    )

  void impl_frtrace_task_resumed_from_isr(uint32_t task_id);
  #define traceTASK_RESUME_FROM_ISR(pxTask) impl_frtrace_task_resumed_from_isr(                                        \
      (uint32_t)(pxTask)->uxTaskNumber /* task id */                                                                   \
    )

  // == Task Suspended =========================================================

  void impl_frtrace_task_suspended(uint32_t task_id);
  #define traceTASK_SUSPEND(pxTask) impl_frtrace_task_suspended(                                                       \
      (uint32_t)(pxTask)->uxTaskNumber /* task id */                                                                   \
    )

  // == Task Delayed ===========================================================

  // FIXME: Add version toggle!

  void impl_frtrace_task_delay(uint32_t ticks);
  #define traceTASK_DELAY() impl_frtrace_task_delay((uint32_t) xTicksToDelay)

  void impl_frtrace_task_delay_until(uint32_t time_to_wake);
  #define traceTASK_DELAY_UNTIL(xTimeToWake) impl_frtrace_task_delay_until(xTimeToWake)

  // == Task Priority Set ======================================================

  void impl_frtrace_task_priority_set(uint32_t task_id, uint32_t priority);
  #define traceTASK_PRIORITY_SET(pxTask, uxNewPriority) impl_frtrace_task_priority_set(                                \
      (uint32_t)(pxTask)->uxTaskNumber /* task id */,                                                                  \
      (uint32_t)(uxNewPriority) /* priority */                                                                         \
    )

  // == Task Priority Inherited ================================================

  void impl_frtrace_task_priority_inherit(uint32_t task_id, uint32_t priority);
  #define traceTASK_PRIORITY_INHERIT(pxTCBOfMutexHolder, uxInheritedPriority) impl_frtrace_task_priority_inherit(      \
      (uint32_t)(pxTCBOfMutexHolder)->uxTaskNumber /* task id */,                                                      \
      (uint32_t)(uxInheritedPriority) /* priority */                                                                   \
    )

  // == Task Priority Disinherited =============================================

  void impl_frtrace_task_priority_disinherit(uint32_t task_id, uint32_t priority);
  #define traceTASK_PRIORITY_DISINHERIT(pxTCBOfMutexHolder, uxOriginalPriority) impl_frtrace_task_priority_disinherit( \
      (uint32_t)(pxTCBOfMutexHolder)->uxTaskNumber /* task id */,                                                      \
      (uint32_t)(uxOriginalPriority) /* priority */                                                                    \
    )

  // == Task Created ===========================================================

  void impl_frtrace_task_create(void * task_handle, uint32_t priority, char *name);
  #define traceTASK_CREATE(pxTask) impl_frtrace_task_create(                                                           \
      (void *)(pxTask) /* task handle */,                                                                              \
      (uint32_t)(pxTask)->uxPriority /* priority */,                                                                   \
      (pxTask)->pcTaskName /* name */                                                                                  \
    )

  // == Task Deleted ===========================================================

  void impl_frtrace_task_deleted(uint32_t task_id);

  #define traceTASK_DELETE(pxTask) impl_frtrace_task_deleted(                                                          \
      (pxTask)->uxTaskNumber  /* task id */                                                                            \
    )

  // == ISRs ===================================================================

  #if (frtrace_configISR_TRACE_ENABLE == 1)

    void impl_frtrace_isr_name(uint32_t isr_id, char *name);
    #define frtrace_isr_name(isr_id, name) impl_frtrace_isr_name(isr_id, name)

    void impl_frtrace_isr_enter(uint32_t isr_id);
    #define frtrace_isr_enter(isr_id) impl_frtrace_isr_enter(isr_id)

    void impl_frtrace_isr_exit(uint32_t isr_id);
    #define frtrace_isr_exit(isr_id) impl_frtrace_isr_exit(isr_id)

  #endif /* frtrace_configISR_TRACE_ENABLE == 1 */

  // == Queue Created ==========================================================

  void impl_frtrace_queue_created(void *handle, uint8_t type_val);
  #define traceQUEUE_CREATE(pxNewQueue) impl_frtrace_queue_created(                                                    \
      (void *)(pxNewQueue),               /* queue handle  */                                                          \
      (uint8_t)(pxNewQueue)->ucQueueType  /* type */                                                                   \
    )

  // == Queue Name =============================================================

  void impl_frtrace_queue_name(void *queue_handle, char *name);
  #define frtrace_queue_name(handle, name) impl_frtrace_queue_name((void *)(handle), (name))
  #define frtrace_binary_semaphore_name(handle, name) frtrace_queue_name((handle), (name))
  #define frtrace_counting_semaphore_name(handle, name) frtrace_queue_name((handle), (name))
  #define frtrace_mutex_name(handle, name) frtrace_queue_name((handle), (name))
  #define frtrace_recursive_mutex_name(handle, name) frtrace_queue_name((handle), (name))

  // // == Queue Send =============================================================

  // FIXME: Add version toggle!
  void impl_frtrace_queue_send(uint32_t id, uint32_t copy_position, uint32_t size_before);
  #define traceQUEUE_SEND(pxQueue) impl_frtrace_queue_send(                                                            \
      (uint32_t)(pxQueue)->uxQueueNumber /* queue id */,                                                               \
      queueSEND_TO_BACK /* copy position */,                                                                           \
      (uint32_t)(pxQueue)->uxMessagesWaiting /* size before */                                                         \
    )

  // FIXME: Add version toggle!
  // Re-route set send to queue send:
  #define traceQUEUE_SET_SEND(pxQueue) impl_frtrace_queue_send(                                                        \
      (uint32_t)(pxQueue)->uxQueueNumber /* queue id */,                                                               \
      queueSEND_TO_BACK /* copy position */,                                                                           \
      (uint32_t)(pxQueue)->uxMessagesWaiting /* size before */                                                         \
    )

  // FIXME: Add version toggle! queueSEND_TO_BACK is not fixed!
  void impl_frtrace_queue_send_from_isr(uint32_t id, uint32_t copy_position, uint32_t size_before);
  #define traceQUEUE_SEND_FROM_ISR(pxQueue) impl_frtrace_queue_send_from_isr(                                          \
      (uint32_t)(pxQueue)->uxQueueNumber /* queue id */,                                                               \
      queueSEND_TO_BACK /* copy position */,                                                                           \
      (uint32_t)(pxQueue)->uxMessagesWaiting /* size before */                                                         \
    )

  // FIXME: Add version toggle!
  void impl_frtrace_blocking_on_queue_send(uint32_t queue_id, uint32_t ticks_to_wait);
  #define traceBLOCKING_ON_QUEUE_SEND(pxQueue) impl_frtrace_blocking_on_queue_send(                                    \
      (uint32_t)(pxQueue)->uxQueueNumber /* queue id */,                                                               \
      (uint32_t)(xTicksToWait) /* ticks to wait */                                                                     \
    )

  // == Queue Receive ==========================================================

  void impl_frtrace_queue_receive(uint32_t id, uint32_t size_before);
  #define traceQUEUE_RECEIVE(pxQueue) impl_frtrace_queue_receive(                                                      \
      (uint32_t)(pxQueue)->uxQueueNumber /* queue id */,                                                               \
      (uint32_t)(pxQueue)->uxMessagesWaiting /* size before */                                                         \
    )

  void impl_frtrace_queue_receive_from_isr(uint32_t id, uint32_t size_before);
  #define traceQUEUE_RECEIVE_FROM_ISR(pxQueue) impl_frtrace_queue_receive_from_isr(                                    \
      (uint32_t)(pxQueue)->uxQueueNumber /* queue id */,                                                               \
      (uint32_t)(pxQueue)->uxMessagesWaiting /* size before */                                                         \
    )

  // FIXME: Add version toggle!
  void impl_frtrace_blocking_on_queue_receive(uint32_t queue_id, uint32_t ticks_to_wait);
  #define traceBLOCKING_ON_QUEUE_RECEIVE(pxQueue) impl_frtrace_blocking_on_queue_receive(                              \
      (uint32_t)((pxQueue)->uxQueueNumber) /* queue id */,                                                             \
      (uint32_t)(xTicksToWait) /* ticks to wait */                                                                     \
    )

  // == Queue Reset ============================================================

  // FIXME: Add version toggle!
  void impl_frtrace_queue_reset(uint32_t id);
  #define traceQUEUE_RESET(pxQueue, xNewQueue) impl_frtrace_queue_reset(                                               \
      (uint32_t)(pxQueue)->uxQueueNumber /* queue id */                                                                \
    )

  // == Queue Peek ============================================================

  // FIXME: Add version toggle!
  void impl_frtrace_blocking_on_queue_peek(uint32_t queue_id, uint32_t ticks_to_wait);
  #define traceBLOCKING_ON_QUEUE_PEEK(pxQueue) impl_frtrace_blocking_on_queue_peek(                                    \
      (uint32_t)((pxQueue)->uxQueueNumber) /* queue id */,                                                             \
      (uint32_t)(xTicksToWait) /* ticks to wait */                                                                     \
    )

#endif /* frtrace_configENABLE == 1*/

// Provide shims for tracing hooks that are called manually if they are disabled:

#ifndef frtrace_gather_scheduler_metadata
  #define frtrace_gather_scheduler_metadata()
#endif /* frtrace_gather_scheduler_metadata */

#ifndef frtrace_isr_name
  #define frtrace_isr_name(isr_id, name)
#endif /* frtrace_isr_exit */

#ifndef frtrace_isr_enter
  #define frtrace_isr_enter(isr_id)
#endif /* frtrace_isr_enter */

#ifndef frtrace_isr_exit
  #define frtrace_isr_exit(isr_id)
#endif /* frtrace_isr_exit */

#ifndef frtrace_queue_name
  #define frtrace_queue_name(handle, name)
#endif /* frtrace_queue_name */

#ifndef frtrace_binary_semaphore_name
  #define frtrace_binary_semaphore_name(handle, name)
#endif /* frtrace_binary_semaphore_name */

#ifndef frtrace_counting_semaphore_name
  #define frtrace_counting_semaphore_name(handle, name)
#endif /* frtrace_counting_semaphore_name */

#ifndef frtrace_mutex_name
  #define frtrace_mutex_name(handle, name)
#endif /* frtrace_mutex_name */

#ifndef frtrace_recursive_mutex_name
  #define frtrace_recursive_mutex_name(handle, name)
#endif /* frtrace_recursive_mutex_name */

#if defined(__cplusplus)
}
#endif

#endif /* FRTRACE_HOOKS_H_ */
// clang-format on
