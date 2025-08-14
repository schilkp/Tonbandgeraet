# Critical Sections

## Motivation

Tonbandgerät tracing hooks can be called from any context - including interrupts. This means Tonbandgerät
requires some mechanism for preventing a higher-priority context from accessing internal state or
submitting trace events, ensuring there are no race conditions and corrupted trace events.

## Implementation Requirements

All sections of Tonbandgerät code that may not be interrupted are wrapped in
`tband_portENTER_CRITICAL_FROM_ANY()` and `tband_portEXIT_CRITICAL_FROM_ANY()` guards,
whose platform-specific implementation must be provided by the user as part of the
Tonbandgerät [port](./porting.md).

**A port must ensure that no tracing events are generated or Tonbandgerät APIs are called
while a critical section is active**. Precisely how this is to be achieved depends on the overall
software design and tracer implementation. A few example scenarios are given below.

Note that critical sections can occur in any context that tracing occurs. If you call tracing APIs in
interrupts or are using an RTOS, this may include interrupts! Furthermore, note that Tonbandgerät
critical sections do not need to be able to nest, but if used with an RTOS, may be placed
inside an RTOS critical section.

#### Bare-metal Context. No tracing is done in any interrupts.

The critical section does not have to do anything. Since interrupts don't interact with the tracer,
it does not matter if one occurs during a critical section.

#### Bare-metal Context. Tracing is done in interrupts.

The critical section must disable interrupts. If only a known subset of interrupts generate tracing
events or call tracer APIs, the critical section may also selectively disable these interrupts by
adjusting interrupt masks or priorities.

#### RTOS.

RTOS tracing will almost always generate tracing events from interrupts. A critical section must,
in this case, disable interrupts and prevent any mechanism for context switching. This is best done
by using the critical section API provided by the RTOS. Note that these APIs may have calling
semantics that differ from how Tonbandgerät's critical sections work. For example, FreeRTOS
provides separate APIs for critical sections within and outside interrupts, and can only
be used if the port supports detecting if execution is currently inside an interrupt. Using the
FreeRTOS ARM Cortex M4 port, this may be done as follows:

```c
#include "FreeRTOS.h"
#include "task.h"

#define tband_portENTER_CRITICAL_FROM_ANY()          \
  bool tband_port_in_irq = xPortIsInsideInterrupt(); \
  BaseType_t tband_port_key = 0;                     \
  if (tband_port_in_irq) {                           \
    tband_port_key = taskENTER_CRITICAL_FROM_ISR();  \
  } else {                                           \
    taskENTER_CRITICAL();                            \
    (void)tband_port_key;                            \
  }

#define tband_portEXIT_CRITICAL_FROM_ANY()           \
  if (tband_port_in_irq) {                           \
    taskEXIT_CRITICAL_FROM_ISR(tband_port_key);      \
  } else {                                           \
    taskEXIT_CRITICAL();                             \
  }
```

## Critical Sections & Multi-core Support

No adaptation of a critical section implementation is necessary when moving to a multi-core configuration,
as Tonbandgerät protects all static resources that are accessed from multiple cores with spinlocks.
