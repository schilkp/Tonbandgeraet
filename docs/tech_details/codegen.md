# Code Generation

The set of possible tracing events and their fields is defined in a simple python code
generation script [here](https://github.com/schilkp/Tonbandgeraet/blob/main/codegen/codegen/__main__.py).
Based on this, the [c event encoder](https://github.com/schilkp/Tonbandgeraet/blob/main/tband/inc/tband_encode.h),
an event [event decoder test file](https://github.com/schilkp/Tonbandgeraet/blob/main/tests/unit_test/test_encoding_funcs/test.c),
the [rust event decoder](https://github.com/schilkp/Tonbandgeraet/blob/main/tools/tband-conv/src/decode/evts.rs),
and the [event index documentation](https://github.com/schilkp/Tonbandgeraet/blob/main/docs/tech_details/bin_events.md) is generated.

# Example Output

Consider the following `isr_name` event as an example:

| **Field Name:** | `id` | `isr_id` | `name` |
| :- | :-: | :-: | :-: |
| **Field Type:** | [u8](./bin_event_fields.md:u8) | [u32](./bin_event_fields.md:s32) | [str](./bin_event_fields.md:str) |
| **Note:** | 0x03 | required | varlen |

- Metadata: yes
- Max length (unframed): 6 bytes + varlen field

### C Encoder
First, the code generator will emit a macro that specifies if the event is a metadata event (meaning it should be appended 
to the metadata buffer) and a macro that gives the maximum framed length of the event so that a buffer can be pre-allocated.
Then, a function is generated that takes the event fields and encodes them into a buffer, returning the actual encoded length
of the message (including a trailing zero termination).

```c
#define EVT_ISR_NAME_IS_METADATA (1)
#define EVT_ISR_NAME_MAXLEN (COBS_MAXLEN((6 + tband_configMAX_STR_LEN)))
static inline size_t encode_isr_name(uint8_t buf[EVT_ISR_NAME_MAXLEN], uint32_t isr_id, const char *name) {/* .. */}
```

### Rust Decoder

The rust code generator emits a struct for each field, and a decoder function that attempts to reconstruct the event
from a given buffer:

```rs
#[derive(Debug, Clone, Serialize)]
pub struct BaseIsrNameEvt {
    pub isr_id: u32,
    pub name: String,
}

impl BaseIsrNameEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> { /* .. */ }
}

```
