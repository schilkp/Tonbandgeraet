# Trace Event Fields

The following field types are supported in trace packages:

### u8
An unsigned, 8-bit value. Encoded as-is.

### u32
An unsigned, 32-bit value. Encoded as a varlen unsigned value.

### u64
An unsigned, 64-bit value. Encoded as a varlen unsigned value.

### s64
A signed 64-bit value. Encoded in sign-magnitude form as a varlen value.
When encoded, the least significant bit indicates the sign of the value, with
a `1` marking the value as negative. All other bits (once shifted to the right
by one) give the magnitude of the value. The only exception is `INT64_MIN`, whose magnitude would
overflow a 63 bit representation. It is instead encoded as negative zero (`0x01`).

This is done to efficiently encode small negative integers, which would otherwise always require
10 bytes after varlen encoding due to the set MSB in the twos-complement representation of negative
values.

### str
A varlen string. Encoded as-is. Must be the final value in the frame. Length given by end of frame.
