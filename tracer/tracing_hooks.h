/**
 * @file tracing_hooks.h
 * @brief FreeRTOS tracing hooks. This file should be included at the end of
 *        of FreeRTOSConfig.h
 * @author Philipp Schilk, 2024
 */

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

#if (traceconfigENABLE == 1)

// #### Tasks Scheduling ####

// Called after a task has been selected to run. At this point pxCurrentTCB
// contains the handle of the task about to enter the Running state. In:
// tasks.c
#define traceTASK_SWITCHED_IN() trace_task_switched_in((uint32_t)pxCurrentTCB->uxTCBNumber)
void trace_task_switched_in(uint32_t task_id);

// Called before a new task is selected to run. At this point pxCurrentTCB
// contains the handle of the task about to leave the Running state. In:
// tasks.c
#define traceTASK_SWITCHED_OUT()

// Called when a task is transitioned into the Ready state. In: tasks.c
#define traceMOVED_TASK_TO_READY_STATE(pxTask) trace_moved_task_to_ready_state((uint32_t)(pxTask)->uxTCBNumber)
void trace_moved_task_to_ready_state(uint32_t task_id);

// Called from within vTaskResume(). In: tasks.c
#define traceTASK_RESUME(pxTask) trace_task_resume((uint32_t)(pxTask)->uxTCBNumber)
void trace_task_resume(uint32_t task_id);

// Called from within xTaskResumeFromISR(). In: tasks.c
#define traceTASK_RESUME_FROM_ISR(pxTask) trace_task_resume((uint32_t)(pxTask)->uxTCBNumber)

// Called from within vTaskSuspend(). In: tasks.c
#define traceTASK_SUSPEND(pxTask) trace_task_suspend((uint32_t)(pxTask)->uxTCBNumber)
void trace_task_suspend(uint32_t task_id);

// #### Tasks Blocking ####
// Trace events indicating that the current task has been blocked, and it will now be
// switched out. After being switched out, it will be in the `blocked` state.

// Called from within vTaskDelay(). In: tasks.c
#define traceTASK_DELAY() trace_task_delay()
void trace_task_delay(void);

// Called from within vTaskDelayUntil(). In: tasks.c
#define traceTASK_DELAY_UNTIL(_v_) trace_task_delay()

// Called from within xQueuePeek() before blocking a task trying to peek an
// empty queue. In: queue.c
#define traceBLOCKING_ON_QUEUE_PEEK(pxQueue) trace_blocking_on_queue_peek((uint32_t)((pxQueue)->uxQueueNumber))
void trace_blocking_on_queue_peek(uint32_t queue_id);

// Indicates that the currently executing task is about to block following
// an attempt to read from an empty queue, or an attempt to 'take' an empty
// semaphore or mutex. In: queue.c
#define traceBLOCKING_ON_QUEUE_RECEIVE(pxQueue) trace_blocking_on_queue_receive((uint32_t)((pxQueue)->uxQueueNumber))
void trace_blocking_on_queue_receive(uint32_t queue_id);

// Indicates that the currently executing task is about to block following
// an attempt to write to a full queue. In: queue.c
#define traceBLOCKING_ON_QUEUE_SEND(pxQueue) trace_blocking_on_queue_send((uint32_t)((pxQueue)->uxQueueNumber))
void trace_blocking_on_queue_send(uint32_t queue_id);

// Called from within xStreamBufferReceive() when the stream buffer is full
// and a block time is specified. In: stream_buffer.c
#define traceBLOCKING_ON_STREAM_BUFFER_RECEIVE(xStreamBuffer)                                                          \
  trace_blocking_on_stream_buffer_receive((uint32_t)(xStreamBuffer)->uxStreamBufferNumber)
void trace_blocking_on_stream_buffer_receive(uint32_t sb_id);

// Called from within xStreamBufferSend() when the stream buffer is full
// when a block time is specified. In: stream_buffer.c
#define traceBLOCKING_ON_STREAM_BUFFER_SEND(xStreamBuffer)                                                             \
  trace_blocking_on_stream_buffer_send((uint32_t)(xStreamBuffer)->uxStreamBufferNumber)
void trace_blocking_on_stream_buffer_send(uint32_t sb_id);

// #### Tasks Priority Changes ####

// Called from within xTaskPriorityInherit() after increasing the priority
// of the task holding a mutex waited on by a higher-priority task. In:
// tasks.c
#define traceTASK_PRIORITY_INHERIT(pxTCBOfMutexHolder, uxInheritedPriority)                                            \
  trace_task_priority_inherit((uint32_t)(pxTCBOfMutexHolder)->uxTCBNumber, (uint32_t)(uxInheritedPriority))
void trace_task_priority_inherit(uint32_t task_id, uint32_t priority);

// Called from within xTaskPriorityDisinherit() before decereasing the
// priority of a task that has since unlocked the mutex. In: tasks.c
#define traceTASK_PRIORITY_DISINHERIT(pxTCBOfMutexHolder, uxOriginalPriority)                                          \
  trace_task_priority_disinherit((uint32_t)(pxTCBOfMutexHolder)->uxTCBNumber, (uint32_t)(uxOriginalPriority))
void trace_task_priority_disinherit(uint32_t task_id, uint32_t priority);

// Called from within vTaskPrioritySet(). In: tasks.c
#define traceTASK_PRIORITY_SET(pxTask, uxNewPriority)                                                                  \
  trace_task_priority_set((uint32_t)(pxTask)->uxTCBNumber, (uint32_t)(uxNewPriority))
void trace_task_priority_set(uint32_t task_id, uint32_t priority);

// #### Tasks Creation / Destruction ####

// Called from within xTaskCreate() (or xTaskCreateStatic()) when the task
// is successfully created. In: tasks.c
#define traceTASK_CREATE(pxTask)                                                                                       \
  trace_task_create((uint32_t)(pxTask)->uxTCBNumber, (uint32_t)(pxTask)->uxPriority, (pxTask)->pcTaskName)
void trace_task_create(uint32_t task_id, uint32_t priority, char *name);

// Called from within vTaskDelete(). In: tasks.c
#define traceTASK_DELETE(pxTask) trace_task_delete((pxTask)->uxTCBNumber)
void trace_task_delete(uint32_t task_id);

// #### ISRs ####
// Does not use FreeRTOS hooks, since those are not necessarily present in every ISRs or may be too
// noisy. Instead we provide manual tracing functions:

#if (traceconfigISR_TRACE_ENABLE == 1)
#define trace_isr_enter(isr_id)      impl_trace_isr_enter(isr_id)
#define trace_isr_exit(isr_id)       impl_trace_isr_exit(isr_id)
#define trace_isr_name(isr_id, name) impl_trace_isr_name(isr_id, name)
#endif /* traceconfigISR_TRACE_ENABLE == 1 */

void impl_trace_isr_enter(int32_t isr_id);
void impl_trace_isr_exit(int32_t isr_id);
void impl_trace_isr_name(int32_t isr_id, char *name);

// #### Queues (and other basic RTOS resources) ####

#define traceQUEUE_CREATE(pxNewQueue)                                                                                  \
  trace_queue_create(pxNewQueue,                         /* handle. type: QueueHandle_t (== Queue *) */                \
                     (uint8_t)(pxNewQueue)->ucQueueType, /* type */                                                    \
                     (uint32_t)(pxNewQueue)->uxLength);  /* length */
void trace_queue_create(void *handle, uint8_t type_val, uint32_t len);

#define trace_queue_name(handle, name)              impl_trace_queue_name(handle, name)
#define trace_binary_semaphore_name(handle, name)   impl_trace_queue_name(handle, name)
#define trace_counting_semaphore_name(handle, name) impl_trace_queue_name(handle, name)
#define trace_mutex_name(handle, name)              impl_trace_queue_name(handle, name)
#define trace_recursive_mutex_name(handle, name)    impl_trace_queue_name(handle, name)
void impl_trace_queue_name(void *queue_handle, char *name);

#define traceQUEUE_SEND(pxQueue)          trace_queue_send((uint32_t)(pxQueue)->uxQueueNumber)
#define traceQUEUE_SEND_FROM_ISR(pxQueue) trace_queue_send((uint32_t)(pxQueue)->uxQueueNumber)
void trace_queue_send(uint32_t id);

#define traceQUEUE_RECEIVE(pxQueue)          trace_queue_receive((uint32_t)(pxQueue)->uxQueueNumber)
#define traceQUEUE_RECEIVE_FROM_ISR(pxQueue) trace_queue_receive((uint32_t)(pxQueue)->uxQueueNumber)
void trace_queue_receive(uint32_t id);

// #### Stream Buffers ####

#define traceSTREAM_BUFFER_CREATE(pxStreamBuffer, xIsMessageBuffer)                                                    \
  trace_stream_buffer_create(pxStreamBuffer, /* handle */ (uint32_t)(pxStreamBuffer)->xLength,                         \
                             /* length */ xIsMessageBuffer)
void trace_stream_buffer_create(void *handle, uint32_t len, int is_message_buffer);

#define trace_stream_buffer_name(handle, name) impl_trace_stream_buffer_name(handle, name)
void impl_trace_stream_buffer_name(void *handle, char *name);

#define traceSTREAM_BUFFER_SEND(xStreamBuffer, xBytesSent)                                                             \
  trace_stream_buffer_send((xStreamBuffer)->uxStreamBufferNumber, (uint32_t)(xBytesSent))
#define traceSTREAM_BUFFER_SEND_FROM_ISR(xStreamBuffer, xBytesSent)                                                    \
  trace_stream_buffer_send((xStreamBuffer)->uxStreamBufferNumber, (uint32_t)(xBytesSent))
void trace_stream_buffer_send(uint32_t id, uint32_t len);

#define traceSTREAM_BUFFER_RECEIVE(xStreamBuffer, xReceivedLength)                                                     \
  trace_stream_buffer_receive((xStreamBuffer)->uxStreamBufferNumber, (xReceivedLength))
void trace_stream_buffer_receive(uint32_t id, uint32_t len);

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
