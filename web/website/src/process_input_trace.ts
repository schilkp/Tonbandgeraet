import { TraceDataPiece } from "./state";


export function process_input_trace(content: string | ArrayBuffer, format: string, core_id: number): TraceDataPiece {
    switch (format) {
        default:
        case "hex":
            const inp = <string>content;
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
            return new TraceDataPiece(core_id, Uint8Array.from(match.map((byte) => parseInt(byte, 16))));
        case "base64":
            var binaryString = atob(<string>content);
            var bytes = new Uint8Array(binaryString.length);
            for (var i = 0; i < binaryString.length; i++) {
                bytes[i] = binaryString.charCodeAt(i);
            }
            return new TraceDataPiece(core_id, bytes);
        case "binary":
            return new TraceDataPiece(core_id, new Uint8Array(<ArrayBuffer>content));
    }
}
