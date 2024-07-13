# Getting Started

## Installation

Add all source files from [`tband/src`](https://github.com/schilkp/Tonbandgeraet/tree/main/tband/src) to your project
and place all headers from [`tband/inc`](https://github.com/schilkp/Tonbandgeraet/tree/main/tband/inc) inside a folder
that is recognized as an include path.

If you are using FreeRTOS, include `tband.h` at the end of the `FreeRTOSConfig.h` header.

To use the tracer, only include `tband.h` in your code. Do not directly include any other Tonbandgerät headers.
Note that Tonbandgerät is written using C11. Older versions of C are not tested.

## Porting
Provide a `tband_port.h` header that implements all [required porting macros](./porting.md). Example implementations
can be found [here](https://github.com/schilkp/Tonbandgeraet/tree/main/tband/portable).

## Configuration

Provide a `tband_config.h` header, and configure Tonbandgerät using the [configuration macros](./config.md). Note that you
have to provide this file even if you are keeping all settings at their default value or are providing configuration through
compiler flags.

## Trace Handling

Pick a [trace handling backend](./handling.md) and implement some mechanism for transmitting trace data to the computer.
