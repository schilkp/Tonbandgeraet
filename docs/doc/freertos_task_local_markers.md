# FreeRTOS Task-local Markers

Task-local markers are event and value markers that are associated with the
currently running task rather than appearing as a global track in the viewer.
In Perfetto, they show up as nested rows inside the task's own timeline, making
it easy to see what a specific task was doing at any point during its execution.

They are otherwise identical to regular [event markers](./evtmarkers.md) and
[value markers](./valmarkers.md) in usage and semantics.

## Requirement

Task-local markers require both FreeRTOS tracing and marker tracing to be
enabled:

```c
#define tband_configFREERTOS_TRACE_ENABLE 1
#define tband_configMARKER_TRACE_ENABLE   1
```

## How They Differ from Global Markers

A regular `tband_evtmarker(id, msg)` call produces an event on a global track
that is visible across the entire trace, independent of which task was running.

A `tband_freertos_task_evtmarker(id, msg)` call produces the same event but
tags it with the ID of the currently running task. The viewer places this event
inside that task's timeline. The marker ID is scoped to the task: two
different tasks can use the same marker ID without conflict, and their events
will appear on separate per-task tracks.

> [!NOTE]
> Task-local markers must only be called from task context, not from ISRs or
> from before the scheduler has started. The implementation reads the current
> task's ID via FreeRTOS internals to determine ownership.

## API

### Event Markers

```c
// Name a task-local event marker track. Metadata event.
void tband_freertos_task_evtmarker_name(uint32_t id, const char *name);

// Trace an instant event on the current task's marker track.
void tband_freertos_task_evtmarker(uint32_t id, const char *msg);

// Trace the beginning of a span event on the current task's marker track.
void tband_freertos_task_evtmarker_begin(uint32_t id, const char *msg);

// Trace the end of a span event on the current task's marker track.
void tband_freertos_task_evtmarker_end(uint32_t id);
```

### Value Markers

```c
// Name a task-local value marker track. Metadata event.
void tband_freertos_task_valmarker_name(uint32_t id, const char *name);

// Trace a numeric value on the current task's value marker track.
void tband_freertos_task_valmarker(uint32_t id, int64_t val);
```

## Example

```c
#define MARKER_WORK  0

void my_task(void *arg) {
    tband_freertos_task_evtmarker_name(MARKER_WORK, "processing");

    for (;;) {
        wait_for_data();

        tband_freertos_task_evtmarker_begin(MARKER_WORK, "");
        do_work();
        tband_freertos_task_evtmarker_end(MARKER_WORK);
    }
}
```

In the viewer, the `processing` span will appear as a nested row inside
`my_task`'s timeline, alongside the task's scheduling events.
