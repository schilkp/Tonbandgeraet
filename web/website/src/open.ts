import { ui_error, ui_info, ui_success } from "./log";

export function open_trace(data: Uint8Array): boolean {
    const ORIGIN = "https://ui.perfetto.dev";

    const win = window.open(ORIGIN);
    if (!win) {
        ui_error(
            "Popups disabled. Enable or download trace and upload manually to ui.perfetto.dev.",
        );
        return false;
    }

    ui_info("Waiting for perfetto..");

    const timer = setInterval(() => win.postMessage("PING", ORIGIN), 50);

    const onMessageHandler = (evt: MessageEvent) => {
        if (evt.data !== "PONG") return;

        // We got a PONG, the UI is ready.
        window.clearInterval(timer);
        window.removeEventListener("message", onMessageHandler);

        win.postMessage(
            {
                perfetto: {
                    buffer: data,
                    title: "FreeRTOS Trace",
                    url: "-",
                },
            },
            ORIGIN,
        );
        ui_success("Trace opened.");
    };

    window.addEventListener("message", onMessageHandler);

    return true;
}
