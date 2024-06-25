import os
import sys
from os.path import abspath, dirname, join

import gen_c_encoder
import gen_rs_decoder
from model import S64, U32, U64, Evt, Str, U8Enum, U8EnumDefinition

# ==== Trace messages definition ===============================================

QueueKindEnum = U8EnumDefinition("QueueKind",
                                 [(0, "QK_QUEUE"), (1, "QK_COUNTING_SEMPHR"), (2, "QK_BINARY_SEMPHR"),
                                  (3, "QK_MUTEX"), (4, "QK_RECURSIVE_MUTEX"), (5, "QK_QUEUE_SET"),]
                                 )

StreamBufferKindEnum = U8EnumDefinition("StreamBufferKind",
                                        [(0, "SBK_STREAM_BUFFER"),
                                         (1, "SBK_MESSAGE_BUFFER")]
                                        )

ENUMS = [
    QueueKindEnum,
    StreamBufferKindEnum
]

# fmt: off
# Note: Every event starts with a U8 ID. Every non-metadata event then automatically contains a
#       a U64 timestamp as its next field.
EVTS = [
    # Tracing events:
    Evt("core_id",           id=0, fields=[U32("core_id")]),
    Evt("dropped_evt_cnt",   id=1, fields=[U32("cnt")]),
    Evt("ts_resolution_ns",  id=2, fields=[U64("ns_per_ts")], is_metadata=True),

    # Task scheduling events:
    Evt("task_switched_in",      id=3, fields=[U32("task_id")]),
    Evt("task_to_rdy_state",     id=4, fields=[U32("task_id")]),
    Evt("task_resumed",          id=5, fields=[U32("task_id")]),
    Evt("task_resumed_from_isr", id=6, fields=[U32("task_id")]),
    Evt("task_suspended",        id=7, fields=[U32("task_id")]),
    Evt("curtask_delay",         id=8, fields=[U32("ticks")]),
    Evt("curtask_delay_until",   id=9, fields=[U32("time_to_wake")]),

    # Task priority events:
    Evt("task_priority_set",        id=10, fields=[U32("task_id"), U32("priority")]),
    Evt("task_priority_inherit",    id=11, fields=[U32("task_id"), U32("priority")]),
    Evt("task_priority_disinherit", id=12, fields=[U32("task_id"), U32("priority")]),

    # Task management events:
    Evt("task_created",       id=13, fields=[U32("task_id")]),
    Evt("task_name",          id=14, fields=[U32("task_id")], varlen_field=Str("name"), is_metadata=True),
    Evt("task_is_idle_task",  id=15, fields=[U32("task_id"), U32("core_id")], is_metadata=True),
    Evt("task_is_timer_task", id=16, fields=[U32("task_id")], is_metadata=True),
    Evt("task_deleted",       id=17, fields=[U32("task_id")]),

    # ISRs:
    Evt("isr_name",  id=18, fields=[U32("isr_id")], varlen_field=Str("name"), is_metadata=True),
    Evt("isr_enter", id=19, fields=[U32("isr_id")]),
    Evt("isr_exit",  id=20, fields=[U32("isr_id")]),

    # Queues:
    Evt("queue_created",                  id=21, fields=[U32("queue_id")]),
    Evt("queue_name",                     id=22, fields=[U32("queue_id")], varlen_field=Str("name"), is_metadata=True),
    Evt("queue_kind",                     id=23, fields=[U32("queue_id"), U8Enum("kind", QueueKindEnum)], is_metadata=True),
    Evt("queue_send",                     id=24, fields=[U32("queue_id"), U32("len_after")]),
    Evt("queue_send_from_isr",            id=25, fields=[U32("queue_id"), U32("len_after")]),
    Evt("queue_overwrite",                id=26, fields=[U32("queue_id"), U32("len_after")]),
    Evt("queue_overwrite_from_isr",       id=27, fields=[U32("queue_id"), U32("len_after")]),
    Evt("queue_receive",                  id=28, fields=[U32("queue_id"), U32("len_after")]),
    Evt("queue_receive_from_isr",         id=29, fields=[U32("queue_id"), U32("len_after")]),
    Evt("queue_reset",                    id=30, fields=[U32("queue_id")]),
    Evt("curtask_block_on_queue_peek",    id=31, fields=[U32("queue_id"), U32("ticks_to_wait")]),
    Evt("curtask_block_on_queue_send",    id=32, fields=[U32("queue_id"), U32("ticks_to_wait")]),
    Evt("curtask_block_on_queue_receive", id=33, fields=[U32("queue_id"), U32("ticks_to_wait")]),

    # # Stream buffers:
    # Evt("streambuffer_created",                  id=34, fields=[U32("streambuffer_id")]),
    # Evt("streambuffer_name",                     id=35, fields=[U32("streambuffer_id")], varlen_field=Str("name"), is_metadata=True),
    # Evt("streambuffer_kind",                     id=36, fields=[U32("streambuffer_id"), U8Enum("kind", StreamBufferKindEnum)], is_metadata=True),
    # Evt("streambuffer_send",                     id=37, fields=[U32("streambuffer_id"), U32("amnt"), U32("len_after")]),
    # Evt("streambuffer_send_from_isr",            id=38, fields=[U32("streambuffer_id"), U32("amnt"), U32("len_after")]),
    # Evt("streambuffer_receive",                  id=39, fields=[U32("streambuffer_id"), U32("amnt"), U32("len_after")]),
    # Evt("streambuffer_receive_from_usr",         id=40, fields=[U32("streambuffer_id"), U32("amnt"), U32("len_after")]),
    # Evt("streambuffer_reset",                    id=41, fields=[U32("streambuffer_id")]),
    # Evt("curtask_block_on_streambuffer_send",    id=42, fields=[U32("streambuffer_id"), U32("ticks_to_wait")]),
    # Evt("curtask_block_on_streambuffer_receive", id=43, fields=[U32("streambuffer_id"), U32("ticks_to_wait")]),

    # Event Markers:
    Evt("evtmarker_name",  id=34, fields=[U32("evtmarker_id")], varlen_field=Str("name"), is_metadata=True),
    Evt("evtmarker",       id=35, fields=[U32("evtmarker_id")], varlen_field=Str("msg")),
    Evt("evtmarker_begin", id=36, fields=[U32("evtmarker_id")], varlen_field=Str("msg")),
    Evt("evtmarker_end",   id=37, fields=[U32("evtmarker_id")]),

    # Value Markers:
    Evt("valmarker_name", id=38, fields=[U32("valmarker_id")], varlen_field=Str("name"), is_metadata=True),
    Evt("valmarker",      id=39, fields=[U32("valmarker_id"), S64("val")]),

    # Task Event Markers:
    Evt("task_evtmarker_name",  id=40, fields=[U32("evtmarker_id"), U32("task_id")], varlen_field=Str("name"), is_metadata=True),
    Evt("task_evtmarker",       id=41, fields=[U32("evtmarker_id")], varlen_field=Str("msg")),
    Evt("task_evtmarker_begin", id=42, fields=[U32("evtmarker_id")], varlen_field=Str("msg")),
    Evt("task_evtmarker_end",   id=43, fields=[U32("evtmarker_id")]),

    # Task Value Markers:
    Evt("task_valmarker_name", id=44, fields=[U32("valmarker_id"), U32("task_id")], varlen_field=Str("name"), is_metadata=True),
    Evt("task_valmarker",      id=45, fields=[U32("valmarker_id"), S64("val")]),
]
# fmt: on

# ==== Entry ===================================================================


def main():
    # Check for duplicate IDs:
    s = set()
    for e in EVTS:
        if e.id in s:
            raise Exception(f"Duplicate id {e.id}/{e.name}")
        s.add(e.id)

    print("Events ok.", file=sys.stderr)

    script_loc = dirname(__file__)

    c_output_file = abspath(join(script_loc, "..", "frtrace-target", "core", "frtrace_encode.h"))
    gen_c_encoder.gen(EVTS, ENUMS, c_output_file)

    rs_crate_dir = abspath(join(script_loc, "..", "frtrace-conv", "frtrace-conv"))
    rs_output_file = abspath(join(script_loc, "..", "frtrace-conv", "frtrace-conv", "src", "decode", "evts.rs"))
    gen_rs_decoder.gen(EVTS, ENUMS, rs_output_file, rs_crate_dir)

if __name__ == '__main__':
    main()
