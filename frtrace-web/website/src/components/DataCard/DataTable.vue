<script setup lang="ts">
import { trace_data } from "../../state";

function delete_trace(idx: number) {
  console.log("Delete Trace ", idx);
  trace_data.pieces.splice(idx, 1);
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
