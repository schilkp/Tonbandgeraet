# FreeRTOS Resource Tracing

Tonbandgerät traces the creation and operation of FreeRTOS queues, semaphores,
and mutexes. All of these are internally represented as queues in FreeRTOS,
and Tonbandgerät treats them the same way: each gets a unique ID, an optional
name, and its operation events are recorded in the trace.

## Internal Use of `uxQueueNumber`

> [!WARNING]
> Tonbandgerät uses the `uxQueueNumber` field of each FreeRTOS queue control
> block to store its internal queue ID. This field is set automatically via
> `vQueueSetQueueNumber()` when a queue (or semaphore or mutex) is created,
> and read by every subsequent queue-related trace hook.
>
> **Do not call `vQueueSetQueueNumber()` or `uxQueueGetQueueNumber()` anywhere
> in your application code.** Doing so will overwrite the tracer's ID
> assignment and cause all subsequent events for that resource to be
> misattributed in the trace viewer.

Every queue, semaphore, or mutex is assigned a unique, monotonically increasing
integer ID the moment it is created (`traceQUEUE_CREATE`). This ID is used to
identify the resource in all subsequent trace events. ID `0` is reserved and
never assigned.

## Naming Resources

Queues, semaphores, and mutexes are anonymous by default. Giving them names
makes traces significantly easier to read. Names are stored in the
[metadata buffer](./metadata_buf.md) and can be set at any time after the
resource is created, even before the trace collection has started:

```c
tband_freertos_queue_name(handle, name);
tband_freertos_binary_semaphore_name(handle, name);
tband_freertos_counting_semaphore_name(handle, name);
tband_freertos_mutex_name(handle, name);
tband_freertos_recursive_mutex_name(handle, name);
```

**Example:**
```c
QueueHandle_t uart_queue = xQueueCreate(16, sizeof(char));
tband_freertos_queue_name(uart_queue, "uart_rx");

SemaphoreHandle_t spi_mutex = xSemaphoreCreateMutex();
tband_freertos_mutex_name(spi_mutex, "spi_bus");
```

## Configuration

Queue/resource operation tracing can be disabled independently of task tracing:

```c
#define tband_configFREERTOS_QUEUE_TRACE_ENABLE 0
```

When disabled, resources are still assigned IDs (via `traceQUEUE_CREATE`,
which always runs), so they remain identifiable. The queue kind and initial
fill level are not recorded, and all send/receive/reset events are suppressed.

## FreeRTOS Trace Hooks

### Creation

| FreeRTOS Hook       | What it records                            |
| ---                 | ---                                        |
| `traceQUEUE_CREATE` | A queue, sempaphore, mutex etc was created |

When any queue, semaphore, or mutex is created, Tonbandgerät records:

- The creation event and assigned ID
- The resource kind (queue, binary semaphore, counting semaphore, mutex, or recursive mutex)
- The initial fill level (always 0 for queues and mutexes; the initial count for counting semaphores)

These are [metadata events](./metadata_buf.md) and are stored in the metadata
buffer.

### Operations

The following events are recorded automatically during operation. 

| FreeRTOS Hook                 | What it records                                       |
| ---                           | ---                                                   |
| `traceQUEUE_SEND`             | An item was sent to a queue (from task context)       |
| `traceQUEUE_SEND_FROM_ISR`    | An item was sent to a queue (from ISR)                |
| `traceQUEUE_RECEIVE`          | An item was received from a queue (from task context) |
| `traceQUEUE_RECEIVE_FROM_ISR` | An item was received from a queue (from ISR)          |
| `traceQUEUE_RESET`            | A queue was reset to its empty state                  |

