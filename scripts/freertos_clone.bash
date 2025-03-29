#!/bin/bash
set -e

# Move to location of this script
cd "$(dirname "$0")"

cd ../
set -v
rm -rf FreeRTOS-Kernel
git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git FreeRTOS-Kernel
