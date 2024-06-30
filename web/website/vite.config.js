import { defineConfig } from 'vite'
import wasm from "vite-plugin-wasm";
import topLevelAwait from "vite-plugin-top-level-await";
import vue from '@vitejs/plugin-vue'

// https://vitejs.dev/config/
export default defineConfig({
    plugins: [vue(), wasm(), topLevelAwait()],
    base: "https://schilk.co/tonbandgeraet/"
})
