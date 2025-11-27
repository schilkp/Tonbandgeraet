# The `STREAMING` Backend

The streaming backend transmits trace data in real-time as events occur. Each
trace event is immediately passed to a user-provided function for transmission,
making it suitable for live tracing over serial ports, debug ports, etc.

## Overview

When the streaming backend is active, every trace event is serialized and
immediately forwarded to the `tband_portBACKEND_STREAM_DATA` hook for
transmission, which must be provided.

If the [metadata buffer](./metadata_buf.md) is enabled, its contents are
automatically transmitted when streaming is started.

## Configuration

Enable the streaming backend in `tband_config.h`:

```c
#define tband_configUSE_BACKEND_STREAMING 1
```
Note that exactly one backend must be enabled. See [Configuration](./config.md)
for details.

## Required Porting

When using the streaming backend, you must implement the
`tband_portBACKEND_STREAM_DATA` macro in your `tband_config.h` or
`tband_port.h` file.

### `tband_portBACKEND_STREAM_DATA(buf, len)`
- Arg. 1: (type: `const uint8_t*`) buffer containing the trace event to transmit.
- Arg. 2: (type: `size_t`) length of the event in the buffer.

**Return value:**
- `true` - Data could not be transmitted and was dropped
- `false` - Data was successfully transmitted

This hook is called by Tonbandgerät for every trace event that is generated.

```admonish info
Tonbandgerät will only attempt to transmit any given trace event once. If
the `tband_portBACKEND_STREAM_DATA` hook returns true, indicating the event
was dropped, Tonbandgerät will drop the event and increment the internal dropped-
event counter. This causes Tonbandgerät to generate droppped-event marker events,
which are generated until one was able to be streamed/submitted sucessfully.
```

```admonish warning
This hook is called from within tracing hooks, and from within
[critical sections](./porting_critical_sections.md). These hooks may be called
from any context, such as an RTOS kernel or interrupts, depending on your
implementation and usage of Tonbandgerät.

This limits your ability to call RTOS APIs, or perform any kind of blocking
operation. Doing so might not work, or even worse, might cause some very
strange and hard to track-down behaviour.

Consider using a communication port specifically designed for such scenarios
(such as SEGGER's RTT), or use this hook to place data into some form of
static buffer that is processed asynchronously.
```

#### Example:
```c
bool stream_data(const uint8_t* buf, size_t len);
#define tband_portBACKEND_STREAM_DATA(buf, len) stream_data(buf, len)
```

## API Functions

### `tband_start_streaming()`

Start streaming trace data.

**Prototype:**
```c
int tband_start_streaming(void);
```

**Return values:**
- `0` - Streaming started successfully
- `-1` - Streaming is already active
- `-2` - Failed to transmit [metadata buffer](./metadata_buf.md) contents (if metadata buffer is enabled)

If the [metadata buffer](./metadata_buf.md) is enabled, this function
automatically transmits the metadata buffer contents for all cores before
enabling trace event streaming. This ensure all the trace events generated
during resource setup (such as names/types of queues, tasks, etc) are
transmitted for a proper trace, even if streaming is not enabled at that time.

### `tband_stop_streaming()`

Stop streaming trace data.

**Prototype:**
```c
int tband_stop_streaming(void);
```

**Return values:**
- `0` - Streaming stopped successfully
- `-1` - Streaming was not active

After calling this function, trace events will no longer be transmitted. The
function returns immediately; some events may still be in flight on individual
cores. Use `tband_tracing_finished()` to check when all cores have completed.

**Example:**
```c
tband_stop_streaming();

// Wait for all backends to finish processing
while (!tband_tracing_finished()) {
    // Wait
}
```

## Multi-core Considerations

In multi-core systems, each core independently calls
`tband_portBACKEND_STREAM_DATA` for its trace events. The implementation should
handle concurrent calls from multiple cores safely.

## Metadata Buffer Integration

If the [metadata buffer](./metadata_buf.md) is enabled
(`tband_configUSE_METADATA_BUF`), metadata events (like marker names and ISR
names) are stored in the per-core metadata buffers and automatically
transmitted when `tband_start_streaming()` is called.

Metadata for all cores is transmitted before regular trace event streaming
begins.
