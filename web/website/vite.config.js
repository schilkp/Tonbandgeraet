import { defineConfig } from "vite";
import wasm from "vite-plugin-wasm";
import topLevelAwait from "vite-plugin-top-level-await";
import vue from "@vitejs/plugin-vue";
import { viteStaticCopy } from "vite-plugin-static-copy";
import fs from "fs";
import path from "path";

// Server/Base URL handling:
const dev_port = 5173;
var base;
if (process.env.NODE_ENV === "development") {
    base = "http://localhost:" + dev_port;
} else {
    base = process.env.TBAND_BASE || "https://schilk.co/Tonbandgeraet";
}
base = base.replace(/\/$/, "") + "/";
console.log("URL base: " + base);

// Demo Traces:
// Generate a define listing the available traces, and bundle them as
// static assets.
const demo_traces_dir = path.resolve(__dirname, "demo_traces");
const demo_traces_available = {};
if (fs.existsSync(demo_traces_dir)) {
    const jsonFiles = fs
        .readdirSync(demo_traces_dir)
        .filter((file) => file.endsWith(".json"));

    // Extract metadata and content from json file:
    jsonFiles.forEach((file) => {
        try {
            const filePath = path.join(demo_traces_dir, file);
            const content = fs.readFileSync(filePath, "utf8");
            const data = JSON.parse(content);
            const key = file.replace(".json", "");
            demo_traces_available[key] = {
                title: data.title,
                path: `demo_traces/${file}`,
            };
            console.info(`found demo trace ` + file);
        } catch (error) {
            console.error(`Error processing ${file}:`, error);
            throw error;
        }
    });
}

// https://vitejs.dev/config/
export default defineConfig({
    base: base,

    define: {
        __TBAND_BASE__: JSON.stringify(base),
        __DEMO_TRACES__: JSON.stringify(demo_traces_available),
    },

    plugins: [
        vue(),
        wasm(),
        topLevelAwait(),
        viteStaticCopy({
            targets: [
                {
                    src: "../../docs/book/*",
                    dest: "docs",
                },
                {
                    src: "demo_traces/*.json",
                    dest: "demo_traces",
                },
            ],
        }),
    ],

    server: {
        port: dev_port,
        strictPort: true, // strict mode so base defined above is always correct.
    },
});
