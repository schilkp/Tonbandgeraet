# Tonbandgerät

> **Tonbandgerät** /ˈtoːnbantɡərɛːt/ _n_.
>
> 1) German for _audio tape recorder_.
> 2) An electrical device for recording sound on magnetic tape for playback at a later time.

A small embedded systems tracer with support for bare-metal and FreeRTOS-based targets.

Philipp Schilk, 2024

---

## Overview

![A sample trace](./imgs/banner.png)

Tonbandgerät's [core](https://github.com/schilkp/Tonbandgeraet/tree/main/tband) is a small, portable, trace event generator
and handler written in C and designed for embedded systems. It requires minimal [porting](./porting.md) and [configuration](./config.md),
and features multiple [backends](./handling.md) for gathering and transmitting traces.

It can be used both with an RTOS, or in bare-metal environments to [instrument user code](./evtmarkers.md) and track hardware events by tracing [interrupts](./interrupts.md).
Full tracing of [FreeRTOS](https://www.freertos.org/index.html) tasks and resources is also supported out-of-the-box.

Tonbandgerät is based on a simple custom [binary trace format](./bin_format.md) designed to be fairly fast to encode and keep traces as small as possible. Recorded
traces can be viewed in Google's in-browser [perfetto](https://perfetto.dev) after conversion with the provided [CLI](./tband_cli.md) tool
or in-browser [converter](./web.md).

## Documentation
The documentation for Tonbandgerät can be found in the `docs/` folder and compiled for viewing with [mdbook](https://github.com/rust-lang/mdBook)
by running `mdbook build` in `docs/`. The latest version of the documentation can also be viewed online [here](https://schilk.co/Tonbandgeraet/docs/index.html).

## Trace Converter + Viewing
The [trace converter](./doc/tband_cli.md) is written in rust, can be found [here](https://github.com/schilkp/Tonbandgeraet/tree/main/conv). For convenience, there is
also a [WASM version with web frontend](./doc/web.md), which runs in the browser and can be found [here](https://schilk.co/Tonbandgeraet/).

## Licensing
The [target tracer sources](https://github.com/schilkp/Tonbandgeraet/tree/main/tband) and [documentation](https://github.com/schilkp/Tonbandgeraet/tree/main/docs) are
released under the [MIT License](https://github.com/schilkp/Tonbandgeraet/blob/main/tband/LICENSE). All conversion and
analysis tools, such as the [decoder and converter](https://github.com/schilkp/Tonbandgeraet/tree/main/tools/tband-conv),
the [CLI](https://github.com/schilkp/Tonbandgeraet/tree/main/tools/tband-cli), and the
[web converter](https://github.com/schilkp/Tonbandgeraet/tree/main/web) are released under the [GNU GPL3 License](https://github.com/schilkp/Tonbandgeraet/blob/main/tools/tband-cli/LICENSE).

## Status

> 🚧 Note
>
> Tonbandgerät is in early development and by no means considered stable. Everything - including the
> binary trace format - is subject to change.
>
> Please report any issues [here](https://github.com/schilkp/Tonbandgeraet/issues).

### Completed:

#### Tonbandgerät:
- Trace encoder.
- Streaming backend.
- Snapshot backend.
- Metadata buffer.
- Initial FreeRTOS support.

#### Conversion tools:
- CLI converter.
- In-browser converter.

#### Other:
- STM32 + FreeRTOS example project.

### Work-In-Progress:

#### Tonbandgerät:

- Support for multicore tracing, including FreeRTOS SMP:
  Implemented and theoretically (almost?) done, but completely untested. Currently this
  is limited to cores that share a single, monotonic, time stamp timer.

- Full FreeRTOS support, including some PRs: PRs are in a draft state/being
  reviewed. Certain FreeRTOS (rare) are not yet traced correctly due to insufficient
  tracing hooks. Tracing of streambuffers, direct-to-task notification, timers, and
  event groups are not yet Implemented.


#### Other:
- This documentation.

### Planned:

#### Tonbandgerät:
- Post-mortem backend.
- Task stack utilization tracing.
- Multi-core for cores without common timebase.

#### Other:
- More examples, including a bare-metal project, RTT-backed project, and
  RP2040 SMP project.
- More example ports.
