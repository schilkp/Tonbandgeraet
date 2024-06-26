<script setup lang="ts">
import { computed, ref } from "vue";

import { core_count, trace_data } from "../../state";
import UploadArea from "./UploadArea.vue";
import { ui_error, ui_success } from "../../log";
import { Flash } from "../../flash_ui_element";
import { process_input_trace } from "../../process_input_trace";

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

function add_trace(
  content: string | ArrayBuffer,
  format: string,
  core_id: number,
) {
  try {
    trace_data.pieces.push(process_input_trace(content, format, core_id))
    clear();
    ui_success("Added trace.");
  } catch (e) {
    ui_error("Failed to add trace: " + e);
    add_trace_flash.flash("uk-button-danger", 500);
  }
}

function clear() {
  if (upload_area.value !== null) {
    upload_area.value.selected_file = null;
  }
  paste_text.value = "";
}

function add_trace_btn() {
  var selected_file = null;
  if (upload_area.value !== null) {
    selected_file = <File>upload_area.value.selected_file;
  }

  const selected_format = format.value || "hex";
  const selected_core_id = parseInt(core_id.value || "0");

  if (selected_file !== null) {
    if (selected_format === "binary") {
      selected_file.arrayBuffer().then(
        (content) => add_trace(content, "binary", selected_core_id),
        (err) => {
          ui_error("Failed to open file: " + err);
          add_trace_flash.flash("uk-button-danger", 500);
        },
      );
    } else {
      selected_file.text().then(
        (content) => add_trace(content, selected_format, selected_core_id),
        (err) => {
          ui_error("Failed to open file: " + err);
          add_trace_flash.flash("uk-button-danger", 500);
        },
      );
    }
  } else if (paste_text.value !== undefined) {
    if (selected_format != "binary") {
      add_trace(paste_text.value, selected_format, selected_core_id);
    } else {
      ui_error("Binary traces can only be added via upload.");
      add_trace_flash.flash("uk-button-danger", 500);
    }
  } else {
    ui_error("invalid trace");
    add_trace_flash.flash("uk-button-danger", 500);
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
    <div class="uk-width-1-2">
      <textarea
        class="uk-textarea uk-height-small"
        rows="5"
        placeholder="Paste trace data"
        :disabled="upload_area?.selected_file"
        v-model="paste_text"
      ></textarea>
    </div>

    <div class="uk-width-1-2">
      <UploadArea ref="upload_area" />
    </div>

    <div class="uk-width-1-2">
      <label class="uk-form-label">Format</label>
      <select class="uk-select" aria-label="Select" v-model="format">
        <option>hex</option>
        <option>binary</option>
        <option>base64</option>
      </select>
    </div>

    <div class="uk-width-1-2">
      <label class="uk-form-label">Core</label>
      <select class="uk-select" aria-label="Select" v-model="core_id">
        <option v-for="core_id in validCoreIds" :value="core_id">
          core #{{ core_id }}
        </option>
      </select>
    </div>

    <div class="uk-width-1-1">
      <button
        class="uk-button uk-button-default uk-align-right uk-width-small"
        @click="add_trace_btn"
        :class="add_trace_state"
        :disabled="!upload_area?.selected_file && is_empty(paste_text)"
      >
        Add
      </button>
    </div>
  </div>
</template>
