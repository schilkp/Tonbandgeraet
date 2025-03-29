#!/bin/bash
set -e

# Move to location of this script
cd "$(dirname "$0")"
script_dir="$(pwd)"

# Build cli:
echo "Build CLI..."
cd "$script_dir"/../../tools/tband-cli
cargo build

# Configure & build simulated target:
echo "Configuring simulated target.."
cd "$script_dir"/simulated_target
rm -rf build
mkdir -p build
cmake -B build -G Ninja
echo "Building simulated target.."
ninja -C build

# Run simulated target:
echo "Running simulated target.."
cd "$script_dir"
./simulated_target/build/bin/simtarget

# Decode trace to ensure it is valid:
echo "Decoding trace.."
cd "$script_dir"/../../tools/tband-cli
cargo run -- conv --format bin --mode bare-metal "${script_dir}"/trace.bin

# Dump trace for check script:
echo "Dumping trace.."
cd "$script_dir"/../../tools/tband-cli
cargo run -- dump --format bin --mode bare-metal "${script_dir}"/trace.bin > "${script_dir}/dump"

# Run check script:
cd "$script_dir"
cat dump
echo "check.."
python check.py
