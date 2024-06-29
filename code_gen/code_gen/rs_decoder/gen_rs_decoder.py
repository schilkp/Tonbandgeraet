import os
import subprocess
import sys
from typing import List

from model import (
    BasicField,
    BasicFieldKind,
    Evt,
    EvtGroup,
    U8EnumDefinition,
    VarlenFieldKind,
)
from utils import pad_to_length


def pascal_case(i: str) -> str:
    return "".join(x for x in i.title() if not x == "_")


def read_file(f: str) -> str:
    script_loc = os.path.dirname(__file__)
    file_path = os.path.join(script_loc, f)
    with open(file_path, "r") as infile:
        return infile.read()


HEADER = read_file("header.rs")


def gen_u8_enum(e: U8EnumDefinition) -> str:
    result = ""

    # Enum:
    result += f"#[derive(Debug, Clone, Copy)]\n"
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
        result += f"      {entry_val} => Ok(Self::{pascal_case(entry_name)}),\n"
    result += f'      _ => Err(anyhow!("Invalid {e.name}"))\n'
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


def gen_evt_types(groups: List[EvtGroup]) -> str:
    result = ""
    result += f"{pad_to_length('// ==== Event Groups ', 100, '=')}\n"
    result += "\n"
    result += "#[derive(Debug, Clone, Copy)]\n"
    result += "pub enum TraceMode {\n"
    for group in groups:
        result += f"    {group.code_name()},\n"

    result += "}\n"
    result += "\n"
    result += "#[derive(Debug, Clone)]\n"
    result += "pub enum RawEvt {\n"
    result += "    Invalid(InvalidEvt),\n"
    for group in groups:
        if group.normal_evt_cnt() > 0:
            result += f"    {group.code_name()}({group.code_name()}Evt),\n"
        if group.metadata_evt_cnt() > 0:
            result += (
                f"    {group.code_name()}Metadata({group.code_name()}MetadataEvt),\n"
            )
    result += "}\n"
    result += "\n"

    result += "impl RawEvt {\n"
    result += "    pub fn ts(&self) -> Option<u64> {\n"
    result += "        match self {\n"
    result += "            RawEvt::Invalid(e) => e.ts,\n"
    for group in groups:
        if group.normal_evt_cnt() > 0:
            result += f"            RawEvt::{group.code_name()}(e) => Some(e.ts),\n"
        if group.metadata_evt_cnt() > 0:
            result += f"            RawEvt::{group.code_name()}Metadata(_) => None,\n"
    result += "        }\n"
    result += "    }\n"
    result += "}\n"
    result += "\n"

    result += "#[derive(Debug, Clone)]\n"
    result += "pub struct InvalidEvt {\n"
    result += "    pub ts: Option<u64>,\n"
    result += "    pub err: Option<Rc<anyhow::Error>>,\n"
    result += "}\n"
    result += "\n"

    return result


def gen_evt_group(group: EvtGroup) -> str:
    result = ""
    header_comment = f"// ==== {group.code_name()} Event Group "
    result += f"{pad_to_length(header_comment, 100, '=')}\n"
    result += f"\n"

    if group.normal_evt_cnt() > 0:
        result += f"#[derive(Debug, Clone)]\n"
        result += f"pub struct {group.code_name()}Evt {{\n"
        result += f"    pub ts: u64,\n"
        result += f"    pub kind: {group.code_name()}EvtKind,\n"
        result += f"}}\n"
        result += f"\n"
        result += f"#[derive(Debug, Clone)]\n"
        result += f"pub enum {group.code_name()}EvtKind {{\n"
        for evt in group.evts:
            if evt.is_metadata:
                continue
            result += f"    {pascal_case(evt.name)}({group.code_name()}{pascal_case(evt.name)}Evt),\n"
        result += f"}}\n"
        result += f"\n"
    if group.metadata_evt_cnt() > 0:
        result += f"#[derive(Debug, Clone)]\n"
        result += f"pub enum {group.code_name()}MetadataEvt {{\n"
        for evt in group.evts:
            if not evt.is_metadata:
                continue
            result += f"    {pascal_case(evt.name)}({group.code_name()}{pascal_case(evt.name)}Evt),\n"
        result += f"}}\n"
        result += f"\n"

    # Enums:
    for enum in group.enums:
        result += gen_u8_enum(enum)

    for evt in group.evts:
        result += gen_evt(evt, group)

    return result


def gen_evt(e: Evt, group: EvtGroup) -> str:
    result = ""

    # Struct:
    result += f"#[derive(Debug, Clone)]\n"
    result += f"pub struct {group.code_name()}{pascal_case(e.name)}Evt {{\n"
    for field in e.fields:
        result += f"  pub {field.name}: {basic_field_type(field.kind)},\n"
    for field in e.optional_fields:
        result += f"  pub {field.name}: Option<{basic_field_type(field.kind)}>,\n"
    if e.varlen_field is not None:
        result += (
            f"  pub {e.varlen_field.name}: {varlen_field_type(e.varlen_field.kind)},\n"
        )
    result += f"}}\n"
    result += f"\n"

    result += f"impl {group.code_name()}{pascal_case(e.name)}Evt {{\n"
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
        result += f"    Ok(RawEvt::{group.code_name()}Metadata({group.code_name()}MetadataEvt::{pascal_case(e.name)}( Self {{\n"
        indent = "      "
    else:
        result += f"    Ok(RawEvt::{group.code_name()}({group.code_name()}Evt {{\n"
        result += f"      ts,\n"
        result += (
            f"      kind: {group.code_name()}EvtKind::{pascal_case(e.name)} (Self {{\n"
        )
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


def gen_main_decode_func(groups: List[EvtGroup]) -> str:

    base_group = None
    for g in groups:
        if g.name == "":
            base_group = g

    assert base_group is not None

    result = ""
    result += f"{pad_to_length('// ==== Main Decode Function ', 100, '=')}\n"
    result += "\n"
    result += "impl RawEvt {\n"
    result += (
        "    pub fn decode(buf: &[u8], mode: TraceMode) -> anyhow::Result<Self> {\n"
    )
    result += "        let mut current_idx: usize = 0;;\n"
    result += "        let id: u8 = decode_u8(buf, &mut current_idx)?;\n"
    result += "        match id {\n"
    for event in base_group.evts:
        result += f"            0x{event.id:X} => {base_group.code_name()}{pascal_case(event.name)}Evt::decode(buf, &mut current_idx),\n"
    result += f"            id => match mode {{\n"
    for group in groups:
        if group.name == "":
            result += f'                TraceMode::{group.code_name()} => Err(anyhow!("Invalid event id 0x{{id:X}}!")),\n'
        else:
            result += f"                TraceMode::{group.code_name()} => match id {{\n"
            for event in group.evts:
                result += f"                0x{event.id:X} => {group.code_name()}{pascal_case(event.name)}Evt::decode(buf, &mut current_idx),\n"
            result += f'                    id => Err(anyhow!("Invalid event id 0x{{id:X}}!")),\n'
            result += f"                }}\n"

    result += f"            }},\n"
    result += "        }\n"
    result += "    }\n"
    result += "}\n"
    result += "\n"

    return result


def gen(groups: List[EvtGroup], output_file: str, crate_dir: str):

    result = ""
    result += HEADER
    result += "\n"

    result += gen_evt_types(groups)

    for group in groups:
        result += gen_evt_group(group)

    result += gen_main_decode_func(groups)

    with open(output_file, "w") as outfile:
        outfile.write(result)

    print(f"Generated {output_file}.", file=sys.stderr)

    subprocess.run(["cargo", "fmt"], cwd=crate_dir, check=True)
    print(f"Formatted frtrace-conv-core.", file=sys.stderr)
