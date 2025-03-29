#!/bin/bash
set -e

# Move to location of this script
cd "$(dirname "$0")"

rm -rf build
mkdir -p build
cmake -B build -G Ninja
ninja -C build
python runner.py -t 0.5 -j 4 build/bin/
