import { TraceDataPiece } from "./state";
import * as wasm from "tband_wasm";

export function convert_trace(core_count: number, data: Array<TraceDataPiece>, mode: wasm.TraceMode): Uint8Array {
    var trace_data: (wasm.TraceData)[] = [];
    for (const piece of data) {
        trace_data.push(wasm.TraceData.new(piece.core_id, piece.data))
    }
    return wasm.convert(core_count, trace_data, mode);
}
