import { defineConfig } from 'vite'
import wasm from "vite-plugin-wasm";
import topLevelAwait from "vite-plugin-top-level-await";
import vue from '@vitejs/plugin-vue'
import { viteStaticCopy } from 'vite-plugin-static-copy'

const dev_port = 5173;

var base;
if (process.env.NODE_ENV === "development") {
    base = "http://localhost:" + dev_port + "/";
} else {
    base = process.env.TBAND_BASE || "https://schilk.co/Tonbandgeraet/";
}

// https://vitejs.dev/config/
export default defineConfig({
    base: base,

    define: {
        __TBAND_BASE__: JSON.stringify(base),
    },

    plugins: [
        vue(),
        wasm(),
        topLevelAwait(),
        viteStaticCopy({
            targets: [
                {
                    src: '../../docs/book/*',
                    dest: 'docs'
                }
            ]
        })
    ],

    server: {
        port: dev_port,
        strictPort: true, // strict mode so base defined above is always correct.
    }
})
