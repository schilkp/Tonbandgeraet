<script setup lang="ts">
import { ref } from "vue";

import { core_count } from "../../state.ts";
import { Flash } from "../../flash_ui_element.ts";

// Core count input updating/validation:
const core_count_inp = defineModel();
core_count_inp.value = core_count.value;

// Flash on error:
const core_count_inp_err = ref<string>("");
const core_count_inp_flash = new Flash("", core_count_inp_err);

function core_count_update(e: any) {
  const val_int = parseInt(e.currentTarget.value, 10);

  core_count_inp_flash.reset();

  if (isNaN(val_int) || val_int === 0 || val_int > 32) {
    console.warn("Invalid core num");
    if (val_int === 0) {
      core_count.value = 1;
    } else if (val_int > 32) {
      core_count.value = 32;
    }
    core_count_inp_flash.flash("uk-form-danger", 1000);
  } else {
    console.info("New core num: " + val_int);
    core_count.value = val_int;
  }
  core_count_inp.value = core_count.value;
}
</script>

<template>
  <label class="uk-form-label">Core Count:</label>
  <input
    class="uk-input"
    :class="core_count_inp_err"
    type="text"
    placeholder="Core Count"
    aria-label="Core Count"
    v-model="core_count_inp"
    @change="core_count_update"
  />
</template>
