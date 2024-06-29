import os
import sys
from typing import List

from model import BasicFieldKind, Evt, EvtGroup, U8EnumDefinition, VarlenFieldKind
from utils import pad_to_length


def read_file(f: str) -> str:
    script_loc = os.path.dirname(__file__)
    file_path = os.path.join(script_loc, f)
    with open(file_path, "r") as infile:
        return infile.read()


HEADER = read_file("header.h")
FOOTER = read_file("footer.h")


def basic_field_type(f: BasicFieldKind) -> str:
    match f:
        case "u8":
            return "uint8_t"
        case "u32":
            return "uint32_t"
        case "u64":
            return "uint64_t"
        case "s64":
            return "int64_t"
        case u8_enum:
            return "enum " + u8_enum.name


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
            return "const char *"


def varlen_field_maxlen(f: VarlenFieldKind) -> str:
    match f:
        case "str":
            return "frtrace_configMAX_STR_LEN"


def gen_u8_enum(e: U8EnumDefinition) -> str:
    result = ""
    result += f"enum {e.name} {{\n"
    for entry_val, entry_name in e.entries:
        result += f"  {entry_name} = 0x{entry_val:x},\n"
    result += f"}};\n"
    result += f"\n"
    return result


def gen_enc_func(evt: Evt, group: EvtGroup) -> str:
    result = ""
    evt_macro_name = evt.name.upper()
    group_macro_name = group.name.upper() + "_" if group.name != "" else ""
    group_func_name = group.name.lower() + "_" if group.name != "" else ""

    # metadata define:
    is_metadata_val = 1 if evt.is_metadata else 0
    result += f"#define EVT_{group_macro_name}{evt_macro_name}_IS_METADATA ({is_metadata_val})\n"

    # Max len define:
    maxlen_unframed = 0
    maxlen_unframed += basic_field_maxlen("u8")  # ID
    if not evt.is_metadata:
        maxlen_unframed += basic_field_maxlen("u64")  # TS
    for field in evt.fields:
        maxlen_unframed += basic_field_maxlen(field.kind)
    if evt.varlen_field is not None:
        maxlen_unframed = (
            f"{maxlen_unframed} + {varlen_field_maxlen(evt.varlen_field.kind)}"
        )
    else:
        maxlen_unframed = f"{maxlen_unframed}"

    maxlen_macro_name = f"EVT_{group_macro_name}{evt_macro_name}_MAXLEN"
    result += f"#define {maxlen_macro_name} (COBS_MAXLEN(({maxlen_unframed})))\n"

    # function args:
    args = [f"uint8_t buf[{maxlen_macro_name}]"]

    if not evt.is_metadata:
        args.append("uint64_t ts")

    for field in evt.fields:
        args.append(f"{basic_field_type(field.kind)} {field.name}")

    if evt.varlen_field is not None:
        args.append(
            f"{varlen_field_type(evt.varlen_field.kind)}{evt.varlen_field.name}"
        )

    result += f"static inline size_t encode_{group_func_name}{evt.name}({', '.join(args)}) {{\n"
    result += f"  struct cobs_state cobs = cobs_start(buf);\n"

    # Event ID:
    result += f"  encode_u8(&cobs, 0x{evt.id:X});\n"

    if not evt.is_metadata:
        result += f"  encode_u64(&cobs, ts);\n"

    for field in evt.fields:
        match field.kind:
            case "u8":
                result += f"  encode_u8(&cobs, {field.name});\n"
            case "u32":
                result += f"  encode_u32(&cobs, {field.name});\n"
            case "u64":
                result += f"  encode_u64(&cobs, {field.name});\n"
            case "s64":
                result += f"  encode_s64(&cobs, {field.name});\n"
            case _:  # u8_enum
                result += f"  encode_u8(&cobs, (uint8_t){field.name});\n"

    if evt.varlen_field is not None:
        match evt.varlen_field.kind:
            case "str":
                result += f"  encode_str(&cobs, {evt.varlen_field.name});\n"

    result += f"  return cobs_finish(&cobs);\n"
    result += f"}}\n"
    result += f"\n"
    return result


def gen(groups: List[EvtGroup], output_file: str):

    result = ""
    result += HEADER
    result += "\n"

    for group in groups:

        divider_comment = f"// ==== {group.code_name()} Enums "
        result += f"{pad_to_length(divider_comment, 80, '=')}\n"
        result += "\n"

        for enum in group.enums:
            result += gen_u8_enum(enum)

        divider_comment = f"// ==== {group.code_name()} Encoder Functions "
        result += f"{pad_to_length(divider_comment, 80, '=')}\n"
        result += "\n"

        for evt in group.evts:
            for variant in evt.get_variants():
                result += gen_enc_func(variant, group)

    result += FOOTER

    with open(output_file, "w") as outfile:
        outfile.write(result)

    print(f"Generated {output_file}.", file=sys.stderr)
