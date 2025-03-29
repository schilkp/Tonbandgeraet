#!/bin/env bash
set -e

# Move to location of this script
cd "$(dirname "$0")"

cd ../

echo "==== Configuring POSIX_FreeRTOS ===="
cd examples/POSIX_FreeRTOS
rm -rf build
mkdir build
cmake -G Ninja -B build .

echo "==== Building POSIX_FreeRTOS ===="
ninja -C build
cd ../../

echo "==== Generating Trace ===="
DUMP_DEMO_TRACE=1 ./examples/POSIX_FreeRTOS/build/POSIX_FreeRTOS > ./web/website/demo_traces/freertos.json
