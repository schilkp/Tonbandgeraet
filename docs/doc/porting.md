# Porting

Tonbandgerät invokes the macros below for all platform-specific operations. They
must *all* be implemented for Tonbandgerät to function properly!

## `tband_portTIMESTAMP()`

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

Resolution of the timestamp timer, in nanoseconds.

#### Example:
```c
// Timestamp counter increases every 10ns:
#define tband_portTIMESTAMP_RESOLUTION_NS (10)
```

## `tband_portENTER_CRITICAL_FROM_ANY()`

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

Number of cores on which the tracer is running. See [Multicore Support](./multicore_support.md) for more
details.

#### Example:
```c
// Single core:
#define tband_portNUMBER_OF_CORES (1)
```

## `tband_portGET_CORE_ID()`

Detect on which core the current execution context is running. See [Multicore Support](./multicore_support.md) for more
details. This macro **must** return a value between 0 and `tband_portNUMBER_OF_CORES - 1` inclusive.

#### Example:
```c
// Single core, always running on core 0:
#define tband_portGET_CORE_ID (0)
```

