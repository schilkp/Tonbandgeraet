#!/bin/bash
set -e

# Move to location of this script
cd "$(dirname "$0")"

# Run codegen
poetry install
poetry run python ./codegen
