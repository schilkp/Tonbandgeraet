import os
import sys
from copy import copy
from typing import List, Tuple

from model import (
    U64,
    BasicFieldKind,
    Evt,
    EvtGroup,
    VarlenFieldKind,
)
from utils import pad_to_length


def read_file(f: str) -> str:
    script_loc = os.path.dirname(__file__)
    file_path = os.path.join(script_loc, f)
    with open(file_path, "r") as infile:
        return infile.read()


HEADER = read_file("header.c")


def varint(val: int, byte_count: int) -> List[int]:
    result = []
    for _ in range(byte_count):
        bits = val & 0x7F
        val = val >> 7
        if val != 0:
            result.append(bits | 0x80)
        else:
            result.append(bits | 0x00)
            break
    return result


def basic_field_test_zero(f: BasicFieldKind) -> Tuple[str, List[int]]:
    match f:
        case "u8":
            return ("0x0", [0])
        case "u32":
            return ("0x0", varint(0, 5))
        case "u64":
            return ("0x0", varint(0, 10))
        case "s64":
            return ("0x0", varint(0, 10))
        case _:  # u8_enum
            return ("0x0", [0])


def basic_field_test_max(f: BasicFieldKind) -> Tuple[str, List[int]]:
    match f:
        case "u8":
            return ("UINT8_MAX", [0xFF])
        case "u32":
            return ("UINT32_MAX", varint(0xFFFF_FFFF, 5))
        case "u64":
            return ("UINT64_MAX", varint(0xFFFF_FFFF_FFFF_FFFF, 10))
        case "s64":
            return ("INT64_MIN + 1", varint(0xFFFF_FFFF_FFFF_FFFF, 10))
        case _:  # u8_enum
            return ("UINT8_MAX", [0xFF])


def varlen_field_test(f: VarlenFieldKind) -> Tuple[str, List[int]]:
    match f:
        case "str":
            return ('"test"', [ord("t"), ord("e"), ord("s"), ord("t")])


def gen_test_func(evt: Evt, group: EvtGroup) -> Tuple[str, str]:
    result = ""
    evt_macro_name = evt.name.upper()
    group_macro_name = group.name.upper() + "_" if group.name != "" else ""
    group_func_name = group.name.lower() + "_" if group.name != "" else ""

    maxlen_macro_name = f"EVT_{group_macro_name}{evt_macro_name}_MAXLEN"

    fields = copy(evt.fields)
    if not evt.is_metadata:
        fields.insert(0, U64("ts"))

    test_name = f"test_{group_func_name}{evt.name}"
    result += f"void {test_name}(void){{\n"

    inputs = []
    output_bytes = [hex(evt.id)]
    for field in fields:
        (inp, out) = basic_field_test_zero(field.kind)
        inputs.append(inp)
        output_bytes.extend(map(hex, out))

    if evt.varlen_field is not None:
        (inp, out) = varlen_field_test(evt.varlen_field.kind)
        inputs.append(inp)
        output_bytes.extend(map(hex, out))

    result += f"  {{\n"
    result += f"    // Min\n"
    result += f"    uint8_t buf[{maxlen_macro_name}] = {{0}};\n"
    result += f"    size_t len = encode_{group_func_name}{evt.name}(buf, {', '.join(inputs)});\n"
    result += f"    uint8_t expected[] = {{{', '.join(output_bytes)}}};\n"
    result += f'    compare_arrays(buf, len, expected, sizeof(expected), "MIN");\n'
    result += f"  }}\n"

    inputs = []
    output_bytes = [hex(evt.id)]
    for field in fields:
        (inp, out) = basic_field_test_max(field.kind)
        inputs.append(inp)
        output_bytes.extend(map(hex, out))

    if evt.varlen_field is not None:
        (inp, out) = varlen_field_test(evt.varlen_field.kind)
        inputs.append(inp)
        output_bytes.extend(map(hex, out))

    result += f"  {{\n"
    result += f"    // Max\n"
    result += f"    uint8_t buf[{maxlen_macro_name}] = {{0}};\n"
    result += f"    size_t len = encode_{group_func_name}{evt.name}(buf, {', '.join(inputs)});\n"
    result += f"    uint8_t expected[] = {{{', '.join(output_bytes)}}};\n"
    result += f'    compare_arrays(buf, len, expected, sizeof(expected), "MAX");\n'
    result += f"  }}\n"

    result += f"}}\n"
    result += f"\n"
    return (result, test_name)


def gen(groups: List[EvtGroup], output_file: str):

    result = ""
    result += HEADER
    result += "\n"

    test_names = []

    for group in groups:

        divider_comment = f"// ==== {group.code_name()} Encoder Tests "
        result += f"{pad_to_length(divider_comment, 120, '=')}\n"
        result += "\n"

        for evt in group.evts:
            for variant in evt.get_variants():
                (lines, test_name) = gen_test_func(variant, group)
                result += lines
                test_names.append(test_name)

    result += "// ==== Main =======================================================================================\n"
    result += "\n"
    result += "void setUp(void) {}\n"
    result += "\n"
    result += "void tearDown(void) {}\n"
    result += "\n"
    result += "int main(void) {\n"
    result += "  UNITY_BEGIN();\n"

    for test_name in test_names:
        result += f"  RUN_TEST({test_name});\n"

    result += "  return UNITY_END();\n"
    result += "}\n"

    with open(output_file, "w") as outfile:
        outfile.write(result)

    print(f"Generated {output_file}.", file=sys.stderr)
