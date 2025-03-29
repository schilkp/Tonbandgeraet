#!/bin/bash
set -e

# Move to location of this script
cd "$(dirname "$0")"

cd ../
set -v
cd FreeRTOS-Kernel && git checkout V10.3.1-kernel-only

