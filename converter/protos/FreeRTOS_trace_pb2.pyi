from google.protobuf.internal import enum_type_wrapper as _enum_type_wrapper
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from typing import ClassVar as _ClassVar, Mapping as _Mapping, Optional as _Optional, Union as _Union

DESCRIPTOR: _descriptor.FileDescriptor

class QueueKind(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = ()
    QK_QUEUE: _ClassVar[QueueKind]
    QK_COUNTING_SEMAPHORE: _ClassVar[QueueKind]
    QK_BINARY_SEMAPHORE: _ClassVar[QueueKind]
    QK_MUTEX: _ClassVar[QueueKind]
    QK_RECURSIVE_MUTEX: _ClassVar[QueueKind]

class StreamBufferKind(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = ()
    SB_NORMAL: _ClassVar[StreamBufferKind]
    SB_MESSAGE_BUFFER: _ClassVar[StreamBufferKind]
QK_QUEUE: QueueKind
QK_COUNTING_SEMAPHORE: QueueKind
QK_BINARY_SEMAPHORE: QueueKind
QK_MUTEX: QueueKind
QK_RECURSIVE_MUTEX: QueueKind
SB_NORMAL: StreamBufferKind
SB_MESSAGE_BUFFER: StreamBufferKind

class TraceEvent(_message.Message):
    __slots__ = ("ts_ns", "task_switched_in", "task_to_ready_state", "task_resumed", "task_suspended", "isr_name", "isr_enter", "isr_exit", "task_delay", "task_priority_set", "task_priority_inherit", "task_priority_disinherit", "task_create", "task_name", "task_is_idle_task", "task_is_timer_task", "task_deleted", "queue_create", "queue_name", "queue_kind", "queue_send", "queue_overwrite", "queue_receive", "queue_reset", "task_blocking_on_queue_peek", "task_blocking_on_queue_send", "task_blocking_on_queue_receive", "stream_buffer_create", "stream_buffer_name", "stream_buffer_kind", "stream_buffer_receive", "stream_buffer_send", "stream_buffer_reset", "task_blocking_on_sb_send", "task_blocking_on_sb_receive")
    TS_NS_FIELD_NUMBER: _ClassVar[int]
    TASK_SWITCHED_IN_FIELD_NUMBER: _ClassVar[int]
    TASK_TO_READY_STATE_FIELD_NUMBER: _ClassVar[int]
    TASK_RESUMED_FIELD_NUMBER: _ClassVar[int]
    TASK_SUSPENDED_FIELD_NUMBER: _ClassVar[int]
    ISR_NAME_FIELD_NUMBER: _ClassVar[int]
    ISR_ENTER_FIELD_NUMBER: _ClassVar[int]
    ISR_EXIT_FIELD_NUMBER: _ClassVar[int]
    TASK_DELAY_FIELD_NUMBER: _ClassVar[int]
    TASK_PRIORITY_SET_FIELD_NUMBER: _ClassVar[int]
    TASK_PRIORITY_INHERIT_FIELD_NUMBER: _ClassVar[int]
    TASK_PRIORITY_DISINHERIT_FIELD_NUMBER: _ClassVar[int]
    TASK_CREATE_FIELD_NUMBER: _ClassVar[int]
    TASK_NAME_FIELD_NUMBER: _ClassVar[int]
    TASK_IS_IDLE_TASK_FIELD_NUMBER: _ClassVar[int]
    TASK_IS_TIMER_TASK_FIELD_NUMBER: _ClassVar[int]
    TASK_DELETED_FIELD_NUMBER: _ClassVar[int]
    QUEUE_CREATE_FIELD_NUMBER: _ClassVar[int]
    QUEUE_NAME_FIELD_NUMBER: _ClassVar[int]
    QUEUE_KIND_FIELD_NUMBER: _ClassVar[int]
    QUEUE_SEND_FIELD_NUMBER: _ClassVar[int]
    QUEUE_OVERWRITE_FIELD_NUMBER: _ClassVar[int]
    QUEUE_RECEIVE_FIELD_NUMBER: _ClassVar[int]
    QUEUE_RESET_FIELD_NUMBER: _ClassVar[int]
    TASK_BLOCKING_ON_QUEUE_PEEK_FIELD_NUMBER: _ClassVar[int]
    TASK_BLOCKING_ON_QUEUE_SEND_FIELD_NUMBER: _ClassVar[int]
    TASK_BLOCKING_ON_QUEUE_RECEIVE_FIELD_NUMBER: _ClassVar[int]
    STREAM_BUFFER_CREATE_FIELD_NUMBER: _ClassVar[int]
    STREAM_BUFFER_NAME_FIELD_NUMBER: _ClassVar[int]
    STREAM_BUFFER_KIND_FIELD_NUMBER: _ClassVar[int]
    STREAM_BUFFER_RECEIVE_FIELD_NUMBER: _ClassVar[int]
    STREAM_BUFFER_SEND_FIELD_NUMBER: _ClassVar[int]
    STREAM_BUFFER_RESET_FIELD_NUMBER: _ClassVar[int]
    TASK_BLOCKING_ON_SB_SEND_FIELD_NUMBER: _ClassVar[int]
    TASK_BLOCKING_ON_SB_RECEIVE_FIELD_NUMBER: _ClassVar[int]
    ts_ns: int
    task_switched_in: int
    task_to_ready_state: int
    task_resumed: int
    task_suspended: int
    isr_name: NameEvent
    isr_enter: int
    isr_exit: int
    task_delay: bool
    task_priority_set: TaskPriorityEvent
    task_priority_inherit: TaskPriorityEvent
    task_priority_disinherit: TaskPriorityEvent
    task_create: int
    task_name: NameEvent
    task_is_idle_task: int
    task_is_timer_task: int
    task_deleted: int
    queue_create: int
    queue_name: NameEvent
    queue_kind: QueueKindEvent
    queue_send: int
    queue_overwrite: int
    queue_receive: int
    queue_reset: int
    task_blocking_on_queue_peek: int
    task_blocking_on_queue_send: int
    task_blocking_on_queue_receive: int
    stream_buffer_create: int
    stream_buffer_name: NameEvent
    stream_buffer_kind: StreamBufferKindEvent
    stream_buffer_receive: StreamBufferTransferEvent
    stream_buffer_send: StreamBufferTransferEvent
    stream_buffer_reset: int
    task_blocking_on_sb_send: int
    task_blocking_on_sb_receive: int
    def __init__(self, ts_ns: _Optional[int] = ..., task_switched_in: _Optional[int] = ..., task_to_ready_state: _Optional[int] = ..., task_resumed: _Optional[int] = ..., task_suspended: _Optional[int] = ..., isr_name: _Optional[_Union[NameEvent, _Mapping]] = ..., isr_enter: _Optional[int] = ..., isr_exit: _Optional[int] = ..., task_delay: bool = ..., task_priority_set: _Optional[_Union[TaskPriorityEvent, _Mapping]] = ..., task_priority_inherit: _Optional[_Union[TaskPriorityEvent, _Mapping]] = ..., task_priority_disinherit: _Optional[_Union[TaskPriorityEvent, _Mapping]] = ..., task_create: _Optional[int] = ..., task_name: _Optional[_Union[NameEvent, _Mapping]] = ..., task_is_idle_task: _Optional[int] = ..., task_is_timer_task: _Optional[int] = ..., task_deleted: _Optional[int] = ..., queue_create: _Optional[int] = ..., queue_name: _Optional[_Union[NameEvent, _Mapping]] = ..., queue_kind: _Optional[_Union[QueueKindEvent, _Mapping]] = ..., queue_send: _Optional[int] = ..., queue_overwrite: _Optional[int] = ..., queue_receive: _Optional[int] = ..., queue_reset: _Optional[int] = ..., task_blocking_on_queue_peek: _Optional[int] = ..., task_blocking_on_queue_send: _Optional[int] = ..., task_blocking_on_queue_receive: _Optional[int] = ..., stream_buffer_create: _Optional[int] = ..., stream_buffer_name: _Optional[_Union[NameEvent, _Mapping]] = ..., stream_buffer_kind: _Optional[_Union[StreamBufferKindEvent, _Mapping]] = ..., stream_buffer_receive: _Optional[_Union[StreamBufferTransferEvent, _Mapping]] = ..., stream_buffer_send: _Optional[_Union[StreamBufferTransferEvent, _Mapping]] = ..., stream_buffer_reset: _Optional[int] = ..., task_blocking_on_sb_send: _Optional[int] = ..., task_blocking_on_sb_receive: _Optional[int] = ...) -> None: ...

class TaskPriorityEvent(_message.Message):
    __slots__ = ("task_id", "new_priority")
    TASK_ID_FIELD_NUMBER: _ClassVar[int]
    NEW_PRIORITY_FIELD_NUMBER: _ClassVar[int]
    task_id: int
    new_priority: int
    def __init__(self, task_id: _Optional[int] = ..., new_priority: _Optional[int] = ...) -> None: ...

class NameEvent(_message.Message):
    __slots__ = ("id", "name")
    ID_FIELD_NUMBER: _ClassVar[int]
    NAME_FIELD_NUMBER: _ClassVar[int]
    id: int
    name: str
    def __init__(self, id: _Optional[int] = ..., name: _Optional[str] = ...) -> None: ...

class QueueKindEvent(_message.Message):
    __slots__ = ("id", "kind")
    ID_FIELD_NUMBER: _ClassVar[int]
    KIND_FIELD_NUMBER: _ClassVar[int]
    id: int
    kind: QueueKind
    def __init__(self, id: _Optional[int] = ..., kind: _Optional[_Union[QueueKind, str]] = ...) -> None: ...

class StreamBufferKindEvent(_message.Message):
    __slots__ = ("id", "kind")
    ID_FIELD_NUMBER: _ClassVar[int]
    KIND_FIELD_NUMBER: _ClassVar[int]
    id: int
    kind: StreamBufferKind
    def __init__(self, id: _Optional[int] = ..., kind: _Optional[_Union[StreamBufferKind, str]] = ...) -> None: ...

class StreamBufferTransferEvent(_message.Message):
    __slots__ = ("id", "amnt")
    ID_FIELD_NUMBER: _ClassVar[int]
    AMNT_FIELD_NUMBER: _ClassVar[int]
    id: int
    amnt: int
    def __init__(self, id: _Optional[int] = ..., amnt: _Optional[int] = ...) -> None: ...
