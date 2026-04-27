# FreeRTOS Tracing

Tonbandgerät supports automatic tracing of FreeRTOS task scheduling and
resource operations by hooking into FreeRTOS's built-in trace facility.

> [!WARNING]
> FreeRTOS tracing is functional but still in active development. Some FreeRTOS
> features, including stream buffers, software timers, event groups, and
> direct-to-task notifications, are not yet traced. See
> [Limitations](#known-limitations) below.

## Overview

When FreeRTOS tracing is enabled, Tonbandgerät installs itself into FreeRTOS's
trace hook macros (`traceTASK_SWITCHED_IN`, `traceQUEUE_SEND`, etc.) by
including `tband.h` from `FreeRTOSConfig.h`. No manual instrumentation is
needed for automatic events. Task switches, queue operations, priority
changes, and more are all recorded without any changes to application code.

A small number of [manual API calls](#manual-api-calls) are still required to
give resources names that appear in the viewer, and to notify the tracer once
the scheduler has started.

## Required FreeRTOS Configuration

The following options must be set in `FreeRTOSConfig.h`:

```c
// Required for Tonbandgerät to access task/queue ID fields:
#define configUSE_TRACE_FACILITY    1

// Required to identify the idle task(s) at startup:
#define INCLUDE_xTaskGetIdleTaskHandle  1
```

Tonbandgerät will produce a compile-time error if either of these is missing.

## Tonbandgerät Configuration

Enable FreeRTOS tracing in `tband_config.h`:

```c
#define tband_configFREERTOS_TRACE_ENABLE 1
```

Three sub-options control which categories of events are recorded. All default
to `1` (enabled) and can be set to `0` to reduce the number of generated
events:

```c
// Task scheduling events (switches, delays, priority changes, ...):
#define tband_configFREERTOS_TASK_TRACE_ENABLE 1

// Queue/semaphore/mutex operations (send, receive, reset, ...):
#define tband_configFREERTOS_QUEUE_TRACE_ENABLE 1
```

Even with `tband_configFREERTOS_TASK_TRACE_ENABLE` disabled, tasks are still
assigned internal IDs and their names are still stored in the [metadata
buffer](./metadata_buf.md). Similarly, with
`tband_configFREERTOS_QUEUE_TRACE_ENABLE` disabled, queues are still assigned
IDs. Only the dynamic operation events are suppressed.

## Integration

Include `tband.h` at the end of `FreeRTOSConfig.h`:

```c
// FreeRTOSConfig.h (at the end of the file):
#ifndef __ASSEMBLER__
#include "tband.h"
#endif
```

This installs all FreeRTOS trace hooks automatically.

> [!TIP]
> The `__ASSEMBLER__` guard is required because some FreeRTOS ports include
> `FreeRTOSConfig.h` from assembly files. 

## Manual API Calls

### `tband_freertos_scheduler_started()`

Call this once, immediately after `vTaskStartScheduler()`:

```c
vTaskStartScheduler();
tband_freertos_scheduler_started();
```

This records which tasks are the idle task(s) and, if
`configUSE_TIMERS == 1`, which task is the FreeRTOS timer daemon. Without this
call, those tasks will appear as unnamed and unlabelled in the viewer. This
information is emitted as [metadata](./metadata_buf.md) and stored in the
metadata buffer even before tracing is started.

> [!NOTE]
> Future versions of FreeRTOS include a `traceSCHEDULER_STARTED` hook that
> will allow this to happen automatically. Until then, the manual call is
> required.

### Resource Naming

Queues, semaphores, and mutexes can optionally be given names that appear in
the trace viewer. These are [metadata events](./metadata_buf.md) and can be
called at any point after the resource is created, even if tracing is not
currently active:

```c
tband_freertos_queue_name(handle, name);
tband_freertos_binary_semaphore_name(handle, name);
tband_freertos_counting_semaphore_name(handle, name);
tband_freertos_mutex_name(handle, name);
tband_freertos_recursive_mutex_name(handle, name);
```

See [FreeRTOS Resource Tracing](./freertos_resources.md) for details.

## Internal Use of FreeRTOS APIs

> [!WARNING]
> Tonbandgerät uses two FreeRTOS fields that are provided specifically for
> trace facilities. **Your application code must not use these APIs**, or it
> will corrupt the tracer's internal ID assignments and produce an invalid
> trace:
>
> - `vTaskSetTaskNumber()` / `uxTaskGetTaskNumber()`: Tonbandgerät stores
>   its internal task ID in each task's `uxTaskNumber` field.
> - `vQueueSetQueueNumber()` / `uxQueueGetQueueNumber()`: Tonbandgerät
>   stores its internal queue ID in each queue's `uxQueueNumber` field.
>
> These fields are set automatically when a task or queue is created and read
> by every subsequent trace hook. Overwriting them from application code will
> cause all subsequent events for that task or queue to be attributed to the
> wrong resource in the viewer.

## Known Limitations

The following FreeRTOS features are not yet traced:

- **Stream buffers and message buffers**: `tband_configFREERTOS_STREAM_BUFFER_TRACE_ENABLE` exists as a config option but the underlying implementation is not yet complete.
- **Software timers**: timer callbacks are not traced as distinct events.
- **Event groups**: not traced.
- **Direct-to-task notifications**: not traced.
