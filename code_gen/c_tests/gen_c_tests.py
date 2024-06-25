import os
import sys
from copy import copy
from typing import List, Optional, Tuple

from model import (U64, BasicField, BasicFieldKind, Evt, U8EnumDefinition,
                   VarlenField, VarlenFieldKind)


def read_file(f: str) -> str:
    script_loc = os.path.dirname(__file__)
    file_path = os.path.join(script_loc, f)
    with open(file_path, 'r') as infile:
        return infile.read()


HEADER = read_file("header.h")


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
            return ('"test"', [ord('t'), ord('e'), ord('s'), ord('t')])


def gen_test_func(name: str, id: int, is_metadata: bool, fields: List[BasicField], varlen_field: Optional[VarlenField]) -> str:
    result = ""
    macro_name = name.upper()
    maxlen_name = f"EVT_{macro_name}_MAXLEN"

    fields = copy(fields)
    if not is_metadata:
        fields.insert(0, U64("ts"))

    result += f"void test_{name}(void){{\n"

    inputs = []
    output_bytes = [hex(id)]
    for field in fields:
        (inp, out) = basic_field_test_zero(field.kind)
        inputs.append(inp)
        output_bytes.extend(map(hex, out))

    if varlen_field is not None:
        (inp, out) = varlen_field_test(varlen_field.kind)
        inputs.append(inp)
        output_bytes.extend(map(hex, out))

    result += f"  {{\n"
    result += f"    // Min\n"
    result += f"    uint8_t buf[{maxlen_name}] = {{0}};\n"
    result += f"    size_t len = encode_{name}(buf, {", ".join(inputs)});\n"
    result += f"    uint8_t expected[] = {{{", ".join(output_bytes)}}};\n"
    result += f"    compare_arrays(buf, len, expected, sizeof(expected), \"MIN\");\n"
    result += f"  }}\n"

    inputs = []
    output_bytes = [hex(id)]
    for field in fields:
        (inp, out) = basic_field_test_max(field.kind)
        inputs.append(inp)
        output_bytes.extend(map(hex, out))

    if varlen_field is not None:
        (inp, out) = varlen_field_test(varlen_field.kind)
        inputs.append(inp)
        output_bytes.extend(map(hex, out))

    result += f"  {{\n"
    result += f"    // Max\n"
    result += f"    uint8_t buf[{maxlen_name}] = {{0}};\n"
    result += f"    size_t len = encode_{name}(buf, {", ".join(inputs)});\n"
    result += f"    uint8_t expected[] = {{{", ".join(output_bytes)}}};\n"
    result += f"    compare_arrays(buf, len, expected, sizeof(expected), \"MAX\");\n"
    result += f"  }}\n"

    result += f"}}\n"
    result += f"\n"
    return result
#


def gen(evts: List[Evt], enums: List[U8EnumDefinition], output_file: str):

    result = ""
    result += HEADER
    result += "\n"

    result += "// ==== Tests ======================================================================================\n"
    result += "\n"

    func_names = []

    for evt in evts:
        if len(evt.optional_fields) != 0:
            name = evt.name + "_opt0"
            func_names.append(name)
            result += gen_test_func(name, evt.id, evt.is_metadata, evt.fields, None)

            for opt_variant in range(len(evt.optional_fields)):
                name = evt.name + f"_opt{opt_variant+1}"
                fields = evt.fields + evt.optional_fields[:opt_variant+1]
                func_names.append(name)
                result += gen_test_func(name, evt.id, evt.is_metadata, fields, None)
        else:
            func_names.append(evt.name)
            result += gen_test_func(evt.name, evt.id, evt.is_metadata, evt.fields, evt.varlen_field)

    result += "// ==== Main =======================================================================================\n"
    result += "\n"
    result += "void setUp(void) {}\n"
    result += "\n"
    result += "void tearDown(void) {}\n"
    result += "\n"
    result += "int main(void) {\n"
    result += "  UNITY_BEGIN();\n"

    for func_name in func_names:
        result += f"  RUN_TEST(test_{func_name});\n"

    result += "  return UNITY_END();\n"
    result += "}\n"

    # result += FOOTER

    with open(output_file, "w") as outfile:
        outfile.write(result)

    print(f"Generated {output_file}.", file=sys.stderr)
