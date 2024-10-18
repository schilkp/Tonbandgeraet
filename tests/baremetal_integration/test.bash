#!/bin/bash
set -e

cd "$(dirname "$0")"
script_dir="$(pwd)"

# Configure & build simulated target:
cd ./simulated_target
rm -rf build
mkdir -p build
cmake -B build -DCMAKE_GENERATOR="Unix Makefiles"
make -C build -j 4

# Run simulated target:
echo "Running simulated target.."
cd "$script_dir"
./simulated_target/build/bin/simtarget

echo "Dumping trace.."
cd "$script_dir"
cd ../../conv/tband-cli
cargo run -- dump --format bin --mode bare-metal "${script_dir}"/trace.bin > "${script_dir}/dump"

cd "$script_dir"
cat dump
echo "check.."
python check.py

