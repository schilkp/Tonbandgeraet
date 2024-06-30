from dataclasses import dataclass
from typing import List, Literal, Optional, OrderedDict, Tuple, TypeAlias


@dataclass
class U8EnumDefinition:
    name: str
    entries: List[Tuple[int, str]]


BasicFieldKind: TypeAlias = Literal["u8", "u64", "u32", "s64"] | U8EnumDefinition


class BasicField:
    name: str
    kind: BasicFieldKind
    enum_entries: OrderedDict[int, str]

    def __init__(self, name: str, kind: BasicFieldKind):
        self.name = name
        self.kind = kind

    def is_enum(self) -> bool:
        return len(self.enum_entries) != 0


def U8(name: str) -> BasicField:
    return BasicField(name, "u8")


def U32(name: str) -> BasicField:
    return BasicField(name, "u32")


def U64(name: str) -> BasicField:
    return BasicField(name, "u64")


def S64(name: str) -> BasicField:
    return BasicField(name, "s64")


def U8Enum(name: str, enum: U8EnumDefinition) -> BasicField:
    return BasicField(name, enum)


VarlenFieldKind: TypeAlias = Literal["str"]


@dataclass
class VarlenField:
    name: str
    kind: VarlenFieldKind


def Str(name: str) -> VarlenField:
    return VarlenField(name, "str")


class Evt:
    name: str
    id: int  # 0-0x7F
    fields: List[BasicField]
    optional_fields: List[BasicField]
    varlen_field: Optional[VarlenField]
    is_metadata: bool

    def __init__(
        self,
        name: str,
        id: int,
        fields=list(),
        optional_fields=list(),
        varlen_field=None,
        is_metadata=False,
    ):
        self.name = name
        self.id = id
        self.fields = fields
        self.optional_fields = optional_fields
        self.varlen_field = varlen_field
        self.is_metadata = is_metadata

        if len(optional_fields) > 0 and varlen_field is not None:
            raise Exception("Event cannot have optional fields and varlen fields.")
