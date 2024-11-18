import sys
from os.path import abspath, dirname, join

from model import S64, U32, U64, Evt, EvtGroup, Str, U8Enum, U8EnumDefinition

from c_encoder import gen_c_encoder
from c_tests import gen_c_tests
from rs_decoder import gen_rs_decoder
from md_docs import gen_md_docs


# ==== Events: Base ============================================================

ENUMS = []

# fmt: off
EVTS = [
    # Tracing events:
    Evt("core_id",           id=0, fields=[U32("core_id")]),
    Evt("dropped_evt_cnt",   id=1, fields=[U32("cnt")]),
    Evt("ts_resolution_ns",  id=2, fields=[U64("ns_per_ts")], is_metadata=True),

    # ISRs:
    Evt("isr_name",  id=3, fields=[U32("isr_id")], varlen_field=Str("name"), is_metadata=True),
    Evt("isr_enter", id=4, fields=[U32("isr_id")]),
    Evt("isr_exit",  id=5, fields=[U32("isr_id")]),

    # Event Markers:
    Evt("evtmarker_name",  id=6, fields=[U32("evtmarker_id")], varlen_field=Str("name"), is_metadata=True),
    Evt("evtmarker",       id=7, fields=[U32("evtmarker_id")], varlen_field=Str("msg")),
    Evt("evtmarker_begin", id=8, fields=[U32("evtmarker_id")], varlen_field=Str("msg")),
    Evt("evtmarker_end",   id=9, fields=[U32("evtmarker_id")]),

    # Value Markers:
    Evt("valmarker_name", id=10, fields=[U32("valmarker_id")], varlen_field=Str("name"), is_metadata=True),
    Evt("valmarker",      id=11, fields=[U32("valmarker_id"), S64("val")]),
]
# fmt: on

GROUP_BASE = EvtGroup("", EVTS, ENUMS)


# ==== Events: FreeRTOS ========================================================

QueueKindEnum = U8EnumDefinition(
    "FrQueueKind",
    [
        (0, "FRQK_QUEUE"),
        (1, "FRQK_COUNTING_SEMPHR"),
        (2, "FRQK_BINARY_SEMPHR"),
        (3, "FRQK_MUTEX"),
        (4, "FRQK_RECURSIVE_MUTEX"),
        (5, "FRQK_QUEUE_SET"),
    ],
)

StreamBufferKindEnum = U8EnumDefinition(
    "FrStreamBufferKind", [(0, "FRSBK_STREAM_BUFFER"), (1, "FRSBK_MESSAGE_BUFFER")]
)

ENUMS = [QueueKindEnum, StreamBufferKindEnum]

# fmt: off
EVTS = [
    # Task scheduling events:
    Evt("task_switched_in",      id=84, fields=[U32("task_id")]),
    Evt("task_to_rdy_state",     id=85, fields=[U32("task_id")]),
    Evt("task_resumed",          id=86, fields=[U32("task_id")]),
    Evt("task_resumed_from_isr", id=87, fields=[U32("task_id")]),
    Evt("task_suspended",        id=88, fields=[U32("task_id")]),
    Evt("curtask_delay",         id=89, fields=[U32("ticks")]),
    Evt("curtask_delay_until",   id=90, fields=[U32("time_to_wake")]),

    # Task priority events:
    Evt("task_priority_set",        id=91, fields=[U32("task_id"), U32("priority")]),
    Evt("task_priority_inherit",    id=92, fields=[U32("task_id"), U32("priority")]),
    Evt("task_priority_disinherit", id=93, fields=[U32("task_id"), U32("priority")]),

    # Task management events:
    Evt("task_created",       id=94, fields=[U32("task_id")]),
    Evt("task_name",          id=95, fields=[U32("task_id")], varlen_field=Str("name"), is_metadata=True),
    Evt("task_is_idle_task",  id=96, fields=[U32("task_id"), U32("core_id")], is_metadata=True),
    Evt("task_is_timer_task", id=97, fields=[U32("task_id")], is_metadata=True),
    Evt("task_deleted",       id=98, fields=[U32("task_id")]),

    # Queues:
    Evt("queue_created",                  id=99, fields=[U32("queue_id")]),
    Evt("queue_name",                     id=100, fields=[U32("queue_id")], varlen_field=Str("name"), is_metadata=True),
    Evt("queue_kind",                     id=101, fields=[U32("queue_id"), U8Enum("kind", QueueKindEnum)], is_metadata=True),
    Evt("queue_send",                     id=102, fields=[U32("queue_id"), U32("len_after")]),
    Evt("queue_send_from_isr",            id=103, fields=[U32("queue_id"), U32("len_after")]),
    Evt("queue_overwrite",                id=104, fields=[U32("queue_id"), U32("len_after")]),
    Evt("queue_overwrite_from_isr",       id=105, fields=[U32("queue_id"), U32("len_after")]),
    Evt("queue_receive",                  id=106, fields=[U32("queue_id"), U32("len_after")]),
    Evt("queue_receive_from_isr",         id=107, fields=[U32("queue_id"), U32("len_after")]),
    Evt("queue_reset",                    id=108, fields=[U32("queue_id")]),
    Evt("curtask_block_on_queue_peek",    id=109, fields=[U32("queue_id"), U32("ticks_to_wait")]),
    Evt("curtask_block_on_queue_send",    id=110, fields=[U32("queue_id"), U32("ticks_to_wait")]),
    Evt("curtask_block_on_queue_receive", id=111, fields=[U32("queue_id"), U32("ticks_to_wait")]),
    Evt("queue_cur_length",               id=112, fields=[U32("queue_id"), U32("length")]),

    # # Stream buffers:
    # Evt("streambuffer_created",                  id=112, fields=[U32("streambuffer_id")]),
    # Evt("streambuffer_name",                     id=113, fields=[U32("streambuffer_id")], varlen_field=Str("name"), is_metadata=True),
    # Evt("streambuffer_kind",                     id=114, fields=[U32("streambuffer_id"), U8Enum("kind", StreamBufferKindEnum)], is_metadata=True),
    # Evt("streambuffer_send",                     id=115, fields=[U32("streambuffer_id"), U32("amnt"), U32("len_after")]),
    # Evt("streambuffer_send_from_isr",            id=116, fields=[U32("streambuffer_id"), U32("amnt"), U32("len_after")]),
    # Evt("streambuffer_receive",                  id=117, fields=[U32("streambuffer_id"), U32("amnt"), U32("len_after")]),
    # Evt("streambuffer_receive_from_usr",         id=118, fields=[U32("streambuffer_id"), U32("amnt"), U32("len_after")]),
    # Evt("streambuffer_reset",                    id=119, fields=[U32("streambuffer_id")]),
    # Evt("curtask_block_on_streambuffer_send",    id=120, fields=[U32("streambuffer_id"), U32("ticks_to_wait")]),
    # Evt("curtask_block_on_streambuffer_receive", id=121, fields=[U32("streambuffer_id"), U32("ticks_to_wait")]),

    # Task Event Markers:
    Evt("task_evtmarker_name",  id=122, fields=[U32("evtmarker_id"), U32("task_id")], varlen_field=Str("name"), is_metadata=True),
    Evt("task_evtmarker",       id=123, fields=[U32("evtmarker_id")], varlen_field=Str("msg")),
    Evt("task_evtmarker_begin", id=124, fields=[U32("evtmarker_id")], varlen_field=Str("msg")),
    Evt("task_evtmarker_end",   id=125, fields=[U32("evtmarker_id")]),

    # Task Value Markers:
    Evt("task_valmarker_name", id=126, fields=[U32("valmarker_id"), U32("task_id")], varlen_field=Str("name"), is_metadata=True),
    Evt("task_valmarker",      id=127, fields=[U32("valmarker_id"), S64("val")]),
]
# fmt: on

GROUP_FREERTOS = EvtGroup("FreeRTOS", EVTS, ENUMS)

# ==== Entry ===================================================================


def main():

    groups = [GROUP_BASE, GROUP_FREERTOS]
    # Check for overlapping IDs within a group, or between the base and another group:
    base_ids = set()
    for e in GROUP_BASE.evts:
        if e.id in base_ids:
            raise Exception(f"Duplicate id {e.id}/{e.name}")
        base_ids.add(e.id)

    for group in groups[1:]:
        group_ids = set()
        for e in group.evts:
            if e.id in base_ids:
                raise Exception(f"Duplicate id {e.id}/{e.name}")
            if e.id in group_ids:
                raise Exception(f"Duplicate id {e.id}/{e.name}")
            group_ids.add(e.id)

    print("Events ok.", file=sys.stderr)

    script_loc = dirname(__file__)

    c_encoder_file = abspath(
        join(script_loc, "..", "..", "tband", "inc", "tband_encode.h")
    )
    gen_c_encoder.gen(groups, c_encoder_file)

    c_test_file = abspath(
        join(
            script_loc,
            "..",
            "..",
            "tests",
            "unit_test",
            "test_encoding_funcs",
            "test.c",
        )
    )
    gen_c_tests.gen(groups, c_test_file)

    rs_crate_dir = abspath(join(script_loc, "..", "..", "tools", "tband-conv"))
    rs_decoder_file = abspath(
        join(
            script_loc,
            "..",
            "..",
            "tools",
            "tband-conv",
            "src",
            "decode",
            "evts.rs",
        )
    )
    gen_rs_decoder.gen(groups, rs_decoder_file, rs_crate_dir)

    md_doc_file = abspath(
        join(
            script_loc,
            "..",
            "..",
            "docs",
            "tech_details",
            "bin_events.md",
        )
    )
    gen_md_docs.gen(groups, md_doc_file)


if __name__ == "__main__":
    main()
