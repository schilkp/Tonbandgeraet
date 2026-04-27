# The `EXTERNAL` Backend

The external backend is an escape hatch. It lets you replace Tonbandgerät's
entire backend implementation with your own by providing a single function.
All of the machinery that the [streaming](./streaming.md) and
[snapshot](./snapshot.md) backends handle automatically (start/stop control,
thread safety, metadata buffer integration, dropped event protection) is
bypassed and your responsibility.

Only use this backend if you have a specific need that the built-in backends
cannot satisfy and you understand the constraints involved.

## Configuration

Enable the external backend in `tband_config.h`:

```c
#define tband_configUSE_BACKEND_EXTERNAL 1
```

Note that exactly one backend must be enabled. See [Configuration](./config.md)
for details.

## What to Implement

You must provide a definition of the following function:

```c
bool tband_submit_to_backend(uint8_t *buf, size_t len, bool is_metadata);
```

- **`buf`**: pointer to a buffer containing one serialized trace event.
- **`len`**: length of the event in the buffer.
- **`is_metadata`**: `true` if this event is a [metadata event](./metadata_buf.md)
  (a task name, queue kind, marker name, etc.). `false` for all other events.

**Return value:**
- `false`: the event was accepted successfully.
- `true`: the event was dropped. Tonbandgerät will increment its internal
  [dropped event counter](./dropped_evts.md) and attempt to report the drop
  in the next trace event.

This function is called by Tonbandgerät for every trace event that is
generated on every core, unconditionally.

## Warnings

> [!WARNING]
> **This function is called from within a Tonbandgerät
> [critical section](./porting_critical_sections.md), from any context.**
> Depending on your port and instrumentation, this includes ISR handlers,
> the FreeRTOS scheduler, and FreeRTOS kernel internals.
>
> This means your implementation must complete immediately and must not:
> - Call any RTOS API (queue sends, semaphore gives, task notifications, ...)
> - Block or busy-wait on any condition
> - Call any Tonbandgerät API

> [!WARNING]
> **There is no start/stop control.** The streaming and snapshot
> backends gate event delivery behind a global `tracing_enabled` flag that is
> managed through their respective APIs (`tband_start_streaming()`,
> `tband_trigger_snapshot()`, etc.). None of this exists for the external
> backend. Your `tband_submit_to_backend` will be called for every event from
> the moment any tracing hook first fires. If you want to only record during a
> specific window, you must implement that logic yourself.

> [!WARNING]
> The functions `tband_tracing_enabled()` and `tband_tracing_finished()` will
> not reflect your backend's state, as they track the internal flag that only
> the built-in backends manage.

> [!WARNING]
> **There is no automatic metadata buffer integration.** The snapshot and
> streaming backends transparently accumulate metadata events into a per-core
> [metadata buffer](./metadata_buf.md) and handle transmitting its contents at
> the right time. With the external backend, all of that is gone. The
> `is_metadata` argument tells you whether a given event is metadata, but what
> you do with that information (if anything) is entirely up to you.

> [!WARNING]
> **There is no automatic thread safety.** The built-in backends protect their
> internal state with per-core spinlocks to guarantee correct behavior in
> multicore configurations. If you access shared state from
> `tband_submit_to_backend`, you are responsible for synchronizing access
> correctly, keeping in mind that it may be called concurrently from multiple
> cores.
