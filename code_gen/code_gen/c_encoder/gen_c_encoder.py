import os
import sys
from typing import List, Optional

from model import (
    BasicField,
    BasicFieldKind,
    Evt,
    U8EnumDefinition,
    VarlenField,
    VarlenFieldKind,
)


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


def gen_enc_func(
    name: str,
    id: int,
    is_metadata: bool,
    fields: List[BasicField],
    varlen_field: Optional[VarlenField],
) -> str:
    result = ""
    macro_name = name.upper()

    # metadata define:
    is_metadata_val = 1 if is_metadata else 0
    result += f"#define EVT_{macro_name}_IS_METADATA ({is_metadata_val})\n"

    # Max len define:
    maxlen_unframed = 0
    maxlen_unframed += basic_field_maxlen("u8")  # ID
    if not is_metadata:
        maxlen_unframed += basic_field_maxlen("u64")  # TS
    for field in fields:
        maxlen_unframed += basic_field_maxlen(field.kind)
    if varlen_field is not None:
        maxlen_unframed = (
            f"{maxlen_unframed} + {varlen_field_maxlen(varlen_field.kind)}"
        )
    else:
        maxlen_unframed = f"{maxlen_unframed}"

    result += f"#define EVT_{macro_name}_MAXLEN (COBS_MAXLEN(({maxlen_unframed})))\n"

    # function args:
    args = [f"uint8_t buf[EVT_{macro_name}_MAXLEN]"]

    if not is_metadata:
        args.append("uint64_t ts")

    for field in fields:
        args.append(f"{basic_field_type(field.kind)} {field.name}")

    if varlen_field is not None:
        args.append(f"{varlen_field_type(varlen_field.kind)}{varlen_field.name}")

    result += f"static inline size_t encode_{name}({', '.join(args)}) {{\n"
    result += f"  struct cobs_state cobs = cobs_start(buf);\n"

    # Event ID:
    result += f"  encode_u8(&cobs, 0x{id:X});\n"

    if not is_metadata:
        result += f"  encode_u64(&cobs, ts);\n"

    for field in fields:
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

    if varlen_field is not None:
        match varlen_field.kind:
            case "str":
                result += f"  encode_str(&cobs, {varlen_field.name});\n"

    result += f"  return cobs_finish(&cobs);\n"
    result += f"}}\n"
    result += f"\n"
    return result


def gen(evts: List[Evt], enums: List[U8EnumDefinition], output_file: str):

    result = ""
    result += HEADER
    result += "\n"

    result += "// ==== Enums ==================================================================\n"
    result += "\n"
    for enum in enums:
        result += gen_u8_enum(enum)

    result += "// ==== Encoder Functions ======================================================\n"
    result += "\n"

    for evt in evts:
        if len(evt.optional_fields) != 0:
            name = evt.name + "_opt0"
            result += gen_enc_func(name, evt.id, evt.is_metadata, evt.fields, None)

            for opt_variant in range(len(evt.optional_fields)):
                name = evt.name + f"_opt{opt_variant+1}"
                fields = evt.fields + evt.optional_fields[: opt_variant + 1]
                result += gen_enc_func(name, evt.id, evt.is_metadata, fields, None)
        else:
            result += gen_enc_func(
                evt.name, evt.id, evt.is_metadata, evt.fields, evt.varlen_field
            )

    result += FOOTER

    with open(output_file, "w") as outfile:
        outfile.write(result)

    print(f"Generated {output_file}.", file=sys.stderr)
