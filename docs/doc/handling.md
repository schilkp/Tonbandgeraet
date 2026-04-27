# Trace Backends

When Tonbandgerät generates a trace event (a task switch, a marker firing, a
queue operation, ...) it serializes it into a compact binary representation and
immediately hands it off to the active *backend*. The backend is responsible
for deciding what to do with those bytes: store them in a buffer, stream them
over a wire, or hand them to custom user code.

Exactly one backend must be enabled at compile time. Enabling zero or more
than one is a compile-time error.

## Available Backends

### [Streaming](./streaming.md)

The streaming backend forwards every trace event to a user-provided hook
(`tband_portBACKEND_STREAM_DATA`) the moment it is generated. It is suitable
for live tracing over interfaces that can keep up, such as SEGGER RTT.

Because the hook is called from within a [critical
section](./porting_critical_sections.md) and may be invoked from any context
including ISRs and the RTOS kernel, the implementation is subject to strict
constraints: it must be non-blocking and must complete immediately. See the
[Streaming Backend](./streaming.md) documentation for details.

Enable with:
```c
#define tband_configUSE_BACKEND_STREAMING 1
```

### [Snapshot](./snapshot.md)

The snapshot backend accumulates trace events into a statically allocated,
per-core buffer until it fills up, then stops. The contents can be read out
and transmitted at any convenient time after recording has finished. No
real-time constraints apply during transmission.

This is the simplest and most robust backend for most use cases.

Enable with:
```c
#define tband_configUSE_BACKEND_SNAPSHOT 1
```

### [External](./external_backend.md)

The external backend lets you provide your own implementation of the internal
`tband_submit_to_backend` function, bypassing all of Tonbandgerät's built-in
backend machinery.

> [!WARNING]
> It is unlikely that you want to use this backend! Consider using the
> [streaming](./streaming.md) backend instead.

All backend responsibilities (start/stop control, thread safety, metadata
handling) fall entirely on the user. See the [External
Backend](./external_backend.md) documentation for details and warnings.

Enable with:
```c
#define tband_configUSE_BACKEND_EXTERNAL 1
```

### [Post-Mortem](./postmortem.md)

> [!WARNING]
> The post-mortem backend has not been implemented yet.

Enable with:
```c
#define tband_configUSE_BACKEND_POST_MORTEM 1
```
