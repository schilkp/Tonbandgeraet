/// <reference types="vite/client" />
interface DemoTraceInfo {
    title: string;
    path: string;
}

declare const __TBAND_BASE__: string;
declare const __DEMO_TRACES__: Record<string, DemoTraceInfo>;
