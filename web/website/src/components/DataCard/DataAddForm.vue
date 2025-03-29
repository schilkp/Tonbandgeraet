<script setup lang="ts">
import { computed, ref } from "vue";

import {
    converted_trace,
    core_count,
    trace_data,
    trace_mode,
} from "../../state";
import UploadArea from "./UploadArea.vue";
import { ui_error, ui_info, ui_success } from "../../log";
import { Flash } from "../../flash_ui_element";
import { process_input_trace } from "../../process_input_trace";
import { convert_trace } from "../../convert";
import { open_trace } from "../../open";

const upload_area = ref<null | typeof UploadArea>(null);

const paste_text = defineModel<string>("paste_text");
const format = defineModel<string>("format");
format.value = "hex";
const core_id = defineModel<string>();
core_id.value = "0";

const validCoreIds = computed(() => {
    var list = [];
    var limit = core_count.value;
    for (var i = 0; i < limit; i++) {
        list.push(i);
    }
    return list;
});

const add_trace_state = ref<string>("");
const add_trace_flash = new Flash("", add_trace_state);
const one_click_add_state = ref<string>("");
const one_click_add_flash = new Flash("", one_click_add_state);
const one_click_add_content = ref<string>("1-CLICK OPEN");

function add_trace(
    content: string | ArrayBuffer,
    format: string,
    core_id: number,
    flash: Flash,
    do_convert_open: boolean,
) {
    try {
        trace_data.pieces.push(process_input_trace(content, format, core_id));
        clear();
        ui_success("Added trace.");
        if (do_convert_open) {
            ui_info("Starting Conversion..");
            one_click_add_content.value = '<div uk-spinner="ratio: 0.8"></div>';
            // Short timeout to allow render of spinner:
            setTimeout(() => {
                try {
                    converted_trace.data = convert_trace(
                        core_count.value,
                        trace_data.pieces,
                        trace_mode.value,
                    );
                    flash.reset();
                    ui_success("Done!");
                    trace_data.pieces = [];
                    open_trace(converted_trace.data);
                } catch (err) {
                    converted_trace.data = null;
                    flash.flash("uk-button-danger", 500);
                    ui_error("Conversion Failed: " + err);
                }
                one_click_add_content.value = "1-CLICK OPEN";
            }, 100);
        }
    } catch (e) {
        ui_error("Failed to add trace (" + e + ").");
        flash.flash("uk-button-danger", 500);
    }
}

function clear() {
    if (upload_area.value !== null) {
        upload_area.value.selected_file = null;
    }
    paste_text.value = "";
}

function one_click_add_btn() {
    import_trace(true, one_click_add_flash);
}

function add_trace_btn() {
    import_trace(false, add_trace_flash);
}

function import_trace(do_convert_open: boolean, flash: Flash) {
    var selected_file = null;
    if (upload_area.value !== null) {
        selected_file = <File>upload_area.value.selected_file;
    }

    const selected_format = format.value || "hex";
    const selected_core_id = parseInt(core_id.value || "0");

    if (selected_file !== null) {
        if (selected_format === "binary") {
            selected_file.arrayBuffer().then(
                (content) =>
                    add_trace(
                        content,
                        "binary",
                        selected_core_id,
                        flash,
                        do_convert_open,
                    ),
                (err) => {
                    ui_error("Failed to open file (" + err + ").");
                    flash.flash("uk-button-danger", 500);
                },
            );
        } else {
            selected_file.text().then(
                (content) =>
                    add_trace(
                        content,
                        selected_format,
                        selected_core_id,
                        flash,
                        do_convert_open,
                    ),
                (err) => {
                    ui_error("Failed to open file (" + err + ").");
                    flash.flash("uk-button-danger", 500);
                },
            );
        }
    } else if (paste_text.value !== undefined) {
        if (selected_format != "binary") {
            add_trace(
                paste_text.value,
                selected_format,
                selected_core_id,
                flash,
                do_convert_open,
            );
        } else {
            ui_error("Binary traces can only be added via upload.");
            flash.flash("uk-button-danger", 500);
        }
    } else {
        ui_error("Invalid trace.");
        flash.flash("uk-button-danger", 500);
    }
}

function is_empty(i: string | undefined): boolean {
    if (i === undefined) {
        return true;
    } else {
        return i.length === 0;
    }
}
</script>

<template>
    <div class="uk-grid-small uk-margin" uk-grid>
        <div class="uk-width-1-2@s">
            <textarea
                class="uk-textarea uk-height-small"
                rows="5"
                placeholder="Paste trace data"
                :disabled="upload_area?.selected_file"
                v-model="paste_text"
            ></textarea>
        </div>

        <div class="uk-width-1-2@s">
            <UploadArea ref="upload_area" />
        </div>

        <div class="uk-width-1-4@s">
            <select class="uk-select" aria-label="Select" v-model="format">
                <option>hex</option>
                <option>binary</option>
                <option>base64</option>
            </select>
        </div>

        <div class="uk-width-1-4@s">
            <select class="uk-select" aria-label="Select" v-model="core_id">
                <option v-for="core_id in validCoreIds" :value="core_id">
                    core #{{ core_id }}
                </option>
            </select>
        </div>

        <div class="uk-width-1-4@s">
            <button
                class="uk-button uk-button-default uk-align-right uk-width-1-1"
                @click="one_click_add_btn"
                :class="one_click_add_state"
                :disabled="!upload_area?.selected_file && is_empty(paste_text)"
            >
                1-Click Open
            </button>
        </div>
        <div class="uk-width-1-4@s">
            <button
                class="uk-button uk-button-default uk-align-right uk-width-1-1"
                @click="add_trace_btn"
                :class="add_trace_state"
                :disabled="!upload_area?.selected_file && is_empty(paste_text)"
            >
                Add
            </button>
        </div>
    </div>
</template>
