/**
 * @file tracing_hooks.h
 * @brief FreeRTOS tracing hooks.
 * @author Philipp Schilk, 2024
 *
 * This file should be included at the end of of FreeRTOSConfig.h
 */
// clang-format off

#ifndef TRACING_HOOKS_H_
#define TRACING_HOOKS_H_

#if defined(__cplusplus)
extern "C" {
#endif

#ifndef traceconfigENABLE
  #define traceconfigENABLE 0
#endif /* traceconfigENABLE */

#ifndef traceportTIMESTAMP_NS
  #error "traceportTIMESTAMP_NS is not defined!"
#endif /* traceportTIMESTAMP_NS */

#ifndef traceportHANDLE_RAW_EVT
  #error "traceportHANDLE_RAW_EVT is not defined!"
#endif /* traceportHANDLE_RAW_EVT */

#ifndef traceportMAX_ISR_ID
  #error "traceportMAX_ISR_ID is not defined!"
#endif /* traceportHANDLE_RAW_EVT */

#ifndef traceconfigTRACE_DROP_CNT_EVERY
  #define traceconfigTRACE_DROP_CNT_EVERY (5)
#endif /* traceportHANDLE_RAW_EVT */

#include <stdint.h>

#if (traceconfigENABLE == 1)

  void trace_task_switched_in(uint32_t task_id);

  // Note: When called, `pxCurrentTCB` contains handle of task about to run.
  #define traceTASK_SWITCHED_IN() trace_task_switched_in(                                                              \
      (uint32_t)(pxCurrentTCB)->uxTCBNumber /* task id */                                                              \
    )

  // Note: When called, `pxCurrentTCB` contains handle of task about to run.
  #define traceTASK_SWITCHED_OUT()

  void trace_moved_task_to_ready_state(uint32_t task_id);

  #define traceMOVED_TASK_TO_READY_STATE(pxTask) trace_moved_task_to_ready_state(                                      \
      (uint32_t)(pxTask)->uxTCBNumber /* task id */                                                                    \
    )

  void trace_task_resume(uint32_t task_id);

  #define traceTASK_RESUME(pxTask) trace_task_resume(                                                                  \
      (uint32_t)(pxTask)->uxTCBNumber /* task id */                                                                    \
    )

  #define traceTASK_RESUME_FROM_ISR(pxTask) trace_task_resume(                                                         \
      (uint32_t)(pxTask)->uxTCBNumber /* task id */                                                                    \
    )

  void trace_task_suspend(uint32_t task_id);


  #define traceTASK_SUSPEND(pxTask) trace_task_suspend(                                                                \
      (uint32_t)(pxTask)->uxTCBNumber /* task id */                                                                    \
    )

  void trace_task_delay(void);

  #define traceTASK_DELAY() trace_task_delay()

  #define traceTASK_DELAY_UNTIL(_v_) trace_task_delay()

  void trace_blocking_on_queue_peek(uint32_t queue_id);

  #define traceBLOCKING_ON_QUEUE_PEEK(pxQueue) trace_blocking_on_queue_peek(                                           \
      (uint32_t)((pxQueue)->uxQueueNumber) /* queue id */                                                              \
    )

  void trace_blocking_on_queue_receive(uint32_t queue_id);

  #define traceBLOCKING_ON_QUEUE_RECEIVE(pxQueue) trace_blocking_on_queue_receive(                                     \
      (uint32_t)((pxQueue)->uxQueueNumber) /* queue id */                                                              \
    )

  void trace_blocking_on_queue_send(uint32_t queue_id);

  #define traceBLOCKING_ON_QUEUE_SEND(pxQueue) trace_blocking_on_queue_send(                                           \
      (uint32_t)((pxQueue)->uxQueueNumber) /* queue id */                                                              \
    )

  void trace_blocking_on_stream_buffer_receive(uint32_t sb_id);

  #define traceBLOCKING_ON_STREAM_BUFFER_RECEIVE(xStreamBuffer) trace_blocking_on_stream_buffer_receive(               \
      (uint32_t)(xStreamBuffer)->uxStreamBufferNumber /* stream buffer id */                                           \
    )

  void trace_blocking_on_stream_buffer_send(uint32_t sb_id);

  #define traceBLOCKING_ON_STREAM_BUFFER_SEND(xStreamBuffer) trace_blocking_on_stream_buffer_send(                     \
      (uint32_t)(xStreamBuffer)->uxStreamBufferNumber /* stream buffer id */                                           \
    )

  void trace_task_priority_inherit(uint32_t task_id, uint32_t priority);

  #define traceTASK_PRIORITY_INHERIT(pxTCBOfMutexHolder, uxInheritedPriority) trace_task_priority_inherit(             \
      (uint32_t)(pxTCBOfMutexHolder)->uxTCBNumber /* task id */,                                                       \
      (uint32_t)(uxInheritedPriority) /* priority */                                                                   \
    )

  void trace_task_priority_disinherit(uint32_t task_id, uint32_t priority);

  #define traceTASK_PRIORITY_DISINHERIT(pxTCBOfMutexHolder, uxOriginalPriority) trace_task_priority_disinherit(        \
      (uint32_t)(pxTCBOfMutexHolder)->uxTCBNumber /* task id */,                                                       \
      (uint32_t)(uxOriginalPriority) /* priority */                                                                    \
    )

  void trace_task_priority_set(uint32_t task_id, uint32_t priority);

  #define traceTASK_PRIORITY_SET(pxTask, uxNewPriority) trace_task_priority_set(                                       \
      (uint32_t)(pxTask)->uxTCBNumber /* task id */,                                                                   \
      (uint32_t)(uxNewPriority) /* priority */                                                                         \
    )

  void trace_task_create(uint32_t task_id, uint32_t priority, char *name);

  #define traceTASK_CREATE(pxTask) trace_task_create(                                                                  \
      (uint32_t)(pxTask)->uxTCBNumber /* task id */,                                                                   \
      (uint32_t)(pxTask)->uxPriority /* priority */,                                                                   \
      (pxTask)->pcTaskName /* name */                                                                                  \
    )

  void trace_task_delete(uint32_t task_id);

  #define traceTASK_DELETE(pxTask) trace_task_delete(                                                                  \
      (pxTask)->uxTCBNumber  /* task id */                                                                             \
    )

  #if (traceconfigISR_TRACE_ENABLE == 1)

    void impl_trace_isr_enter(int32_t isr_id);

    #define trace_isr_enter(isr_id) impl_trace_isr_enter(                                                              \
        (int32_t)isr_id /* isr id */                                                                                   \
      )

    void impl_trace_isr_exit(int32_t isr_id);

    #define trace_isr_exit(isr_id) impl_trace_isr_exit(                                                                \
        (int32_t)isr_id /* isr id */                                                                                   \
      )

    void impl_trace_isr_name(int32_t isr_id, char *name);

    #define trace_isr_name(isr_id, name) impl_trace_isr_name(                                                          \
        (int32_t)isr_id /* isr id */, name, /* name */                                                                 \
      )

  #endif /* traceconfigISR_TRACE_ENABLE == 1 */

  void trace_queue_create(void *handle, uint8_t type_val);

  #define traceQUEUE_CREATE(pxNewQueue) trace_queue_create(                                                            \
      (void *)(pxNewQueue),               /* queue handle  */                                                          \
      (uint8_t)(pxNewQueue)->ucQueueType  /* type */                                                                   \
    )

  void impl_trace_queue_name(void *queue_handle, char *name);

  #define trace_queue_name(handle, name) impl_trace_queue_name(                                                        \
      (void *)(handle) /* handle */,                                                                                   \
      (name) /* name */                                                                                                \
    )

  #define trace_binary_semaphore_name(handle, name) trace_queue_name((handle), (name))

  #define trace_counting_semaphore_name(handle, name) trace_queue_name((handle), (name))

  #define trace_mutex_name(handle, name) trace_queue_name((handle), (name))

  #define trace_recursive_mutex_name(handle, name) trace_queue_name((handle), (name))

  void trace_queue_send(uint32_t id);

  #define traceQUEUE_SEND(pxQueue) trace_queue_send(                                                                   \
      (uint32_t)(pxQueue)->uxQueueNumber /* queue id */                                                                \
    )

  #define traceQUEUE_SEND_FROM_ISR(pxQueue) trace_queue_send(                                                          \
      (uint32_t)(pxQueue)->uxQueueNumber /* queue id */                                                                \
    )

  void trace_queue_receive(uint32_t id);

  #define traceQUEUE_RECEIVE(pxQueue) trace_queue_receive(                                                             \
      (uint32_t)(pxQueue)->uxQueueNumber /* queue id */                                                                \
    )

  #define traceQUEUE_RECEIVE_FROM_ISR(pxQueue) trace_queue_receive(                                                    \
      (uint32_t)(pxQueue)->uxQueueNumber /* queue id */                                                                \
    )

  void trace_stream_buffer_create(void *handle, int is_message_buffer);

  #define traceSTREAM_BUFFER_CREATE(pxStreamBuffer, xIsMessageBuffer) trace_stream_buffer_create(                      \
      (void *)(pxStreamBuffer) /* handle */,                                                                           \
      (xIsMessageBuffer) /* is message buffer */                                                                       \
    )

  void impl_trace_stream_buffer_name(void *handle, char *name);

  #define trace_stream_buffer_name(handle, name) impl_trace_stream_buffer_name(                                        \
      (void *)(handle) /* handle */,                                                                                   \
      (name) /* name */                                                                                                \
    )

  void trace_stream_buffer_send(uint32_t id, uint32_t len);

  #define traceSTREAM_BUFFER_SEND(xStreamBuffer, xBytesSent) trace_stream_buffer_send(                                 \
      (xStreamBuffer)->uxStreamBufferNumber /* stream buffer id */,                                                    \
      (uint32_t)(xBytesSent) /* amnt */                                                                                \
    )

  #define traceSTREAM_BUFFER_SEND_FROM_ISR(xStreamBuffer, xBytesSent) trace_stream_buffer_send(                        \
      (xStreamBuffer)->uxStreamBufferNumber /* stream buffer id */,                                                    \
      (uint32_t)(xBytesSent) /* amnt */                                                                                \
    )

  void trace_stream_buffer_receive(uint32_t id, uint32_t len);

  #define traceSTREAM_BUFFER_RECEIVE(xStreamBuffer, xReceivedLength) trace_stream_buffer_receive(                      \
      (xStreamBuffer)->uxStreamBufferNumber /* stream buffer id */,                                                    \
      (xReceivedLength) /* amnt */                                                                                     \
    )

#endif /* traceconfigENABLE == 1*/

// Provide shims for tracing hooks that are called manually if they are disabled:

#ifndef trace_isr_enter
  #define trace_isr_enter(isr_id)
#endif /* trace_isr_enter */

#ifndef trace_isr_exit
  #define trace_isr_exit(isr_id)
#endif /* trace_isr_exit */

#ifndef trace_isr_name
  #define trace_isr_name(isr_id, name)
#endif /* trace_isr_exit */

#ifndef trace_queue_name
  #define trace_queue_name(handle, name)
#endif /* trace_queue_name */

#ifndef trace_binary_semaphore_name
  #define trace_binary_semaphore_name(handle, name)
#endif /* trace_binary_semaphore_name */

#ifndef trace_counting_semaphore_name
  #define trace_counting_semaphore_name(handle, name)
#endif /* trace_counting_semaphore_name */

#ifndef trace_mutex_name
  #define trace_mutex_name(handle, name)
#endif /* trace_mutex_name */

#ifndef trace_recursive_mutex_name
  #define trace_recursive_mutex_name(handle, name)
#endif /* trace_recursive_mutex_name */

#ifndef trace_stream_buffer_name
  #define trace_stream_buffer_name(handle, name)
#endif /* trace_stream_buffer_name */

#if defined(__cplusplus)
}
#endif

#endif /* TRACING_HOOKS_H_ */
