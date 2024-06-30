import os
import sys
from typing import List

from model import (BasicFieldKind, Evt, EvtGroup, U8EnumDefinition,
                   VarlenFieldKind)


def read_file(f: str) -> str:
    script_loc = os.path.dirname(__file__)
    file_path = os.path.join(script_loc, f)
    with open(file_path, "r") as infile:
        return infile.read()


HEADER = read_file("header.md")


def basic_field_type(f: BasicFieldKind) -> str:
    match f:
        case "u8":
            return "[u8](./bin_event_fields.md:u8)"
        case "u32":
            return "[u32](./bin_event_fields.md:s32)"
        case "u64":
            return "[u64](./bin_event_fields.md:u64)"
        case "s64":
            return "[s64](./bin_event_fields.md:s64)"
        case u8_enum:
            return f"[u8](./bin_event_fields.md:u8) enum [{u8_enum.name}](#{u8_enum.name.lower()})"


def basic_field_maxlen(f: BasicFieldKind) -> int:
    match f:
        case "u8":
            return 1
        case "u32":
            return 5
        case "u64":
            return 10
        case "s64":
            return 10
        case _:  # u8_enum
            return 1


def varlen_field_type(f: VarlenFieldKind) -> str:
    match f:
        case "str":
            return "[str](./bin_event_fields.md:str)"


def gen_u8_enum(e: U8EnumDefinition) -> str:
    result = f"#### {e.name}:\n"
    result += f"\n"
    for (val, name) in e.entries:
        result += f"- 0x{val:02X}: `{name}`\n"
    result += "\n"
    return result


def gen_evt_doc(evt: Evt, group: EvtGroup) -> str:
    result = ""
    result += f"### {group.code_name()}/{evt.name}:\n"
    result += f"\n"

    # fields:
    row_lbl = ["**Field Name:**", "`id`"]
    row_type = ["**Field Type:**", basic_field_type('u8')]
    row_note = ["**Note:**", f"0x{evt.id:02X}"]

    if not evt.is_metadata:
        row_lbl.append("`ts`")
        row_type.append(basic_field_type("u64"))
        row_note.append("required")

    for field in evt.fields:
        row_lbl.append(f"`{field.name}`")
        row_type.append(basic_field_type(field.kind))
        row_note.append("required")

    for field in evt.optional_fields:
        row_lbl.append(f"`{field.name}`")
        row_type.append(basic_field_type(field.kind))
        row_note.append("optional")

    if evt.varlen_field is not None:
        row_lbl.append(f"`{evt.varlen_field.name}`")
        row_type.append(varlen_field_type(evt.varlen_field.kind))
        row_note.append("varlen")


    row_sep = [":-"] + [":-:"] * (len(row_lbl)-1)

    result += "| " + " | ".join(row_lbl) + " |\n"
    result += "| " + " | ".join(row_sep) + " |\n"
    result += "| " + " | ".join(row_type) + " |\n"
    result += "| " + " | ".join(row_note) + " |\n"
    result += "\n"

    # metadata:
    is_metadata_val = "yes" if evt.is_metadata else "no"
    result += f"- Metadata: {is_metadata_val}\n"

    # max len:
    maxlen_unframed = 0
    maxlen_unframed += basic_field_maxlen("u8")  # ID
    if not evt.is_metadata:
        maxlen_unframed += basic_field_maxlen("u64")  # TS
    for field in evt.fields:
        maxlen_unframed += basic_field_maxlen(field.kind)
    for field in evt.optional_fields:
        maxlen_unframed += basic_field_maxlen(field.kind)

    if evt.varlen_field is not None:
        maxlen_unframed = f"{maxlen_unframed} bytes + varlen field"
    else:
        maxlen_unframed = f"{maxlen_unframed} bytes"

    result += f"- Max length (unframed): {maxlen_unframed}\n"
    result += f"\n"

    return result


def gen(groups: List[EvtGroup], output_file: str):

    result = ""
    result += HEADER
    result += "\n"

    for group in groups:

        result += f"## {group.code_name()}:\n"
        result += "\n"

        if len(group.enums) > 0:
            result += f"### {group.code_name()} Enums:\n"
            result += f"\n"
            for enum in group.enums:
                result += gen_u8_enum(enum)

        for evt in group.evts:
            result += gen_evt_doc(evt, group)

    with open(output_file, "w") as outfile:
        outfile.write(result)

    print(f"Generated {output_file}.", file=sys.stderr)
