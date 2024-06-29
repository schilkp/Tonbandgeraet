<script setup lang="ts">
import { ui_info } from "../../log";
import { trace_data } from "../../state";

function delete_trace(idx: number) {
  trace_data.pieces.splice(idx, 1);
  ui_info("Deleted trace.");
}
</script>

<template>
  <table class="uk-table uk-table-divider uk-table-small">
    <thead>
      <tr>
        <th class="uk-table-shrink">#</th>
        <th class="uk-table-shrink">Core</th>
        <th class="uk-table-expand">Size (bytes)</th>
        <th class="uk-width-small uk-text-center">Delete</th>
      </tr>
    </thead>
    <tbody>
      <tr v-for="[idx, piece] in trace_data.pieces.entries()">
        <td>{{ idx + 1 }}</td>
        <td>#{{ piece.core_id }}</td>
        <td>{{ piece.data.length }}</td>
        <td>
          <button
            type="button"
            class="uk-align-center"
            uk-close
            @click="delete_trace(idx)"
          ></button>
        </td>
      </tr>
    </tbody>
  </table>
</template>
