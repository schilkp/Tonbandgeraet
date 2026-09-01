_default:
    just --list

# === Utils ===

# Run code generator script.
codegen:
    ./codegen/run.bash

# Format sources
format:
    ./scripts/clang-format.bash
    cd ./tools && cargo fmt
    cd ./web/tband-wasm/ && cargo fmt
    cd ./web/website && npm run format

# === Testing ===

# Run unit tests.
test_unit:
    ./tests/unit_test/run.bash

# Run bare-metal integration tests.
test_baremetal_integration:
    ./tests/baremetal_integration/run.bash

# Build examples (requires a FreeRTOS checkout)
test_build_examples:
    ./tests/build_examples.bash

# Run FreeRTOS integration tests (Requires a FreeRTOS checkout v11 or newer)
test_freertos_integration_posix:
    ./tests/freertos_integration_posix/run.bash

# Run FreeRTOS integration tests against a QEMU-emulated MPS2-AN385 board (requires a FreeRTOS checkout, and qemu-system-arm)
test_freertos_integration_qemu:
    ./tests/freertos_integration_qemu/run.bash

# Run all tests:
test: test_unit test_baremetal_integration test_freertos_integration_posix test_freertos_integration_qemu test_build_examples
    @echo "✅ all tests ok ✅"

# === FreeRTOS checkout ===

# Put project FreeRTOS checkout at latest commit.
freertos_checkout_main:
    ./scripts/freertos_checkout.bash "main"

# Put project FreeRTOS checkout at v10.3.1.
freertos_checkout_v10_3_1:
    ./scripts/freertos_checkout.bash "V10.3.1-kernel-only"

# Put project FreeRTOS checkout at v10.4.0.
freertos_checkout_v10_4_0:
    ./scripts/freertos_checkout.bash "V10.4.0-kernel-only"

# Put project FreeRTOS checkout at v11.1.0.
freertos_checkout_v11_1_0:
    ./scripts/freertos_checkout.bash "V11.1.0"

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
    ./scripts/generate_web_demo_traces.bash

# Setup website environment.
setup_website:
    cd web/website && npm install

# Serve website from dev server.
serve_website:
    cd web/website && npm run dev

# Build website.
build_website:
    cd web/website && npm run build
