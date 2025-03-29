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
    set -e
    cd tests/unit_test
    rm -rf build
    mkdir -p build
    cmake -B build -DCMAKE_GENERATOR="Unix Makefiles"
    make -C build -j 4
    python run_tests.py -t 0.5 -j 4 build/bin/

# Run bare-metal integration tests.
baremetal_integration_tests:
    cd ./tests/baremetal_integration/ && ./test.bash

# Build examples (requires a FreeRTOS checkout)
build_examples:
    #!/bin/env bash
    set -e
    if [ "${SKIP_STM32L476RGT6_FreeRTOS:-0}" != "1" ]; then
        echo "===== Configuring STM32L476RGT6_FreeRTOS ====="
        cd examples/STM32L476RGT6_FreeRTOS
        rm -rf build
        mkdir build
        cmake -G Ninja -B build .
        echo "==== Building STM32L476RGT6_FreeRTOS ===="
        ninja -C build
        cd ../../
    else
        @echo "===== Skipping STM32L476RGT6_FreeRTOS ====="
    fi
    if [ "${SKIP_POSIX_FreeRTOS:-0}" != "1" ]; then
        echo "==== Configuring POSIX_FreeRTOS ===="
        cd examples/POSIX_FreeRTOS
        rm -rf build
        mkdir build
        cmake -G Ninja -B build .
        echo "==== Building POSIX_FreeRTOS ===="
        ninja -C build
        cd ../../
    else
        echo "==== Skipping POSIX_FreeRTOS ===="
    fi

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

# Serve documentation from dev server.
serve_docs:
    cd docs && mdbook serve

# Build documentation
build_docs:
    cd docs && mdbook build

# Build WASM bindings.
build_wasm:
    cd web/tband-wasm && wasm-pack build

# Generate web demo traces. Requires FreeRTOS checkout (v11 or newer)
generate_web_demo_traces:
    #!/bin/env bash
    set -e
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

# Setup website environment.
setup_website:
    cd web/website && npm install

# Serve website from dev server.
serve_website:
    cd web/website && npm run dev

# Build website.
build_website:
    cd web/website && npm run build
