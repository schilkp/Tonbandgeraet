# Configuration

Tonbandgerät requires a user-provided `tband_config.h` header file, where the following configuration
macros can be set:

## `tband_configENABLE`:
- Possible Values: `0, 1`
- Default: `0`

Set to `1` to enable Tonbandgerät. If disabled, all code is excluded to save space.

#### Example:
```c
// tband_config.h:
#define tband_configENABLE 1 
```

## `tband_configMAX_STR_LEN`;
- Possible Values: `1+`
- Default: `20`

Maximum string length that Tonbandgerät will serialize and trace. Serves as a safeguard
against incorrectly terminated strings, and helps providing a static upper bound on
worst-case trace hook execution time.

## `tband_configTRACE_DROP_CNT_EVERY`:
- Possible Values: `0+`
- Default: `50`

In addition to sending the [dropped event counter](./dropped_evts.md) after an event was
dropped, Tonbandgerät will also serialize and trace the number of dropped events after every
`tband_configTRACE_DROP_CNT_EVERY` normal tracing events. Set to zero to disable periodic
dropped event tracing.

## `tband_configMARKER_TRACE_ENABLE`:
- Possible Values: `0, 1`
- Default: `1`

Set to 0 to disable serialization and tracing of calls to [event markers](./evtmarkers.md) and [value markers](./valmarkers.md) functions.
Can be disabled to reduce the number of generated events.

## `tband_configISR_TRACE_ENABLE`:
- Possible Values: `0, 1`
- Default: `1`

Set to 0 to disable serialization and tracing of [interrupts](./interrupts.md). Can be disabled to reduce the number of generated events.

---
# Metadata Buffer Config:

## `tband_configUSE_METADATA_BUF`:
- Possible Values: `0, 1`
- Default: `1`

Enables the [metadata buffer](./metadata_buf.md) if set to `1`.

## `tband_configMETADATA_BUF_SIZE`:
- Possible Values: `1+`
- Default: `256`

Size of the [metadata buffer](./metadata_buf.md) in bytes, if enabled.

---
# Streaming Backend Config:

## `tband_configUSE_BACKEND_STREAMING`:
- Possible Values: `0, 1`
- Default: `0`

Set to `1` to enable the [streaming backend](./streaming.md). Note that exactly one backend must be
enabled!

---
# Snapshot Backend Config:

## `tband_configUSE_BACKEND_SNAPSHOT`:
- Possible Values: `0, 1`
- Default: `0`

Set to `1` to enable the [snapshot backend](./snapshot.md). Note that exactly one backend must be
enabled!

## `tband_configBACKEND_SNAPSHOT_BUF_SIZE`:
- Possible Values: `1+`
- Default: `32768`

Size of the per-core [snapshot buffer](./snapshot.md) in bytes, if enabled.

---
# Post-Mortem Backend Config:

## `tband_configUSE_BACKEND_POST_MORTEM`:
- Possible Values: `0, 1`
- Default: `0`

Set to `1` to enable the [post-mortem backend](./postmortem.md). Note that exactly one backend must be
enabled!

```admonish warning
The post-mortem backend has not been implemented yet.
```

---
# External Backend Config:

## `tband_configUSE_BACKEND_EXTERNAL`:
- Possible Values: `0, 1`
- Default: `0`

Set to `1` to enable the [external backend](./external_backend.md). Note that exactly one backend must be
enabled!

---
# FreeRTOS Tracing Config:

## `tband_configFREERTOS_TRACE_ENABLE`:
- Possible Values: `0, 1`
- Default: `0`

Set to `1` to enable [FreeRTOS](./freertos.md) tracing.

## `tband_configFREERTOS_TASK_TRACE_ENABLE`:
- Possible Values: `0, 1`
- Default: `1`

Set to 0 to disable serialization and tracing of FreeRTOS task scheduling and execution.
Can be disabled to reduce the number of generated events.

## `tband_configFREERTOS_QUEUE_TRACE_ENABLE`:
- Possible Values: `0, 1`
- Default: `1`

Set to 0 to disable serialization and tracing of FreeRTOS queue operations.
Can be disabled to reduce the number of generated events.

## `tband_configFREERTOS_STREAM_BUFFER_TRACE_ENABLE`:
- Possible Values: `0, 1`
- Default: `1`

Set to 0 to disable serialization and tracing of FreeRTOS stream buffer operations.
Can be disabled to reduce the number of generated events.

```admonish warning
Stream buffers are not yet supported.
```
