<script setup lang="ts">
import { trace_data } from "../../state";
import DataAddForm from "./DataAddForm.vue";
import DataTable from "./DataTable.vue";
</script>

<template>
  <div
    class="uk-card uk-card-default uk-card-body uk-margin-left uk-margin-right uk-margin-top uk-margin-bottom"
  >
    <legend class="uk-legend">Trace Data</legend>
    <DataAddForm />
    <DataTable v-if="trace_data.pieces.length !== 0" />
  </div>
</template>
