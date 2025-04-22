<script setup lang="ts">
import { TraceMode } from "tband_wasm";
import { ref } from "vue";
import { Flash } from "../../flash_ui_element.ts";
import {
    core_count,
    trace_data,
    trace_mode,
    TraceDataPiece,
} from "../../state";
import { process_hex_trace } from "../../process_input_trace.ts";
import UIKit from "uikit";

const demos = __DEMO_TRACES__;

const demo_input = defineModel<string>();
demo_input.value = "";
const demo_flash_class = ref<string>("");
const demo_flash = new Flash("", demo_flash_class);

async function fetch_and_load_demo(path: string) {
    const demo_fetch = await fetch(__TBAND_BASE__ + path);
    if (!demo_fetch.ok) {
        throw "Failed to fetch demo.";
    }
    try {
        let demo = await demo_fetch.json();
        switch (demo.trace_mode) {
            case "freertos":
                trace_mode.value = TraceMode.FreeRTOS;
                break;
            case "baremetal":
                trace_mode.value = TraceMode.FreeRTOS;
                break;
            default:
                throw "Invalid trace mode '" + demo.trace_mode + "'";
        }

        let demo_trace_pieces: TraceDataPiece[] = [];
        let demo_core_count = 1;
        for (let piece of demo.data) {
            demo_trace_pieces.push(process_hex_trace(piece.hex, piece.core_id));
            demo_core_count = Math.max(demo_core_count, piece.core_id + 1);
        }

        core_count.value = demo_core_count;
        trace_data.pieces = demo_trace_pieces;
    } catch (e) {
        console.error(e);
        throw "Invalid demo format.";
    }
}

function demo_update() {
    if (demo_input.value === null || demo_input.value === undefined) {
        return;
    }

    if (demo_input.value != "") {
        // Load demo trace:
        const demo_info = demos[demo_input.value];
        console.log("Loading demo trace '" + demo_info.title + "'");
        fetch_and_load_demo(demo_info.path)
            .then(() => {
                UIKit.notification(
                    'Loaded demo.<br>Click "Convert" and "Open" to view.',
                    { status: "primary", pos: "bottom-right" },
                );
                demo_flash.reset();
                demo_flash.flash("uk-form-success", 300);
                setTimeout(() => {
                    demo_input.value = "";
                }, 600);
            })
            .catch((e) => {
                console.error("Loading demo failed: " + e);
                demo_flash.reset();
                demo_flash.flash("uk-form-danger", 300);
                setTimeout(() => {
                    demo_input.value = "";
                }, 600);
            });
    }
}
</script>

<template>
    <label class="uk-form-label">Open Demo:</label>
    <select
        class="uk-select"
        :class="demo_flash_class"
        v-model="demo_input"
        @change="demo_update"
    >
        <option value="">Select a demo...</option>
        <option v-for="demo in Object.keys(demos)" :value="demo">
            {{ demos[demo].title }}
        </option>
    </select>
</template>
