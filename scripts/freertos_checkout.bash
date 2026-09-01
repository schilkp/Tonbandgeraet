#!/usr/bin/env bash

# Fail on error code, unknown var, and propagate errors from pipes:
set -euo pipefail

# Grab arg:
if [[ $# -ne 1 ]]; then
    echo "Usage: $0 <commit/tag>" >&2
    exit 1
fi
REF="$1"

# Move to repo root
SCRIPT_DIR="$(realpath "$(dirname "$0")")"
cd "$SCRIPT_DIR"
cd ..


# Clone if not present:
if [[ ! -d FreeRTOS-Kernel ]]; then
    echo "Cloning..."
    git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git FreeRTOS-Kernel
fi

echo "Checking-out $REF..."
cd FreeRTOS-Kernel && git checkout "$REF"
