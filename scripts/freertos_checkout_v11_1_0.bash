#!/bin/bash
set -e

# Move to location of this script
cd "$(dirname "$0")"

cd ../
set -v
cd FreeRTOS-Kernel && git checkout V11.1.0

