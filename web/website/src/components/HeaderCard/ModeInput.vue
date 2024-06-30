<script setup lang="ts">
import { TraceMode } from "tband_wasm";
import { trace_mode } from "../../state.ts";

const trace_mode_input = defineModel();
trace_mode_input.value = "freertos";

function trace_mode_update() {
  switch (trace_mode_input.value) {
    case "freertos":
      trace_mode.value = TraceMode.FreeRTOS;
      break;
    case "baremetal":
      trace_mode.value = TraceMode.BareMetal;
      break;
    default:
      trace_mode_input.value = "freertos";
      trace_mode.value = TraceMode.FreeRTOS;
      console.error("Invalid mode selected");
      break;
  }
}
</script>

<template>
  <label class="uk-form-label">Trace Mode:</label>
  <select
    class="uk-select"
    v-model="trace_mode_input"
    @change="trace_mode_update"
  >
    <option value="freertos">FreeRTOS</option>
    <option value="baremetal">Bare-Metal</option>
  </select>
</template>
