# Varlen Encoding

Many trace events feature large fields that often contain only a very small value. For example,
all interrupt-related events feature a 32-bit interrupt ID, but most microcontrollers will
at most have a few hundred interrupts. This would mean that most bytes of most numeric
fields are zero most of the time, wasting trace storage capacity or transfer bandwidth.

To combat this, Tonbandgerät uses the same specific form of variable-length (varlen) encoding that
is also by UTF-8 for most numeric values:

> Values are split into 7-bit septets, and are encoding starting with the least significant
> septet. Each septet is encoded as an 8-bit value, consisting of the septet in the lower bits,
> and a control bit in the most significant bit position that is set to `1` if there are more
> septets to follow, or `0` if this is the last septet and all following bits should be assumed
> to be zero.


For example, consider the 32-bit value `0x5`. With the scheme above, it is encoded as a single
byte:

```test
        +---> First 7 bits
     ___|___
    00000101
    |
    +--> No more bits to follow.
```

The value `0xFF` requires more than seven bits and therefor is split into two bytes:

```test
        +---> First 7 bits          +---> Next 7 bits
     ___|___                     ___|___
    11111111                    00000001
    |                           |
    +--> More bits to follow.   +--> No more bits to follow.
```

This system trades a much improved average message length for a longer worst-case 
message size.
