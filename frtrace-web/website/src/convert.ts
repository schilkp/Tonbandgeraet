import { TraceData } from "./state";
import * as wasm from "frtrace_wasm";

export function convert_trace(core_count: number, data: Array<TraceData>, mode: wasm.TraceMode): Uint8Array {
    var trace_data: (wasm.FrTraceData)[] = [];
    for (const piece of data) {
        trace_data.push(wasm.FrTraceData.new(piece.core_id, piece.data))
    }
    return wasm.convert(core_count, trace_data, mode);
}
