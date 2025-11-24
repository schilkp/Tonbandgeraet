#!/bin/bash
set -e

# Move to location of this script
cd "$(dirname "$0")"

# Run codegen
uv run python ./codegen
