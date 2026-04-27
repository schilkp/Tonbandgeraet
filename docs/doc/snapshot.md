# The `SNAPSHOT` Backend

The snapshot backend records trace events into a statically allocated, per-core
buffer until it fills up, then stops tracing automatically. This makes it the
simplest backend to use: there is no need to handle trace data in real-time,
and the buffer can be read out and transmitted at any point with any interface
after recording has finished.

## Overview

When the snapshot backend is active, every trace event is appended to an
internal, per-core buffer of size `tband_configBACKEND_SNAPSHOT_BUF_SIZE`.
Once a buffer fills up, tracing stops globally and the optional
`tband_portBACKEND_SNAPSHOT_BUF_FULL_CALLBACK` hook is invoked to notify your
code that recording has completed.

The buffer can then be read out and transmitted to the host over any interface
(UART, USB, a debugger).

If the [metadata buffer](./metadata_buf.md) is enabled, its contents must be
transmitted alongside the snapshot buffer to produce a valid trace. See
[Retrieving the Trace](#retrieving-the-trace) below.

If tonbandgeraet is used in a multi-core configuration, the
`tband_portBACKEND_SNAPSHOT_BUF_FULL_CALLBACK` is called on the core which
first fills it's buffer. While this immediately signals to all other cores to
stop finishing tracing, other cores might take a short while to finish processing
any tracing events that are already in flight. Use `tband_tracing_finished` to
check whether all cores have finished before transmitting all snapshot buffers.

## Configuration

Enable the snapshot backend in `tband_config.h`:

```c
#define tband_configUSE_BACKEND_SNAPSHOT 1
```

Note that exactly one backend must be enabled. See [Configuration](./config.md)
for details.

The size of each per-core snapshot buffer (in bytes) is configured with:

```c
#define tband_configBACKEND_SNAPSHOT_BUF_SIZE 32768
```

The default is 32768 bytes. In multi-core configurations, one independent
buffer of this size is allocated per core.

## Optional Porting

### `tband_portBACKEND_SNAPSHOT_BUF_FULL_CALLBACK()`
- Required: `NO`

If defined, this macro is called when the snapshot buffer fills up and tracing
stops. It provides a way to signal the rest of your application that recording
is complete.

> [!WARNING]
> This callback is invoked from within the tracing hook of the first event that
> could not be stored, and therefore may be called from any context (application
> code, interrupts, the RTOS scheduler, ...). *No FreeRTOS APIs may be called
> from inside this callback*
>
>
> It is called from within a Tonbandgerät [critical section](./porting_critical_sections.md)
> while internal spinlocks are held. **No Tonbandgerät APIs may be called from
> inside this callback.**

> [!NOTE]
> In a multi-core setup, this callback is called exactly once on whichever
> core first fills its buffer. Other cores may still be in the process of
> writing their last event when the callback fires. Use `tband_tracing_finished()`
> to determine when all cores have fully stopped.

#### Example:
```c
extern volatile bool snapshot_done;
#define tband_portBACKEND_SNAPSHOT_BUF_FULL_CALLBACK() snapshot_done = true

// For example, in systick handler:
void systick_irq() {
    // ...
    if (snapshot_done) {
        if (tband_tracing_finished()) {
            signal_that_trace_is_done_to_tx_task();
            snapshot_done = false;
        }
    }
}
```

## API Functions

### `tband_trigger_snapshot()`

Start recording a snapshot.

**Prototype:**
```c
int tband_trigger_snapshot(void);
```

**Return values:**
- `0`:  Snapshot started successfully
- `-1`: Tracing is already active

Enables the snapshot backend. Trace events will be appended to the per-core
buffers until they fill up, at which point tracing stops automatically.

> [!TIP]
> The metadata buffer, if enabled, accumulates metadata events independently
> of the backend state.

### `tband_stop_snapshot()`

Stop recording before the buffer fills up.

**Prototype:**
```c
int tband_stop_snapshot(void);
```

**Return values:**
- `0` : Stopped successfully
- `-1`: Tracing was not active

Manually halts snapshot recording.

After calling this function, some events may still be in flight on individual
cores. Use `tband_tracing_finished()` to confirm that all cores have stopped
before reading the buffers.

**Example:**
```c
tband_stop_snapshot();

while (!tband_tracing_finished()) {
    // Wait for all cores to finish
}
```

### `tband_reset_snapshot()`

Clear the snapshot buffers so a new snapshot can be recorded.

**Prototype:**
```c
int tband_reset_snapshot(void);
```

**Return values:**
- `0`: Buffers cleared successfully
- `-1`: Tracing is still active

Resets the write index of all per-core snapshot buffers to zero. Must only be
called after tracing has fully stopped (i.e., after `tband_tracing_finished()`
returns `true`). After a successful reset, `tband_trigger_snapshot()` can be
called again to record a fresh snapshot.

### `tband_get_core_snapshot_buf()`

Get a pointer to a core's snapshot buffer.

**Prototype:**
```c
const volatile uint8_t *tband_get_core_snapshot_buf(unsigned int core_id);
```

Returns a pointer to the raw snapshot buffer for the given core. The buffer is
statically allocated and its address does not change.

> [!NOTE]
> Always wait until `tband_tracing_backend_finished(core_id)` returns `true`
> before reading a core's buffer.

> [!NOTE]
> The amount of valid data in a specific buffer that should be transmitted is
> given by `tband_get_core_snapshot_buf_amnt()`.

### `tband_get_core_snapshot_buf_amnt()`

Get the number of valid bytes in a core's snapshot buffer.

**Prototype:**
```c
size_t tband_get_core_snapshot_buf_amnt(unsigned int core_id);
```

Returns the number of bytes that have been written to the snapshot buffer for
the given core. Returns `0` if that core's backend has not yet finished.
Wait for `tband_tracing_backend_finished(core_id)` or `tband_tracing_finished()`
before using this value.

## Retrieving the Trace

Once tracing has stopped, you need to transmit two pieces of data to the host
for each core to produce a valid trace:

1. **The metadata buffer** (if `tband_configUSE_METADATA_BUF` is enabled)
   contains names of tasks, queues, markers, etc. that were generated before
   or during the snapshot. Retrieved with `tband_get_metadata_buf()` and
   `tband_get_metadata_buf_amnt()`.
2. **The snapshot buffer** contains the recorded trace events. Retrieved
   with `tband_get_core_snapshot_buf()` and `tband_get_core_snapshot_buf_amnt()`.

The metadata buffer contents should be transmitted before the snapshot data
so that the converter can resolve all names correctly.

**Example:**
```c
// Wait for all backends to finish
while (!tband_tracing_finished()) {
    // Spin or yield
}

for (unsigned int core = 0; core < tband_portNUMBER_OF_CORES; core++) {

    // Transmit metadata buffer (if enabled)
    size_t meta_len = tband_get_metadata_buf_amnt(core);
    if (meta_len > 0) {
        const volatile uint8_t *meta_buf = tband_get_metadata_buf(core);
        transmit(meta_buf, meta_len);
    }

    // Transmit snapshot buffer
    size_t snap_len = tband_get_core_snapshot_buf_amnt(core);
    if (snap_len > 0) {
        const volatile uint8_t *snap_buf = tband_get_core_snapshot_buf(core);
        transmit(snap_buf, snap_len);
    }
}
```

> [!TIP]
> Because the snapshot buffer is static and transmission does not have to be
> real-time, this is a good place to use blocking I/O such as a simple blocking
> UART write, a USB bulk transfer, or even copying the buffer out over a
> debugger. As long as you take care with what you call in the
> `tband_configBACKEND_SNAPSHOT_BUF_FULL_CALLBACK` to signal to your code that
> the data is ready, you can then use whatever is easiest to transmit.

## Multi-core Considerations

In a multi-core configuration, each core records into its own independent
buffer of `tband_configBACKEND_SNAPSHOT_BUF_SIZE` bytes.

Tracing stops globally as soon as *any* core's buffer fills. The
`tband_portBACKEND_SNAPSHOT_BUF_FULL_CALLBACK` callback is invoked exactly
once, on the core that triggered the stop. Other cores may still finish writing
their current event before they notice that tracing has been disabled. Use
`tband_tracing_finished()` to wait for all of them before reading any buffer.

To retrieve data for all cores, iterate over each `core_id` from `0` to
`tband_portNUMBER_OF_CORES - 1` and use `tband_tracing_backend_finished()`,
`tband_get_core_snapshot_buf()`, and `tband_get_core_snapshot_buf_amnt()` for
each.
