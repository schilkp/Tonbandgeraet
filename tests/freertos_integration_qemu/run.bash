#!/bin/bash
set -e

# Move to location of this script
cd "$(dirname "$0")"
script_dir="$(pwd)"
project_root="$script_dir"/../../

# Build cli:
echo "Build CLI..."
cd "$project_root"/tools/tband-cli
cargo build --no-default-features

# Configure & build simulated target:
echo "Configuring QEMU_MPS2_AN385_FREERTOS example..."
cd "$project_root"/examples/QEMU_MPS2_AN385_FREERTOS
rm -rf build
mkdir build
cmake -G Ninja -B build .

echo "Building QEMU_MPS2_AN385_FREERTOS example..."
cd "$project_root"/examples/QEMU_MPS2_AN385_FREERTOS
ninja -C build

# Run under QEMU.
echo "Running QEMU_MPS2_AN385_FREERTOS example..."
cd "$script_dir"
rm -f trace.bin
timeout 60 qemu-system-arm \
    -machine mps2-an385 \
    -cpu cortex-m3 \
    -display none \
    -serial none \
    -monitor none \
    -semihosting-config enable=on,target=native \
    -kernel "$project_root"/examples/QEMU_MPS2_AN385_FREERTOS/build/QEMU_MPS2_AN385_FREERTOS.elf

echo "Decoding trace..."
cd "$project_root"/tools/tband-cli
cargo run --no-default-features -- --Werror conv --format bin --mode free-rtos "${script_dir}"/trace.bin

# Dump trace for check script:
echo "Dumping trace.."
cd "$project_root"/tools/tband-cli
cargo run --no-default-features -- --Werror dump --format bin --mode free-rtos "${script_dir}"/trace.bin > "${script_dir}/dump"

# Run check script:
cd "$script_dir"
echo "check.."
python check.py
