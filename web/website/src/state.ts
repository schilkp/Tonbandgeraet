import { TraceMode } from 'tband_wasm';
import { reactive } from 'vue'

export class TraceDataPiece {
    core_id: number;
    data: Uint8Array;

    constructor(core_id: number, data: Uint8Array) {
        this.core_id = core_id;
        this.data = data;
    }
};

export const trace_mode = reactive({
    value: TraceMode.FreeRTOS,
});

export const core_count = reactive({
    value: 1,
});

export const trace_data = reactive<{ pieces: Array<TraceDataPiece> }>({
    pieces: []
});

export const converted_trace = reactive<{ data: Uint8Array | null }>({
    data: null
});
