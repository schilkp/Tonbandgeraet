export function ui_error(msg: string) {
    const details = { msg: msg, lvl: "Error" };
    const evt = new CustomEvent("frtrace_log", { detail: details });
    document.dispatchEvent(evt);
}

export function ui_warn(msg: string) {
    const details = { msg: msg, lvl: "Warn" };
    const evt = new CustomEvent("frtrace_log", { detail: details });
    document.dispatchEvent(evt);
}

export function ui_success(msg: string) {
    const details = { msg: msg, lvl: "Success" };
    const evt = new CustomEvent("frtrace_log", { detail: details });
    document.dispatchEvent(evt);
}

export function ui_info(msg: string) {
    const details = { msg: msg, lvl: "Info" };
    const evt = new CustomEvent("frtrace_log", { detail: details });
    document.dispatchEvent(evt);
}

export function ui_debug(msg: string) {
    const details = { msg: msg, lvl: "Debug" };
    const evt = new CustomEvent("frtrace_log", { detail: details });
    document.dispatchEvent(evt);
}
