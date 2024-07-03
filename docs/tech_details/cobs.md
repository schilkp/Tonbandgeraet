# COBS Framing

The tracer uses COBS (Consistent Overhead Byte Stuffing, [wikipedia](https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing)) framing to
separate binary trace events that have been stored or transmitted together. Specifically, the COBS algorithm removes all zeroes from a binary message
in a reversible fashion, with only minimal overhead. Zeroes are then used to delimit individual trace messages.

Specifically, after COBS framing, an \\( N \neq 0 \\) byte message will be at most
\\( 1 + \left\lceil \frac{N}{254} \right\rceil + N \\) bytes long, including a trailing zero for delimination.

Please read the above article for a more precise specification, but roughly speaking this is done by replacing all zeroes with
a pointer to the next zero:

```text
Original values:          0x01  0x02  0x00  0x04  0x00  0x05

                      +-----> +3 -----+  +-> +2 --+  +-> +2 --+
                      |               |  |        |  |        |
COBS framed:       0x03   0x01  0x02  0x02  0x04  0x02  0x05  0x00

                   Start  Data  Data  Zero  Data  Zero  Data  Delim.

```

Special care has to be given to a run of 254 or more consecutive non-zero bytes, as an 8-bit pointer is not sufficient
to point beyond such a run. In this case, an additional pointer byte is added that does not correspond to a zero in the
original data:

```text
Original values:          0x00  0x01  0x02 ... 0xFD  0xFE         0xFF

                      +->-+  +----------> +255 ------------+  +--> +2 --+
                      |   |  |                             |  |         |
COBS framed:       0x01   0xFF  0x01  0x02 ... 0xFD  0xFE  0x02   0xFF  0x00

                  Start   Zero  Data  Zero ... Data  Zero  Extra  Data  Delim.

```

When decoding, the value pointed at by a `0xFF` pointer must therefor not be decoded to a zero but only be interpreted as a 
another pointer.

Because trace events are usually very short, this means that most can be framed with only two bytes of ove11jrhead.
