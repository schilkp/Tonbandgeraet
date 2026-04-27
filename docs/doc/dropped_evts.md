# Dropped Events

## Overview

Some backends, such as the [streaming backend](./streaming.md) implementation,
may fail to transmit a trace event due to bandwidth limits or other breakdowns.

To handle such cases, Tonbandgerät backends can signal to the trace handling
core that an individual trace event could not be submitted and traced. 

Because Tonbandgerät buffering and re-transmitting suche events could both 
significantly slow down the firmware and make the time required to complete
an individual trace hook even more unpredictable, no effort is made to 
ensure all events are actually transmitted.

However, Tonbandgerät does track such dropped events and ensures a dropped
event marker is inserted into the trace, informing the user that some events
were dropped and the trace might not perfectly reflect reality.

> [!NOTE]
> If your trace contains dropped events, all recordings past that point *may be*
> corrupted.

## The Dropped Event Counter

Tonbandgerät maintains a single global counter that increments each time an
event is dropped. This counter is shared across all cores.

Whenever a drop occurs, Tonbandgerät will attempt to encode and submit a
*dropped event counter event*  (a special trace event that carries the current
value of the counter) on every subsequent trace event until one such counter
event is successfully transmitted. This ensures the converter always learns
about losses as soon as the channel recovers, assuming the dropped-event
detecting reported by the backend is accurate.

If the counter event itself cannot be transmitted (the channel is still
congested), the counter is incremented again and the actual event that triggered
the attempt is abandoned. The next trace event will again try to submit a dropped
event counter message.

## Periodic Submission

As an additional safety measure, Tonbandgerät can also submit a dropped event
counter event periodically, even when no drops have occurred. This is controlled
by `tband_configTRACE_DROP_CNT_EVERY`:

```c
// tband_config.h
#define tband_configTRACE_DROP_CNT_EVERY 50  // default
```

When set to a value `N > 0`, a dropped event counter event is submitted
approximately every `N` trace events. This gives the converter a regular
heartbeat it can use to confirm that no events were silently lost and provides
another assurance that the trace is accurate.

Set to `0` to disable periodic submission entirely. In that case, a counter
event is only emitted when the drop count actually changes. This saves a small
amount of bandwidth at the cost of the safety net.

> [!TIP]
> The overhead of periodic submission is very small at the default interval of
> 50. It is generally worth keeping enabled unless bandwidth is extremely
> constrained.

## In the Trace Viewer

When the converter detects a non-zero or increasing dropped event counter in the
trace stream, it emits a log message to inform you that events were lost. This
appears in the converter output alongside the converted trace. 
