import { Ref } from "vue";

export class Flash {
    timer_id: number | undefined;
    reset_state: string;
    class_ref: Ref<string>;

    constructor(reset_state: string, class_ref: Ref<string>) {
        this.timer_id = undefined;
        this.reset_state = reset_state;
        this.class_ref = class_ref;
    }

    flash(state: string, time_ms: number) {
        if (this.timer_id !== null) {
            clearTimeout(this.timer_id);
        }
        this.class_ref.value = state;
        setTimeout(() => (this.class_ref.value = this.reset_state), time_ms);
    }

    reset() {
        if (this.timer_id !== null) {
            clearTimeout(this.timer_id);
        }
        this.class_ref.value = this.reset_state;
    }
}
