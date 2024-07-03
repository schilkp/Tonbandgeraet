# CLI Trace Converter: `tband-cli`

## Compile & Install

The trace converter and CLI is written in rust 🦀. To compile and run `tband-cli` locally, first you
need to [download and install rust](https://www.rust-lang.org/learn/get-started) if you don't already have it.

Then, open the [`./conv/tband-cli`](https://github.com/schilkp/Tonbandgeraet/tree/main/conv/tband-cli) folder.

Now you have two options. To compile & install `tband-cli`, run:

```bash
> cargo install --path .
```

This will compile the tool, and place the finished executable in your local cargo binary direction. Where that
is [depends on your system](https://doc.rust-lang.org/cargo/commands/cargo-install.html). Most likely you will
have to add it to your `PATH`.

To build and run the CLI tool directly from the repository, type:

```bash
> cargo run --
```

Any command line arguments you want to provide need to go after the `--` separator.

## Commands

The tool features 5 main commands:

```text
{{#include ./cli_help/main.txt}}
```

### `conv`

The `conv` command is where most of the action is. It takes one or more trace files, decodes them, and 
converts them to the perfetto format:

```text
{{#include ./cli_help/conv.txt}}
```

It supports both binary and hex trace files, and both bare-metal and FreeRTOS traces. After conversion,
the tool can save the result to a file (`--output`), open it directly in perfetto (`--open`), or 
provide a link and host a local server to provide the trace to perfetto (`--serve`).

The input files must be given last. If converting a multi-core trace split into seperate files, 
append the core id to each file as follows:

```text
> tband-cli conv --format=bin --core-count=2 --open core0_trace.bin@0 core1_trace.bin@1
```

### `dump`

The dump command takes a single trace file, decodes it, and dumps its content in human-readable form
to stdout.

```text
{{#include ./cli_help/dump.txt}}
```

### `completion`

The completion command can generate shell completion scripts for most common shells. How you can
install a completion script depends on your shell and system configuration.

```text
{{#include ./cli_help/compl.txt}}
```
