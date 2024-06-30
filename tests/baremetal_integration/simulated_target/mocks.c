#include "mocks.h"

DEFINE_FFF_GLOBALS

// tband_port.h:
uint64_t mock_port_ts(void);
DEFINE_FAKE_VALUE_FUNC(uint64_t, mock_port_ts)
