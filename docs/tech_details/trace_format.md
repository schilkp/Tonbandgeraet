# The Tonbandgerät Binary Trace Format

To store trace events in a compact binary format, Tonbandgerät uses zero-delimited [COBS](./cobs.md)
frames each containing a single trace event. The [set of trace event types](./bin_events.md) (each identified by an 8
bit id) and their respective structure is fixed.

## Trace Event Structure

Inside the COBS frame, each event type is structured as follows:

- An 8-bit ID, which identifies the type of trace event and hence internal structure of the fields to follow.
- Zero or more *required* [known-length fields](#known-length-fields), which are always present in every event instance of this type.
- One of the following:
    - Zero or more *optional* [known-length fields](#known-length-fields), which (under some restrictions) are not required to
      be present in every event instance of this type.
    - One [variable-length](#variable-length-fields) field.
    - Nothing


Some examples of valid event structures follow, where each field is denoted as `name:type` with optional fields enclosed in square braces (`[]`), and variable-length fields appended with an ellipses (`...`):

```text
id:u8
id:u8 field1:u64
id:u8 field1:u64 field2:u8
id:u8 field1:u64 (opt:s64)
id:u8 field1:u64 (opt:s64) (abc:u8)
id:u8 (opt:s64)  (abc:u8)
id:u8 field1:64  name:str...
id:u8 name:str...
```
See [here](./bin_event_fields.md) for a description of the specific field types.

## Known-length fields

Known-length fields are fields whose encoded length is either fixed (such as `u8` fields, which are always 1 byte)
or can be determined from the encoded data (such as `u32` fields, whose varlen encoding scheme identifies the last
byte of a field by having its MSB be zero, see [field types](./bin_event_fields.md)).

## Required fields

Required fields are straight forward to encode and decode: The encoder serializes all fields and inserts them
sequentially into the event frame. Since they are all required to be [known-length fields](#known-length-fields), and
the decoder is aware of the set of required fields present, it can simply identify and decode them.

## Optional fields

Optional fields must not necessarily be included in an event. If they are present, they are encoded as usual.
If they are not present, they are simply omitted. Importantly however, they must always appear in the order in which they are defined
and cannot be excluded and included freely without restriction:

If an event type contains \\(N\\) optional fields, an event instance may only exclude the *last* \\(0 \leq n \leq N\\) fields.
In other words, if an event type contains the optional fields \\(A\\), \\(B\\), and \\(C\\), valid
event instances could only take on one of the following layouts:

 - \\(\emptyset\\)
 - \\(A\\)
 - \\(A\\), \\(B\\)
 - \\(A\\), \\(B\\), \\(C\\)

This restriction comes from the fact that, to save space, there is no field identification mechanism in the trace format.
Instead, while decoding optional events, the decoder will simply continue to decode until it reaches the end of the frame,
and mark all events that were not seen as not present.

## Variable-length fields

Variable length fields (such as strings) are encoded without any delimiter. Since they must appear as the last field in an 
event, the decoder assumes that all bytes that follow the last required field are a part of the variable-length field.
