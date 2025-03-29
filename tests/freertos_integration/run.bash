#!/bin/bash
set -e

# Move to location of this script
cd "$(dirname "$0")"
script_dir="$(pwd)"
project_root="$script_dir"/../../

# Build cli:
echo "Build CLI..."
cd "$project_root"/tools/tband-cli
cargo build

# Configure & build simulated target:
echo "Configuring POSIX_FreeRTOS example..."
cd "$project_root"/examples/POSIX_FreeRTOS
rm -rf build
mkdir build
cmake -G Ninja -B build .

echo "Building POSIX_FreeRTOS example..."
cd "$project_root"/examples/POSIX_FreeRTOS
ninja -C build

echo "Running POSIX_FreeRTOS example..."
cd "$project_root"/examples/POSIX_FreeRTOS
WRITE_TRACE="$script_dir/trace.bin" ./build/POSIX_FreeRTOS

echo "Decoding trace..."
cd "$script_dir"/../../tools/tband-cli
cargo run -- conv --format bin --mode free-rtos "${script_dir}"/trace.bin
