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
QK_QUEUE: QueueKind
QK_COUNTING_SEMAPHORE: QueueKind
QK_BINARY_SEMAPHORE: QueueKind
QK_MUTEX: QueueKind
QK_RECURSIVE_MUTEX: QueueKind

class TraceEvent(_message.Message):
    __slots__ = ("ts_ns", "dropped_evts_cnt", "task_switched_in", "task_to_ready_state", "task_resumed", "task_suspended", "task_delay", "task_blocking_on_queue_peek", "task_blocking_on_queue_send", "task_blocking_on_queue_receive", "task_blocking_on_sb_send", "task_blocking_on_sb_receive", "task_priority_set", "task_priority_inherit", "task_priority_disinherit", "task_created", "task_deleted", "isr_info", "isr_enter", "isr_exit", "queue_create", "queue_info", "queue_send", "queue_receive")
    TS_NS_FIELD_NUMBER: _ClassVar[int]
    DROPPED_EVTS_CNT_FIELD_NUMBER: _ClassVar[int]
    TASK_SWITCHED_IN_FIELD_NUMBER: _ClassVar[int]
    TASK_TO_READY_STATE_FIELD_NUMBER: _ClassVar[int]
    TASK_RESUMED_FIELD_NUMBER: _ClassVar[int]
    TASK_SUSPENDED_FIELD_NUMBER: _ClassVar[int]
    TASK_DELAY_FIELD_NUMBER: _ClassVar[int]
    TASK_BLOCKING_ON_QUEUE_PEEK_FIELD_NUMBER: _ClassVar[int]
    TASK_BLOCKING_ON_QUEUE_SEND_FIELD_NUMBER: _ClassVar[int]
    TASK_BLOCKING_ON_QUEUE_RECEIVE_FIELD_NUMBER: _ClassVar[int]
    TASK_BLOCKING_ON_SB_SEND_FIELD_NUMBER: _ClassVar[int]
    TASK_BLOCKING_ON_SB_RECEIVE_FIELD_NUMBER: _ClassVar[int]
    TASK_PRIORITY_SET_FIELD_NUMBER: _ClassVar[int]
    TASK_PRIORITY_INHERIT_FIELD_NUMBER: _ClassVar[int]
    TASK_PRIORITY_DISINHERIT_FIELD_NUMBER: _ClassVar[int]
    TASK_CREATED_FIELD_NUMBER: _ClassVar[int]
    TASK_DELETED_FIELD_NUMBER: _ClassVar[int]
    ISR_INFO_FIELD_NUMBER: _ClassVar[int]
    ISR_ENTER_FIELD_NUMBER: _ClassVar[int]
    ISR_EXIT_FIELD_NUMBER: _ClassVar[int]
    QUEUE_CREATE_FIELD_NUMBER: _ClassVar[int]
    QUEUE_INFO_FIELD_NUMBER: _ClassVar[int]
    QUEUE_SEND_FIELD_NUMBER: _ClassVar[int]
    QUEUE_RECEIVE_FIELD_NUMBER: _ClassVar[int]
    ts_ns: int
    dropped_evts_cnt: int
    task_switched_in: int
    task_to_ready_state: int
    task_resumed: int
    task_suspended: int
    task_delay: bool
    task_blocking_on_queue_peek: int
    task_blocking_on_queue_send: int
    task_blocking_on_queue_receive: int
    task_blocking_on_sb_send: int
    task_blocking_on_sb_receive: int
    task_priority_set: TaskPriorityEvent
    task_priority_inherit: TaskPriorityEvent
    task_priority_disinherit: TaskPriorityEvent
    task_created: TaskCreatedEvent
    task_deleted: int
    isr_info: ISRInfoEvent
    isr_enter: int
    isr_exit: int
    queue_create: QueueCreatedEvent
    queue_info: QueueInfoEvent
    queue_send: int
    queue_receive: int
    def __init__(self, ts_ns: _Optional[int] = ..., dropped_evts_cnt: _Optional[int] = ..., task_switched_in: _Optional[int] = ..., task_to_ready_state: _Optional[int] = ..., task_resumed: _Optional[int] = ..., task_suspended: _Optional[int] = ..., task_delay: bool = ..., task_blocking_on_queue_peek: _Optional[int] = ..., task_blocking_on_queue_send: _Optional[int] = ..., task_blocking_on_queue_receive: _Optional[int] = ..., task_blocking_on_sb_send: _Optional[int] = ..., task_blocking_on_sb_receive: _Optional[int] = ..., task_priority_set: _Optional[_Union[TaskPriorityEvent, _Mapping]] = ..., task_priority_inherit: _Optional[_Union[TaskPriorityEvent, _Mapping]] = ..., task_priority_disinherit: _Optional[_Union[TaskPriorityEvent, _Mapping]] = ..., task_created: _Optional[_Union[TaskCreatedEvent, _Mapping]] = ..., task_deleted: _Optional[int] = ..., isr_info: _Optional[_Union[ISRInfoEvent, _Mapping]] = ..., isr_enter: _Optional[int] = ..., isr_exit: _Optional[int] = ..., queue_create: _Optional[_Union[QueueCreatedEvent, _Mapping]] = ..., queue_info: _Optional[_Union[QueueInfoEvent, _Mapping]] = ..., queue_send: _Optional[int] = ..., queue_receive: _Optional[int] = ...) -> None: ...

class TaskPriorityEvent(_message.Message):
    __slots__ = ("task_id", "new_priority")
    TASK_ID_FIELD_NUMBER: _ClassVar[int]
    NEW_PRIORITY_FIELD_NUMBER: _ClassVar[int]
    task_id: int
    new_priority: int
    def __init__(self, task_id: _Optional[int] = ..., new_priority: _Optional[int] = ...) -> None: ...

class TaskCreatedEvent(_message.Message):
    __slots__ = ("task_id", "priority", "name")
    TASK_ID_FIELD_NUMBER: _ClassVar[int]
    PRIORITY_FIELD_NUMBER: _ClassVar[int]
    NAME_FIELD_NUMBER: _ClassVar[int]
    task_id: int
    priority: int
    name: str
    def __init__(self, task_id: _Optional[int] = ..., priority: _Optional[int] = ..., name: _Optional[str] = ...) -> None: ...

class ISRInfoEvent(_message.Message):
    __slots__ = ("isr_id", "name")
    ISR_ID_FIELD_NUMBER: _ClassVar[int]
    NAME_FIELD_NUMBER: _ClassVar[int]
    isr_id: int
    name: str
    def __init__(self, isr_id: _Optional[int] = ..., name: _Optional[str] = ...) -> None: ...

class QueueCreatedEvent(_message.Message):
    __slots__ = ("id", "kind", "size")
    ID_FIELD_NUMBER: _ClassVar[int]
    KIND_FIELD_NUMBER: _ClassVar[int]
    SIZE_FIELD_NUMBER: _ClassVar[int]
    id: int
    kind: QueueKind
    size: int
    def __init__(self, id: _Optional[int] = ..., kind: _Optional[_Union[QueueKind, str]] = ..., size: _Optional[int] = ...) -> None: ...

class QueueInfoEvent(_message.Message):
    __slots__ = ("id", "name")
    ID_FIELD_NUMBER: _ClassVar[int]
    NAME_FIELD_NUMBER: _ClassVar[int]
    id: int
    name: str
    def __init__(self, id: _Optional[int] = ..., name: _Optional[str] = ...) -> None: ...
