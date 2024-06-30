import { defineConfig } from 'vite'
import wasm from "vite-plugin-wasm";
import topLevelAwait from "vite-plugin-top-level-await";
import vue from '@vitejs/plugin-vue'
import { viteStaticCopy } from 'vite-plugin-static-copy'

// https://vitejs.dev/config/
export default defineConfig({
    base: "https://schilk.co/Tonbandgeraet/",
    plugins: [vue(), wasm(), topLevelAwait(),
    viteStaticCopy({
        targets: [
            {
                src: '../../docs/book/*',
                dest: 'docs'
            }
        ]
    })
    ],
})
