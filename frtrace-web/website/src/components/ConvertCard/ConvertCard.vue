<script setup lang="ts">
import { ref } from "vue";
import LogViewer from "./LogViewer.vue";
import { Flash } from "../../flash_ui_element";
import { ui_error, ui_info, ui_success } from "../../log";

import { convert_trace } from "../../convert";
import { open_trace } from "../../open";
import { converted_trace, core_count, trace_data } from "../../state";
import { download_trace } from "../../download";

function reset() {
  const table = <HTMLTableElement>document.getElementById("logs");
  table.innerHTML = "";
  converted_trace.data = null;
  trace_data.pieces = [];
  ui_info("Reset.");
}

const convert_class = ref<string>("");
const convert_flash = new Flash("", convert_class);
function convert() {
  ui_info("Starting Conversion");
  try {
    converted_trace.data = convert_trace(core_count.value, trace_data.pieces);
    convert_flash.reset();
    ui_success("Done!");
  } catch (err) {
    converted_trace.data = null;
    convert_flash.flash("uk-button-danger", 500);
    ui_error("Conversion Failed: " + err);
  }
}

function open() {
  if (converted_trace.data != null) {
    open_trace(converted_trace.data);
  }
}

function download() {
  if (converted_trace.data != null) {
    download_trace(converted_trace.data, "trace.proto");
  }
}
</script>

<template>
  <div
    class="uk-card uk-card-default uk-card-body uk-margin-left uk-margin-right uk-margin-top uk-margin-bottom"
  >
    <legend class="uk-legend">Convert</legend>

    <div class="uk-grid-small uk-margin" uk-grid>
      <div class="uk-width-1-4@s">
        <button
          class="uk-button uk-button-default uk-align-right uk-width-1-1"
          id="btn_conv"
          :class="convert_class"
          :disabled="trace_data.pieces.length === 0"
          @click="convert"
        >
          Convert
        </button>
      </div>
      <div class="uk-width-1-4@s">
        <button
          class="uk-button uk-button-default uk-align-right uk-width-1-1"
          :disabled="converted_trace.data === null"
          @click="download"
        >
          Download
        </button>
      </div>
      <div class="uk-width-1-4@s">
        <button
          class="uk-button uk-button-default uk-align-right uk-width-1-1"
          id="btn_conv_and_open"
          :disabled="converted_trace.data === null"
          @click="open"
        >
          Open
        </button>
      </div>
      <div class="uk-width-1-4@s">
        <button
          class="uk-button uk-button-default uk-align-right uk-width-1-1"
          id="btn_reset"
          @click="reset"
        >
          Reset
        </button>
      </div>
    </div>
    <LogViewer />
  </div>
</template>
