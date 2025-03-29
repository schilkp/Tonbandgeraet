import { ui_info } from "./log";
import { TraceDataPiece } from "./state";

export function process_input_trace(
    content: string | ArrayBuffer,
    format: string,
    core_id: number,
): TraceDataPiece {
    switch (format) {
        default:
        case "hex":
            return process_hex_trace(<string>content, core_id);
        case "base64":
            return process_b64_trace(<string>content, core_id);
        case "binary":
            return new TraceDataPiece(
                core_id,
                new Uint8Array(<ArrayBuffer>content),
            );
    }
}

function remove_markers(content: string): string {
    const markers = [
        "==== START OF TONBANDGERAET BUFFER ====",
        "==== END OF TONBANDGERAET BUFFER ====",
    ];

    for (var marker of markers) {
        if (content.includes(marker)) {
            ui_info("Ignoring marker \'" + marker + "\'");
        }
        content = content.replace(marker, "");
    }

    return content;
}

export function process_hex_trace(
    content: string,
    core_id: number,
): TraceDataPiece {
    let inp = remove_markers(<string>content);

    var hex = "";

    for (var c of inp) {
        if (/\s/.test(c)) {
            continue;
        }
        if (!/[a-fA-F0-9]/.test(c)) {
            throw "Non-hex char '" + c + "'.";
        }
        hex = hex + c;
    }
    if (hex.length === 0) {
        throw "Empty";
    }
    if (hex.length % 2 !== 0) {
        throw "Non-even number of hex characters.";
    }

    let match = hex.match(/.{1,2}/g);
    if (!match) {
        throw "Malformed.";
    }
    return new TraceDataPiece(
        core_id,
        Uint8Array.from(match.map((byte) => parseInt(byte, 16))),
    );
}

function process_b64_trace(content: string, core_id: number): TraceDataPiece {
    var binaryString = atob(remove_markers(<string>content));
    var bytes = new Uint8Array(binaryString.length);
    for (var i = 0; i < binaryString.length; i++) {
        bytes[i] = binaryString.charCodeAt(i);
    }
    return new TraceDataPiece(core_id, bytes);
}
