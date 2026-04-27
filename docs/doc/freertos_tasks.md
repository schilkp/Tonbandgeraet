# FreeRTOS Task Tracing

When `tband_configFREERTOS_TASK_TRACE_ENABLE` is enabled (the default),
Tonbandgerät automatically traces FreeRTOS task scheduling events via the
FreeRTOS trace hook macros. No changes to application task code are needed.

## Internal Use of `uxTaskNumber`

> [!WARNING]
> Tonbandgerät uses the `uxTaskNumber` field of each FreeRTOS task control
> block to store its internal task ID. This field is set automatically via
> `vTaskSetTaskNumber()` when a task is created, and read by every subsequent
> task-related trace hook.
>
> **Do not call `vTaskSetTaskNumber()` or `uxTaskGetTaskNumber()` anywhere in
> your application code.** Doing so will overwrite the tracer's ID assignment
> and cause all subsequent events for that task to be misattributed in the
> trace viewer.

## Task IDs

Every task is assigned a unique, monotonically increasing integer ID the moment
it is created (`traceTASK_CREATE`). This ID is used to identify the task in
all subsequent trace events. ID `0` is reserved and never assigned to a user
task.

Task names are always recorded as [metadata](./metadata_buf.md) when a task is
created, regardless of whether `tband_configFREERTOS_TASK_TRACE_ENABLE` is on.
The idle task(s) and timer daemon task are identified as such when
`tband_freertos_scheduler_started()` is called. See the [FreeRTOS Tracing
overview](./freertos.md#manual-api-calls) for details. Since such events
are considered metadata, this is also tracked if tracing is not yet enabled.

## Configuration

Task tracing can be disabled independently of queue tracing:

```c
#define tband_configFREERTOS_TASK_TRACE_ENABLE 0
```

When disabled, tasks are still assigned IDs and their names are still stored
in the [metadata buffer](./metadata_buf.md), so they remain identifiable if
any other events reference them. Only the dynamic scheduling events listed
above are suppressed.

## FreeRTOS Trace Hooks

The following task scheduling events are recorded automatically:

| FreeRTOS Hook                    | What it records                                                    |
| ---                              | ---                                                                |
| `traceTASK_SWITCHED_IN`          | The task that is about to start running on this core               |
| `traceMOVED_TASK_TO_READY_STATE` | A task has been moved to the ready state                           |
| `traceTASK_RESUME`               | A task has been resumed (from task context)                        |
| `traceTASK_RESUME_FROM_ISR`      | A task has been resumed from an ISR                                |
| `traceTASK_SUSPEND`              | A task has been suspended                                          |
| `traceTASK_DELAY`                | The current task called `vTaskDelay()`                             |
| `traceTASK_DELAY_UNTIL`          | The current task called `vTaskDelayUntil()`                        |
| `traceTASK_PRIORITY_SET`         | A task's priority was explicitly changed                           |
| `traceTASK_PRIORITY_INHERIT`     | A task inherited a higher priority from a mutex holder             |
| `traceTASK_PRIORITY_DISINHERIT`  | A task's inherited priority was restored                           |
| `traceTASK_DELETE`               | A task was deleted                                                 |
| `traceBLOCKING_ON_QUEUE_SEND`    | The current task is about to block waiting to send to a queue      |
| `traceBLOCKING_ON_QUEUE_RECEIVE` | The current task is about to block waiting to receive from a queue |
| `traceBLOCKING_ON_QUEUE_PEEK`    | The current task is about to block waiting to peek a queue         |

The blocking-on-queue events are included here under task tracing because they
describe task state changes, even though they also reference a queue ID.
