_default:
    just --list

# === Utils ===

# Run code generator script.
codegen:
    ./codegen/run.bash

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
test_freertos_integration:
    ./tests/freertos_integration/run.bash

# === FreeRTOS checkout ===

# Setup project FreeRTOS checkout.
freertos_clone:
    ./scripts/freertos_clone.bash

# Put project FreeRTOS checkout at latest commit.
freertos_checkout_main:
    ./scripts/freertos_checkout_main.bash

# Put project FreeRTOS checkout at v10.3.1.
freertos_checkout_v10_3_1:
    ./scripts/freertos_checkout_v10_3_1.bash

# Put project FreeRTOS checkout at v11.1.0.
freertos_checkout_v11_1_0:
    ./scripts/freertos_checkout_v11_1_0.bash

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
