import os
import subprocess
import sys
from typing import List

from model import (BasicField, BasicFieldKind, Evt, U8EnumDefinition,
                   VarlenFieldKind)


def pascal_case(i: str) -> str:
    return ''.join(x for x in i.title() if not x == '_')


def read_file(f: str) -> str:
    script_loc = os.path.dirname(__file__)
    file_path = os.path.join(script_loc, f)
    with open(file_path, 'r') as infile:
        return infile.read()


HEADER = read_file("rs_file_header.rs")


def gen_u8_enum(e: U8EnumDefinition) -> str:
    result = ""

    # Enum:
    result += f"#[derive(Debug, Clone)]\n"
    result += f"pub enum {e.name} {{\n"
    for entry_val, entry_name in e.entries:
        result += f"  {pascal_case(entry_name)},\n"
    result += f"}}\n"
    result += f"\n"

    # u8 to enum conversion:
    result += f"impl TryFrom<u8> for {e.name} {{\n"
    result += f"  type Error = anyhow::Error;\n"
    result += f"\n"
    result += f"  fn try_from(value: u8) -> Result<Self, Self::Error> {{\n"
    result += f"    match value {{\n"
    for entry_val, entry_name in e.entries:
        result += f"      {
            entry_val} => Ok(Self::{pascal_case(entry_name)}),\n"
    result += f"      _ => Err(anyhow!(\"Invalid {e.name}\"))\n"
    result += f"    }}\n"
    result += f"  }}\n"
    result += f"}}\n"
    result += f"\n"

    return result


def basic_field_type(f: BasicFieldKind) -> str:
    match f:
        case "u8":
            return "u8"
        case "u32":
            return "u32"
        case "u64":
            return "u64"
        case "s64":
            return "i64"
        case u8_enum:
            return u8_enum.name


def basic_field_decode(f: BasicField) -> str:
    match f.kind:
        case "u8":
            return f"decode_u8(buf, current_idx).context(\"Failed to decode '{f.name}' u8 field.\")?"
        case "u32":
            return f"decode_u32(buf, current_idx).context(\"Failed to decode '{f.name}' u32 field.\")?"
        case "u64":
            return f"decode_u64(buf, current_idx).context(\"Failed to decode '{f.name}' u64 field.\")?"
        case "s64":
            return f"decode_s64(buf, current_idx).context(\"Failed to decode '{f.name}' s64 field.\")?"
        case u8_enum:
            return f"{u8_enum.name}::try_from(decode_u8(buf, current_idx).context(\"Failed to decode '{f.name}' u8 enum field.\")?).context(\"Failed to decode '{f.name}' u8 enum field.\")?"


def varlen_field_type(f: VarlenFieldKind) -> str:
    match f:
        case "str":
            return "String"


def varlen_field_decode(f: VarlenFieldKind) -> str:
    match f:
        case "str":
            return "decode_string(buf, current_idx)?"


def gen_evt_types(evts: List[Evt]) -> str:
    result = ""

    # Metadata Events:
    result += f"#[derive(Debug, Clone)]\n"
    result += f"pub enum RawMetadataEvt {{\n"
    for evt in evts:
        if evt.is_metadata:
            result += f"  {pascal_case(evt.name)}({pascal_case(evt.name)}Evt),\n"
    result += f"}}\n"
    result += f"\n"

    # Normal trace events::
    result += f"#[derive(Debug, Clone)]\n"
    result += f"pub enum RawTraceEvtKind {{\n"
    for evt in evts:
        if not evt.is_metadata:
            result += f"  {pascal_case(evt.name)}({pascal_case(evt.name)}Evt),\n"
    result += f"}}\n"
    result += f"\n"

    # Decode:
    result += f"impl RawEvt {{\n"
    result += f"  pub fn decode(buf: &[u8]) -> anyhow::Result<Self> {{\n"
    result += f"    let mut idx = 0;\n"
    result += f"    let id = decode_u8(buf, &mut idx)?;\n"
    result += f"    match id {{\n"
    for evt in evts:
        result += f"      {evt.id} => {pascal_case(evt.name)}Evt::decode(buf, &mut idx),\n"
    result += f"      _ => Err(anyhow!(\"Invalid event id {{id}}\"))?\n"
    result += f"    }}\n"
    result += f"  }}\n"
    result += f"}}\n"
    result += f"\n"

    return result


def gen_evt(e: Evt) -> str:
    result = ""

    # Struct:
    result += f"#[derive(Debug, Clone)]\n"
    result += f"pub struct {pascal_case(e.name)}Evt {{\n"
    for field in e.fields:
        result += f"  pub {field.name}: {basic_field_type(field.kind)},\n"
    for field in e.optional_fields:
        result += f"  pub {field.name}: Option<{
            basic_field_type(field.kind)}>,\n"
    if e.varlen_field is not None:
        result += f"  pub {e.varlen_field.name}: {
            varlen_field_type(e.varlen_field.kind)},\n"
    result += f"}}\n"
    result += f"\n"

    result += f"impl {pascal_case(e.name)}Evt {{\n"
    result += f"  fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {{\n"

    if not e.is_metadata:
        result += f"    let ts = decode_u64(buf, current_idx)?;\n"

    for field in e.fields:
        result += f"    let {field.name} = {basic_field_decode(field)};\n"
    for field in e.optional_fields:
        result += f"  let {field.name} = if bytes_left(buf, *current_idx) {{\n"
        result += f"    Some({basic_field_decode(field)})\n"
        result += f"  }} else {{\n"
        result += f"    None\n"
        result += f"  }};n"
    if e.varlen_field is not None:
        result += f"    let {e.varlen_field.name} = {varlen_field_decode(e.varlen_field.kind)};\n"
    result += f"    if bytes_left(buf, *current_idx) {{\n"
    result += f"      return Err(anyhow!(\"Loose bytes at end of '{pascal_case(e.name)}' event.\"));\n"
    result += f"    }}\n"

    if e.is_metadata:
        result += f"    Ok(RawEvt::Metadata(RawMetadataEvt::{pascal_case(e.name)}( Self {{\n"
        indent = "      "
    else:
        result += f"    Ok(RawEvt::Trace(RawTraceEvt {{\n"
        result += f"      ts,\n"
        result += f"      kind: RawTraceEvtKind::{pascal_case(e.name)} (Self {{\n"
        indent = "        "

    for field in e.fields:
        result += f"{indent}{field.name},\n"
    for field in e.optional_fields:
        result += f"{indent}{field.name},\n"
    if e.varlen_field is not None:
        result += f"{indent}{e.varlen_field.name},\n"

    if e.is_metadata:
        result += f"    }})))\n"
    else:
        result += f"      }})\n"
        result += f"    }}))\n"
    result += f"  }}\n"
    result += f"}}\n"
    result += f"\n"

    return result


def gen(evts: List[Evt], enums: List[U8EnumDefinition], output_file: str, crate_dir: str):

    result = ""
    result += HEADER
    result += "\n"

    # Enums:
    result += "// ==== Enums ==================================================================\n"
    result += "\n"
    for enum in enums:
        result += gen_u8_enum(enum)

    # Events:
    result += "// ==== Event Types ============================================================\n"
    result += "\n"
    result += gen_evt_types(evts)

    result += "// ==== Individual Events ======================================================\n"
    result += "\n"
    for evt in evts:
        result += gen_evt(evt)

    with open(output_file, "w") as outfile:
        outfile.write(result)

    print(f"Generated {output_file}.", file=sys.stderr)

    subprocess.run(["cargo", "fmt"], cwd=crate_dir, check=True)
    print(f"Formatted frtrace-conv-core.", file=sys.stderr)
