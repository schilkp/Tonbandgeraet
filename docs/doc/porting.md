# Porting

Tonbandgerät invokes the macros below for all platform-specific operations. They
must *all* be implemented for Tonbandgerät to function properly!

## `tband_portTIMESTAMP()`
- Required: `YES`
- Return type: `uint64_t`

Get current value of the 64bit unsigned monotonic timestamp timer. The timer should have a
resolution/frequency of `tband_portTIMESTAMP_RESOLUTION_NS` (see below).

Note that the timer must be shared between all cores. Smaller timers are allowed, but the
return value of this macro should be `uint64_t`.

#### Example:
```c
uint64_t platform_ts(void);
#define tband_portTIMESTAMP() platform_ts()
```

## `tband_portTIMESTAMP_RESOLUTION_NS`
- Required: `YES`
- Value type: `uint64_t`

Resolution of the timestamp timer, in nanoseconds.

#### Example:
```c
// Timestamp counter increases every 10ns:
#define tband_portTIMESTAMP_RESOLUTION_NS (10)
```

## `tband_portENTER_CRITICAL_FROM_ANY()`
- Required: `YES`

Enter a critical section from any context. For precise details of what properties
a critical section must have, see [here](./porting_critical_sections.md).

#### Example:
```c
// FreeRTOS, ARM CM4F:
#define tband_portENTER_CRITICAL_FROM_ANY()            \
  bool tband_port_in_irq = xPortIsInsideInterrupt();   \
  BaseType_t tband_port_key = 0;                       \
  if (tband_port_in_irq) {                             \
    tband_port_key = taskENTER_CRITICAL_FROM_ISR();    \
  } else {                                             \
    taskENTER_CRITICAL();                              \
    (void)tband_port_key;                              \
  }
```

## `tband_portEXIT_CRITICAL_FROM_ANY()`
- Required: `YES`

See `tband_portENTER_CRITICAL_FROM_ANY()` above.

#### Example:
```c
// FreeRTOS, ARM CM4F:
#define tband_portEXIT_CRITICAL_FROM_ANY()             \
  if (tband_port_in_irq) {                             \
    taskEXIT_CRITICAL_FROM_ISR(tband_port_key);        \
  } else {                                             \
    taskEXIT_CRITICAL();                               \
  }
```

## `tband_portNUMBER_OF_CORES`
- Required: `YES`
- Return type: `uint32_t`

Number of cores on which the tracer is running. See [Multicore Support](./multicore_support.md) for more
details.

#### Example:
```c
// Single core:
#define tband_portNUMBER_OF_CORES (1)
```

## `tband_portGET_CORE_ID()`
- Required: `YES`
- Return type: `uint32_t`

Detect on which core the current execution context is running. See [Multicore Support](./multicore_support.md) for more
details. This macro **must** return a value between 0 and `tband_portNUMBER_OF_CORES - 1` inclusive.

#### Example:
```c
// Single core, always running on core 0:
#define tband_portGET_CORE_ID (0)
```
---
# Streaming Backend Porting

## `tband_portBACKEND_STREAM_DATA(buf, len)`
- Required: `YES` (if streaming backend is enabled)
- Return type: `bool`
- Arg. 1 type: `const uin8_t*`
- Arg. 2 type: `size_t`

Required if the [streaming backend](./streaming.md) is used. Called by Tonbandgerät to
submit data that is to be streamed. Return value of `true` indicates that data could *not*
be streamed and was dropped. Return value of `false` indicates that data was not dropped
and succesfully streamed.

#### Example:
```c
bool stream_data(const uin8_t* buf, size_t buf_len);
#define tband_portBACKEND_STREAM_DATA(buf, len) stream_data(buf, len)
```

---
# Snapshot Backend Porting

## `tband_portBACKEND_SNAPSHOT_BUF_FULL_CALLBACK()`
- Required: `NO`
- Return type: `void`

If the [snapshot backend](./snapshot.md) is active and stops because of a full
snapshot buffer, this callback is called.

Note that this macro is called *from within the tracing hook of the first event that could not be stored in the buffer*, and
therefor may be called from any context (interrupts, RTOS tasks, RTOS scheduler, ...).

Because the callback is always called from within a Tonbandgerät [critical section](./porting_critical_sections.md) and
while certain internal spin locks are held, *no Tonbandgerät APIs may be called from inside this callback*.

Even in a multicore setp, this callback is called only once on the one core that first filled
its buffer.

Furthermore, note that this callback is called once the (first) buffer is full, but it may some moments for the snapshot
backend to finish, especially on all cores.

#### Example:
```c
extern volatile bool snapshot_full;
#define tband_portBACKEND_SNAPSHOT_BUF_FULL_CALLBACK() snapshot_full = true
```
