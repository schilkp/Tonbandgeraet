<script setup lang="ts">
import CoreCountInput from "./CoreCountInput.vue";
import ModeInput from "./ModeInput.vue";
</script>

<template>
    <div
        class="uk-card uk-card-default uk-card-body uk-margin-left uk-margin-right uk-margin-top uk-margin-bottom"
    >
        <div class="uk-child-width-expands@s uk-grid-match" uk-grid>
            <div class="uk-width-1-1@s">
                <h1><strong>Tonbandgerät</strong></h1>
            </div>
            <div class="uk-width-1-2@s">
                <ModeInput />
            </div>
            <div class="uk-width-1-2@s">
                <CoreCountInput />
            </div>
        </div>
    </div>
</template>
