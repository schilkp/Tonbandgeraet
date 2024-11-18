#!/bin/bash 
set -e
set -o xtrace

cd "$(dirname "$0")"
script_dir="$(pwd)"

# Grab jelp messages
cd ../tools/
main_help=$(cargo run -- --help)
conv_help=$(cargo run -- conv --help)
dump_help=$(cargo run -- dump --help)
compl_help=$(cargo run -- completion --help)

cd "$script_dir"

echo "> tband-cli --help" > ./doc/cli_help/main.txt
echo "$main_help" >> ./doc/cli_help/main.txt

echo "> tband-cli conv --help" > ./doc/cli_help/conv.txt
echo "$conv_help" >> ./doc/cli_help/conv.txt

echo "> tband-cli dump --help" > ./doc/cli_help/dump.txt
echo "$dump_help" >> ./doc/cli_help/dump.txt

echo "> tband-cli completion --help" > ./doc/cli_help/compl.txt
echo "$compl_help" >> ./doc/cli_help/compl.txt
