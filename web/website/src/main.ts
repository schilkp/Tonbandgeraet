import { createApp } from "vue";
import "../node_modules/uikit/dist/css/uikit.min.css";
import "../node_modules/uikit/dist/js/uikit.min.js";
import App from "./App.vue";
import * as wasm from "tband_wasm";

wasm.setup_log();

const app = createApp(App);
app.config.errorHandler = (err) => {
    console.error("Vue: " + err);
};
app.mount("#app");
