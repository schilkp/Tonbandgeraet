_default:
    just --list

#### barectf ###################################################################

setup_nanopb:
    rm -rf tracer/env
    python3 -m venv tracer/env
    tracer/env/bin/pip3 install nanopb==0.4.8 setuptools

compile_protos:
    protoc --python_out=converter/protos/ --pyi_out=converter/protos/ FreeRTOS_trace.proto
    tracer/env/bin/nanopb_generator FreeRTOS_trace.proto -Dtracer/protos/

quicktest:
    just build
    just flash
    just quicktest_receive
    just quicktest_convert

quicktest_receive:
    rm -f trace.hex
    just flush_uart
    just uart_receive

quicktest_convert:
    rm -f out.json
    python converter/trace_converter.py > out.json

#### Example Firmware ##########################################################

gdb_port := "12312"
exe_prog := "STM32_Programmer_CLI"
exe_gdbserver := "ST-LINK_gdbserver"
exe_gdb := "arm-none-eabi-gdb"

firmware_folder_path := justfile_directory() / "Example_STM32_FW"
firmware_name := "FreeRTOS_Integration"
build_dir_debug := firmware_folder_path / "build" / "Debug"
elf_debug := build_dir_debug / firmware_name + ".elf"

# Use Ninja to compile the program in debug mode.
configure:
    @echo Configuring...
    mkdir -p {{build_dir_debug}}
    cd {{firmware_folder_path}} && cmake -B {{build_dir_debug}} -DCMAKE_GENERATOR=Ninja

# Use Ninja to compile the program in debug mode.
build:
    @echo Building...
    ninja -C {{build_dir_debug}}

# Use Ninja to compile the program in debug mode with verbose output.
build_verbose:
    @echo Building...
    ninja --verbose -C {{build_dir_debug}}

# Use Ninja to remove all build artifacts.
clean:
    @echo Cleaning...
    ninja -C {{build_dir_debug}} clean

distclean:
    @echo Full cleaning...
    rm -rf {{firmware_folder_path}}/build

# Erase STM32.
erase:
    @echo Erasing...
    {{exe_prog}} -c port=SWD freq=4000 -e all

# Flash firmware on STM32.
flash: build
    @echo Flashing...
    {{exe_prog}} -c port=SWD freq=4000 -w {{elf_debug}}
    {{exe_prog}} -c port=SWD freq=4000 -rst

# UART monitor.
uart_monitor port="/dev/ttyACM0":
    @echo Exit picocom: CTRL-A CTRL-X
    picocom --baud 115200 --imap lfcrlf {{port}}

# UART receive.
uart_receive port="/dev/ttyACM0":
    @echo Exit picocom: CTRL-A CTRL-X
    picocom --baud 115200 --imap lfcrlf --logfile trace.hex {{port}} --exit-after 2000

# Flush uart.
flush_uart port="/dev/ttyACM0":
    picocom --baud 115200 --imap lfcrlf {{port}} --exit-after 100

# Start ST-Link debug server.
debug_server:
    @echo Starting debug server..
    {{exe_gdbserver}} -d -v -t -e -cp $(dirname $(which {{exe_prog}})) -p 12312

# Launch GDB, connect to debug server, flash, and halt at main.
gdb: build
    {{exe_gdb}} {{elf_debug}} \
    --eval-command="set pagination off" \
    --eval-command="tui enable" \
    --eval-command="echo \n Connecting to target remote :{{gdb_port}}...\n\n" \
    --eval-command="target remote :{{gdb_port}}" \
    --eval-command="echo \n Flashing..\n\n" \
    --eval-command="load" \
    --eval-command="echo \n Resetting and running to main..\n\n" \
    --eval-command="b main" \
    --eval-command="monitor reset" \
    --eval-command="c"
