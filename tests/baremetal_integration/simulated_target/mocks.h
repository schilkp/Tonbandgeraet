#ifndef MOCKS_H_
#define MOCKS_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Mocking framework:
#include "fff.h"

// tband_port.h:
uint64_t mock_port_ts(void);
DECLARE_FAKE_VALUE_FUNC(uint64_t, mock_port_ts)

#endif /* MOCKS_H_ */
