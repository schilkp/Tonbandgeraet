import json
import sys
from dataclasses import dataclass
from typing import List, Literal, Optional, OrderedDict, Tuple

from protos.FreeRTOS_trace_pb2 import QueueKind, TraceEvent


def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


# ==== Event Stream Decoding ===================================================


@dataclass
class InvalidEvent:
    ts: int


def decode_trace(binary_dump: bytes) -> List[TraceEvent | InvalidEvent]:

    binary_frames = []

    # Undo COBS framing:
    for binary_frame in binary_dump.split(b'\0'):
        binary_frame = undo_cobs(binary_frame)
        if binary_frame is not None:
            binary_frames.append(binary_frame)

    frames = []
    last_ts = None  # type: Optional[int]
    for binary_frame in binary_frames:
        try:
            frame = TraceEvent.FromString(binary_frame)
            frames.append(frame)
            last_ts = frame.ts_ns

            if frame.WhichOneof('event') is None:
                hex_str = []
                for byte in binary_frame:
                    hex_str.append(hex(byte))
                eprint(f"Warnign: Empty trace event {" ".join(hex_str)}")

        except:
            eprint("Warning: Event decode error. Dropping.")
            frames.append(InvalidEvent(last_ts or 0))

    return frames


def undo_cobs(frame: bytes) -> Optional[bytearray]:
    if len(frame) <= 1:
        eprint("Info: Empty frame. Dropping")
        return None

    new_frame = bytearray(frame)
    valid_idx = range(len(frame))

    at = new_frame[0]
    while at in valid_idx:
        new_at = at + new_frame[at]
        new_frame[at] = 0
        at = new_at

    new_frame.pop(0)

    return new_frame

# ==== Event Stream Interpretation =============================================


type Timestamp = int

type TaskStateName = Literal['running',
                             'ready', 'blocked', 'suspended', 'deleted']


@dataclass
class TaskStateEvt:
    ts: Timestamp
    name: TaskStateName
    note: Optional[str]

    def __str__(self):
        if self.note is None:
            return self.name
        else:
            return f"{self.name} ({self.note})"


class Task:
    id: int
    name: Optional[str]
    state_evts: List[TaskStateEvt]
    priority_evts: List[Tuple[Timestamp, int]]

    _state_when_switched_out: TaskStateEvt

    def __init__(self, id: int):
        self.id = id
        self.name = None
        self.state_evts = []
        self.priority_evts = []
        self.state_when_switched_out = 'ready'

    def is_running(self) -> bool:
        if len(self.state_evts) == 0:
            return False
        else:
            state = self.state_evts[-1]
            return state.name == 'running'


type IsrEvtName = Literal['enter', 'exit']


class ISR:
    id: int
    name: Optional[str]
    state_evts: List[Tuple[Timestamp, IsrEvtName]]

    def __init__(self, isr_id: int):
        self.id = isr_id
        self.name = None
        self.state_evts = []

    def is_active(self) -> bool:
        if len(self.state_evts) == 0:
            return False
        else:
            (_, evt) = self.state_evts[-1]
            return evt == 'enter'


type QueueKindName = Literal['queue', 'counting semaphore',
                             'binary semaphore', 'mutex', 'recursive mutex']


class Queue:
    id: int
    name: Optional[str]
    kind: Optional[QueueKindName]
    size: Optional[int]
    state_evts: List[Tuple[Timestamp, int]]

    def __init__(self, isr_id: int):
        self.id = isr_id
        self.name = None
        self.state_evts = []
        self.kind = None
        self.size = None

    def set_kind(self, kind):
        if kind == QueueKind.QK_QUEUE:
            self.kind = "queue"
        elif kind == QueueKind.QK_COUNTING_SEMAPHORE:
            self.kind = "counting semaphore"
        elif kind == QueueKind.QK_BINARY_SEMAPHORE:
            self.kind = "binary semaphore"
        elif kind == QueueKind.QK_MUTEX:
            self.kind = "mutex"
        elif kind == QueueKind.QK_RECURSIVE_MUTEX:
            self.kind = "recursive mutex"
        else:
            eprint(f"Warning: Unknown resource kind {kind}")

    def current_state(self) -> int:
        if len(self.state_evts) == 0:
            return 0
        else:
            return self.state_evts[-1][1]

    def __str__(self) -> str:
        return f"{self.name or str(self.id)} ({self.kind or 'unknown'})"


def u32_subtraction(a: int, b: int) -> int:
    if b < a:
        return a - b
    else:
        return a + (2**32) - b


class TraceState:
    tasks: OrderedDict[int, Task]
    isrs: OrderedDict[int, ISR]
    queues: OrderedDict[int, Queue]

    error_evts: List[Tuple[Timestamp, str]]

    _current_task: Optional[int]
    _current_isr: Optional[int]
    _dropped_evts_cnt: int

    def __init__(self):
        self.tasks = OrderedDict()
        self.isrs = OrderedDict()
        self.queues = OrderedDict()
        self.error_evts = []

        self._current_task = None
        self._dropped_evts_cnt = 0
        self._current_isr = None

    def ensure_task_exists(self, task_id: int):
        if task_id not in self.tasks:
            self.tasks[task_id] = Task(task_id)

    def ensure_isr_exists(self, isr_id: int):
        if isr_id not in self.isrs:
            self.isrs[isr_id] = ISR(isr_id)

    def ensure_queue_exists(self, id: int):
        if id not in self.queues:
            self.queues[id] = Queue(id)

    def handle_evt(self, evt: TraceEvent | InvalidEvent):

        if isinstance(evt, TraceEvent):

            ts = evt.ts_ns

            if evt.HasField('dropped_evts_cnt') is not None:
                if self._dropped_evts_cnt != evt.dropped_evts_cnt:
                    dropped_cnt = u32_subtraction(self._dropped_evts_cnt, evt.dropped_evts_cnt)
                    self.error_evts.append((evt.ts_ns, f"Dropped {dropped_cnt} events!"))
                    self._dropped_evts_cnt = evt.dropped_evts_cnt
                    eprint(f"Warning: Dropped {dropped_cnt} events!")

            if evt.HasField("task_switched_in"):
                task_id = evt.task_switched_in

                # Switch-out previous task (if any):
                if self._current_task is not None:
                    prev_task = self.tasks[self._current_task]
                    prev_task._state_when_switched_out.ts = ts
                    prev_task.state_evts.append(
                        prev_task._state_when_switched_out)

                # Swtich-in new task:
                self.ensure_task_exists(task_id)
                self.tasks[task_id].state_evts.append(
                    TaskStateEvt(ts, 'running', note=None)
                )
                self.tasks[task_id]._state_when_switched_out = TaskStateEvt(
                    0, "ready", note=None)
                self._current_task = task_id

            if evt.HasField("task_to_ready_state"):
                task_id = evt.task_to_ready_state
                self.ensure_task_exists(task_id)
                self.tasks[task_id].state_evts.append(
                    TaskStateEvt(ts, 'ready', note=None))

            if evt.HasField("task_resumed"):
                task_id = evt.task_resumed
                self.ensure_task_exists(task_id)
                self.tasks[task_id].state_evts.append(
                    TaskStateEvt(ts, 'ready', note=None))

            if evt.HasField("task_suspended"):
                task_id = evt.task_suspended
                self.ensure_task_exists(task_id)

                if self._current_task:
                    note = f"by task #t{self._current_task}"
                else:
                    note = None

                if self.tasks[task_id].is_running():
                    self.tasks[task_id]._state_when_switched_out = TaskStateEvt(
                        0, 'suspended', note)
                else:
                    self.tasks[task_id].state_evts.append(
                        TaskStateEvt(ts, 'suspended', note))

            if evt.HasField("task_delay"):
                if self._current_task is not None:
                    self.tasks[self._current_task]._state_when_switched_out = TaskStateEvt(
                        0, 'blocked', 'delay')
                else:
                    self.error_evts.append(
                        (ts, "Current task event with no current task"))

            if evt.HasField("task_blocking_on_queue_peek"):
                resource_id = evt.task_blocking_on_queue_peek

                if self._current_task is not None:
                    self.tasks[self._current_task]._state_when_switched_out = TaskStateEvt(
                        0, 'blocked', f'peek from #q{resource_id} (#q_kind{resource_id})')
                else:
                    self.error_evts.append(
                        (ts, "Current task event with no current task"))

            if evt.HasField("task_blocking_on_queue_send"):
                resource_id = evt.task_blocking_on_queue_send
                if self._current_task is not None:
                    self.tasks[self._current_task]._state_when_switched_out = TaskStateEvt(
                        0, 'blocked', f'send to #q{resource_id} (#q_kind{resource_id})')
                else:
                    self.error_evts.append(
                        (ts, "Current task event with no current task"))

            if evt.HasField("task_blocking_on_queue_receive"):
                resource_id = evt.task_blocking_on_queue_receive
                if self._current_task is not None:
                    self.tasks[self._current_task]._state_when_switched_out = TaskStateEvt(
                        0, 'blocked', f'receive from #q{resource_id} (#q_kind{resource_id})')
                else:
                    self.error_evts.append(
                        (ts, "Current task event with no current task"))

            if evt.HasField("task_blocking_on_sb_send"):
                sb_id = evt.task_blocking_on_sb_send
                if self._current_task is not None:
                    self.tasks[self._current_task]._state_when_switched_out = TaskStateEvt(
                        0, 'blocked', f'sb send #sb{sb_id}')
                else:
                    self.error_evts.append(
                        (ts, "Current task event with no current task"))

            if evt.HasField("task_blocking_on_sb_receive"):
                sb_id = evt.task_blocking_on_sb_receive
                if self._current_task is not None:
                    self.tasks[self._current_task]._state_when_switched_out = TaskStateEvt(
                        0, 'blocked', f'sb receive #sb{sb_id}')
                else:
                    self.error_evts.append(
                        (ts, "Current task event with no current task"))

            if evt.HasField("task_priority_set"):
                task_id = evt.task_priority_set.task_id
                p = evt.task_priority_set.new_priority
                self.ensure_task_exists(task_id)
                self.tasks[task_id].priority_evts.append((ts, p))

            if evt.HasField("task_priority_inherit"):
                task_id = evt.task_priority_inherit.task_id
                p = evt.task_priority_inherit.new_priority
                self.ensure_task_exists(task_id)
                self.tasks[task_id].priority_evts.append((ts, p))

            if evt.HasField("task_priority_disinherit"):
                task_id = evt.task_priority_disinherit.task_id
                p = evt.task_priority_disinherit.new_priority
                self.ensure_task_exists(task_id)
                self.tasks[task_id].priority_evts.append((ts, p))

            if evt.HasField("task_created"):
                task_id = evt.task_created.task_id
                task_name = evt.task_created.name
                task_priority = evt.task_created.priority
                self.ensure_task_exists(task_id)
                self.tasks[task_id].name = task_name
                self.tasks[task_id].priority_evts.append((ts, task_priority))

            if evt.HasField("task_deleted"):
                task_id = evt.task_deleted
                self.ensure_task_exists(task_id)

                if self._current_task:
                    note = f"by task #t{self._current_task}"
                else:
                    note = None

                if self.tasks[task_id].is_running():
                    self.tasks[task_id]._state_when_switched_out = TaskStateEvt(
                        0, "deleted", note)
                else:
                    self.tasks[task_id].state_evts.append(
                        TaskStateEvt(ts, 'deleted', note=note))

            if evt.HasField("isr_enter"):
                isr_id = evt.isr_enter
                self.ensure_isr_exists(isr_id)
                self.isrs[isr_id].state_evts.append((ts, 'enter'))

            if evt.HasField("isr_exit"):
                isr_id = evt.isr_exit
                if isr_id in self.isrs and self.isrs[isr_id].is_active():
                    self.isrs[isr_id].state_evts.append((ts, 'exit'))

            if evt.HasField("isr_info"):
                isr_id = evt.isr_info.isr_id
                isr_name = evt.isr_info.name
                self.ensure_isr_exists(isr_id)
                self.isrs[isr_id].name = isr_name

            if evt.HasField("queue_create"):
                queue_id = evt.queue_create
                self.ensure_queue_exists(queue_id.id)
                self.queues[queue_id.id].set_kind(queue_id.kind)
                self.queues[queue_id.id].size = queue_id.size

            if evt.HasField("queue_info"):
                info = evt.queue_info
                self.ensure_queue_exists(info.id)
                self.queues[info.id].name = info.name

            if evt.HasField("queue_send"):
                queue_id = evt.queue_send
                self.ensure_queue_exists(queue_id)
                new_state = self.queues[queue_id].current_state() + 1
                self.queues[queue_id].state_evts.append((ts, new_state))

            if evt.HasField("queue_receive"):
                queue_id = evt.queue_receive
                self.ensure_queue_exists(queue_id)
                new_state = self.queues[queue_id].current_state() - 1
                self.queues[queue_id].state_evts.append((ts, new_state))

        else:
            self.error_evts.append((evt.ts, "Invalid Event"))

    def replace_id_markers(self, s: str) -> str:
        for task_id, task in self.tasks.items():
            if task.name is not None:
                s = s.replace(f"#t{task_id}", task.name)
            else:
                s = s.replace(f"#t{task_id}", str(task_id))

        for rs_id, queue in self.queues.items():
            if queue.name is not None:
                s = s.replace(f"#q{rs_id}", queue.name)
            else:
                s = s.replace(f"#q{rs_id}", str(rs_id))

            if queue.kind is not None:
                s = s.replace(f"#q_kind{rs_id}", queue.kind)
            else:
                s = s.replace(f"#q_kind{rs_id}", "unknown queue type")

        return s

# ==== Output Generation =======================================================


def genevt_global_instant(name: str, ts: int) -> str:
    v = {
        "ph": "i",  # Instant event
        "ts": ts,
        "name": name,
        "s": "g",  # Scope: Global
    }
    return json.dumps(v)


def genevt_duration_begin(name: str, ts: int, task_id: int, pid: int, cat=None) -> str:
    v = {
        "ph": "B",  # Begin
        "pid": pid,
        "tid": task_id,
        "ts": ts,
        "name": name,
    }
    if cat is not None:
        v["cat"] = cat

    return json.dumps(v)


def genevt_duration_end(ts: int, task_id: int, pid: int) -> str:
    v = {
        "ph": "E",
        "pid": pid,
        "tid": task_id,
        "ts": ts,
    }
    return json.dumps(v)


def genevt_metadata_task_name(name: str, task_id: int, pid: int) -> str:
    v = {
        "ph": "M",  # Metadata
        "pid": pid,
        "tid": task_id,
        "ts": 0,
        "name": "thread_name",
        "args": {
            "name": name
        }
    }
    return json.dumps(v)


def genevt_metadata_pid_name(name: str, pid: int) -> str:
    v = {
        "ph": "M",  # Metadata
        "pid": pid,
        "ts": 0,
        "name": "process_name",
        "args": {
            "name": name
        }
    }
    return json.dumps(v)


def genevt_counter(name: str, property: str, task_id: int, pid: int, ts: int,  value) -> str:
    v = {
        "ph": "C",  # Counter
        "pid": pid,
        "tid": task_id,
        "ts": ts,
        "name": name,
        "args": {
            property: value
        }
    }
    return json.dumps(v)


def generate_json(state: TraceState, evts: List[TraceEvent | InvalidEvent]) -> str:
    trace_logs = []

    first_ts = None
    last_ts = 0
    # Include raw traces:
    for evt in evts:
        if isinstance(evt, InvalidEvent):
            continue

        evt_str = f"{evt.WhichOneof('event')}: {evt}"
        trace_logs.append(genevt_global_instant(evt_str, evt.ts_ns))

        if first_ts is None:
            first_ts = evt.ts_ns
        last_ts = evt.ts_ns

    if first_ts is None:
        return f"{{ [] }}"

    # Include errors:
    for error in state.error_evts:
        trace_logs.append(genevt_global_instant(error[1], error[0]))

    # PID 1: Overview
    pid = 1
    trace_logs.append(genevt_metadata_pid_name(
        "OVERVIEW: CURRENTLY RUNNING", pid))
    for task_id, task in state.tasks.items():
        # Name:

        if task.name is not None:
            name = task.name
        else:
            name = f"Task {task_id}"

        trace_logs.append(genevt_metadata_task_name(name, task_id, pid))

        # Running:
        is_running = False
        for new_state in task.state_evts:
            if new_state.name == "running" and not is_running:
                trace_logs.append(genevt_duration_begin(
                    new_state.name, new_state.ts, task_id, pid, new_state.name))
                is_running = True
            else:
                if is_running:
                    trace_logs.append(genevt_duration_end(
                        new_state.ts, task_id, pid))
                    is_running = False

    # PID 2: Task State
    pid = 2
    trace_logs.append(genevt_metadata_pid_name("TASK STATES", pid))
    for task_id, task in state.tasks.items():
        # Name:
        if task.name is not None:
            name = f"{task.name} State"
        else:
            name = f"Task {task_id} State"

        trace_logs.append(genevt_metadata_task_name(name, task_id, pid))

        # State:
        for idx, new_state in enumerate(task.state_evts):
            new_state_str = state.replace_id_markers(str(new_state))

            if idx != 0:
                trace_logs.append(genevt_duration_end(
                    new_state.ts, task_id, pid))
            trace_logs.append(genevt_duration_begin(
                new_state_str, new_state.ts, task_id, pid, cat=new_state.name))

    # PID 3: Task Priority
    pid = 3
    trace_logs.append(genevt_metadata_pid_name("TASK PRIORITIES", pid))
    for task_id, task in state.tasks.items():
        # Name:
        if task.name is not None:
            name = f"{task.name}"
        else:
            name = f"Task #{task_id}"

        trace_logs.append(genevt_metadata_task_name(name, task_id, pid))

        # Priority:
        for (ts, new_priority) in task.priority_evts:
            trace_logs.append(genevt_counter(
                name, "priority", task_id, pid, ts, new_priority))

        # Add final value (Prevents priorities disappearing when zooming in):
        if len(task.priority_evts) > 0:
            last_priority = task.priority_evts[-1][1]
            trace_logs.append(genevt_counter(
                name, "priority", task_id, pid, last_ts, last_priority))

    # PID 4: Interrupts
    pid = 4
    trace_logs.append(genevt_metadata_pid_name("INTERRUPTS", pid))
    for isr_id, isr in state.isrs.items():
        # Name:
        if isr.name is not None:
            name = f"{isr.name}"
        else:
            name = f"ISR #{isr_id}"

        trace_logs.append(genevt_metadata_task_name(name, isr_id, pid))

        # State:
        for (ts, evt) in isr.state_evts:
            if evt == "enter":
                trace_logs.append(genevt_duration_begin(
                    name, ts, isr_id, pid, cat=name))
            else:
                trace_logs.append(genevt_duration_end(ts, isr_id, pid))

    # PID 5: Queues/Semaphores/Mutex
    pid = 5
    trace_logs.append(genevt_metadata_pid_name("QUEUES/SEMAPHORES/MUTEX", pid))
    for q_id, q in state.queues.items():
        # Name:
        name = str(q)
        trace_logs.append(genevt_metadata_task_name(name, q_id, pid))

        if q.kind == "mutex" or q.kind == "recursive mutex":
            for idx, (ts, state_evt) in enumerate(q.state_evts):
                state_str = "locked" if state_evt == 0 else "unlocked"
                if idx != 0:
                    trace_logs.append(genevt_duration_end(ts, q_id, pid))
                trace_logs.append(genevt_duration_begin(
                    state_str, ts, q_id, pid, cat="mutex"))
        else:
            for idx, (ts, state_evt) in enumerate(q.state_evts):
                trace_logs.append(genevt_counter(
                    name, "state", q_id, pid, ts, state_evt))

            # Add final value (Prevents priorities disappearing when zooming in):
            if len(q.state_evts) > 0:
                last_state = q.state_evts[-1][1]
                trace_logs.append(genevt_counter(
                    name, "state", q_id, pid, last_ts, last_state))

    return f"{{ \"traceEvents\": [\n{',\n'.join(trace_logs)}\n] }}"


# ==== Main File ===============================================================

def main():
    file = "trace.hex"

    with open(file, 'r') as infile:
        hex_dump = infile.read()

    try:
        binary_dump = bytes.fromhex(hex_dump)
    except Exception as e:
        eprint("Invalid hex dump")
        eprint(e)
        return

    decoded_events = decode_trace(binary_dump)

    # Interpret trace:
    state = TraceState()
    for evt in decoded_events:
        state.handle_evt(evt)

    print(generate_json(state, decoded_events))


if __name__ == '__main__':
    main()
