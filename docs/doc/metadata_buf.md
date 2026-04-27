# The Metadata Buffer

## Motivation

Some trace events describe the static structure of the firmware rather than
dynamic events: the name of a FreeRTOS task, the kind of a queue, the name of
an event marker track, the timestamp timer resolution. These are called
*metadata events*.

The problem is that these events are naturally generated very early: Tasks are
created and named during startup, marker names are set during initialisation, etc.
Often, this takes place long before tracing begins. If the backend is not yet
active at that point, those events are lost and the trace will show unnamed
tasks, anonymous queues, and so on.

The metadata buffer solves this by accumulating all metadata events into a
statically allocated, per-core buffer regardless of whether the backend is
currently active. When the backend is later started, the metadata buffer
contents can be transmitted first to ensure the converter has all the
information it needs.

> [!NOTE]
> The metadata buffer is designed for typical small emebedded firmwares that
> do not dynamically create and destroy an unbounded set of resources.
> Because the size of the metadata buffer is statically bounded, such a
> firmware will inevitably overflow it.

> [!WARNING]
> 🚧 The metadata buffer detects overflows but currently this information is not reported anywhere. 🚧
> `TODO(schilkp)`

## Configuration

The metadata buffer is enabled by default:

```c
#define tband_configUSE_METADATA_BUF 1
```

Set to `0` to disable it. If disabled, any metadata events generated while the
backend is inactive are permanently lost.

The size of each per-core metadata buffer (in bytes) is configured with:

```c
#define tband_configMETADATA_BUF_SIZE 256
```
The default is 256 bytes. See [Sizing the Buffer](#sizing-the-buffer) below.

## Which Events Are Stored

Take a look at the list of [all tracing events](../tech_details/bin_events.md)
for the definite reference of events are considered metadata.

Roughly speaking, the following events are considered metadata and are stored
in the buffer:

- Timestamp timer resolution (`tband_gather_system_metadata()`)
- ISR names (`tband_isr_name`)
- Event marker names (`tband_evtmarker_name`)
- Value marker names (`tband_valmarker_name`)
- FreeRTOS task names and types (task name, idle task, timer task)
- FreeRTOS queue names and kinds

Dynamic events (task switches, queue sends/receives, marker firings, ISR
enter/exit) are not metadata and are not stored in the buffer.

## API

### `tband_get_metadata_buf()`

Get a pointer to a core's metadata buffer.

**Prototype:**
```c
const volatile uint8_t *tband_get_metadata_buf(unsigned int core_id);
```

Returns a pointer to the raw metadata buffer for the given core. The buffer is
statically allocated and its address does not change.

### `tband_get_metadata_buf_amnt()`

Get the number of valid bytes in a core's metadata buffer.

**Prototype:**
```c
size_t tband_get_metadata_buf_amnt(unsigned int core_id);
```

Returns the number of bytes currently stored in the metadata buffer for the
given core. This value can be read at any time.

## Backend Integration

How the metadata buffer is used depends on which backend is active.

### Streaming Backend

`tband_start_streaming()` automatically transmits the metadata buffer contents
for all cores before enabling event streaming. No manual handling is required.

### Snapshot Backend

The metadata buffer is not automatically included in the snapshot. After a
snapshot has been collected, both the metadata buffer and the snapshot buffer
must be transmitted to produce a valid trace. See the [snapshot backend
documentation](./snapshot.md#retrieving-the-trace) for a worked example.

## Sizing the Buffer

Each metadata event occupies a small number of bytes in the buffer. Name events
consist of a few bytes of overhead plus the length of the name string (up to
`tband_configMAX_STR_LEN`). A few representative sizes with the default
`tband_configMAX_STR_LEN` of 20:

| Event type                | Approximate size |
| ---                       | ---              |
| Timestamp resolution      | up to 11 bytes   |
| ISR / marker / queue name | up to ~28 bytes  |
| Task name                 | up to ~28 bytes  |
| Queue kind                | ~3 bytes         |

The buffer needs to be large enough to hold all metadata events that will ever
be generated. This is well-bounded as long as the set of tasks, queues,
interrupts, and markers with names is fixed: a buffer of 256 bytes comfortably
covers a handful of named resources with short names, but a project with many
tasks and queues may need a larger buffer.

> [!WARNING]
> 🚧 The metadata buffer detects overflows but currently this information is
> not reported anywhere. 🚧
> If the metadata buffer overflows, the excess events are silently discarded.
> The internal `did_ovf` flag is set but is not currently reported or surfaced
> through any API. If your trace is missing names for some resources, increase
> `tband_configMETADATA_BUF_SIZE`.
> `TODO(schilkp)`
