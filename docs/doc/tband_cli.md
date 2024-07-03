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
```text
{{#include ./cli_help/main.txt}}
```

### `conv`

```text
{{#include ./cli_help/conv.txt}}
```

### `dump`

```text
{{#include ./cli_help/dump.txt}}
```

### `completion`

```text
{{#include ./cli_help/compl.txt}}
```

> TODO
