<script setup lang="ts">
import { ref } from "vue";

const selected_file = ref<null | File>(null);

function upload_file_changed(e: Event) {
  const target = <HTMLInputElement>e.currentTarget;
  if (target.files !== null && target.files.length > 0) {
    selected_file.value = target.files[0];
  } else {
    selected_file.value = null;
  }
}

const dragover_active = ref<string>("");
function drag_over(e: Event) {
  e.preventDefault();
  dragover_active.value = "uk-dragover";
}
function drag_enter(e: Event) {
  e.preventDefault();
  dragover_active.value = "uk-dragover";
}
function drag_exit(e: Event) {
  e.preventDefault();
  dragover_active.value = "";
}
function drag_drop(e: DragEvent) {
  e.preventDefault();
  dragover_active.value = "";
  if (e.dataTransfer !== null && e.dataTransfer.files.length > 0) {
    selected_file.value = e.dataTransfer.files[0];
  }
}
function delete_file() {
  selected_file.value = null;
}

defineExpose({ selected_file });
</script>

<template>
  <div
    class="uk-placeholder uk-text-center uk-height-small"
    @dragover="drag_over"
    @dragenter="drag_enter"
    @dragexit="drag_exit"
    @drop="drag_drop"
    :class="dragover_active"
  >
    <div>
      <img
        v-if="selected_file === null"
        src="../../../assets/upload.svg"
        alt="Upload"
        height="35"
        width="35"
      />
      <img
        v-if="selected_file !== null"
        src="../../../assets/file.svg"
        alt="Uploaded File"
        height="35"
        width="35"
      />
    </div>

    <p />

    <div v-if="selected_file === null">
      <span class="uk-text-middle"> Drop trace file here or </span>
      <div uk-form-custom>
        <input type="file" @change="upload_file_changed" single />
        <span class="uk-link">select it</span>.
      </div>
    </div>
    <div v-if="selected_file !== null">
      {{ selected_file.name }}
      <button @click="delete_file" type="button" uk-close></button>
    </div>
  </div>
</template>
