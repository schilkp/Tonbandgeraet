_default:
    just --list

# === Utils ===

# Run code generator script.
codegen:
    cd codegen && poetry install
    cd codegen && poetry run python ./codegen

# === Testing ===

# Run unit tests.
unit_tests:
    #!/bin/env bash
    cd tests/unit_test
    rm -rf build
    mkdir -p build
    cmake -B build -DCMAKE_GENERATOR="Unix Makefiles"
    make -C build -j 4
    python run_tests.py -t 0.5 -j 4 build/bin/

# Run bare-metal integration tests.
baremetal_integration_tests:
    cd ./tests/baremetal_integration/ && ./test.bash

# === FreeRTOS checkout ===

# Setup project FreeRTOS checkout.
freertos_clone:
    rm -rf FreeRTOS-Kernel
    git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git FreeRTOS-Kernel

# Put project FreeRTOS checkout at latest commit.
freertos_checkout_main:
    cd FreeRTOS-Kernel && git checkout main

# Put project FreeRTOS checkout at v10.3.1.
freertos_checkout_v10_3_1:
    cd FreeRTOS-Kernel && git checkout V10.3.1-kernel-only

# Put project FreeRTOS checkout at v11.1.0.
freertos_checkout_v11_1_0:
    cd FreeRTOS-Kernel && git checkout V11.1.0

# === Website ===

# Build WASM bindings.
build_wasm:
    cd web/tband-wasm && wasm-pack build

# Setup website environment.
setup_website:
    cd web/website && npm install

# Serve website from dev server.
serve_website:
    cd web/website && npm run dev

# Build website.
build_website:
    cd web/website && npm run build
