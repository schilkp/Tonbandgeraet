#ifndef TBAND_PORT_MOCKS_H_
#define TBAND_PORT_MOCKS_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Mocking framework:
#include "fff.h"

// tband_port.h:
// uint64_t mock_port_timestamp(void);
// #define tband_portTIMESTAMP() mock_port_timestamp()
DECLARE_FAKE_VALUE_FUNC(uint64_t, mock_port_timestamp)

// tband_port.h:
// void mock_port_enter_critical_from_any(void);
// #define tband_portKERNEL_ENTER_CRITICAL_FROM_ANY() mock_port_enter_critical_from_any()
DECLARE_FAKE_VOID_FUNC(mock_port_enter_critical_from_any)

// tband_port.h:
// void mock_port_exit_critical_from_any(void);
// #define tband_portKERNEL_EXIT_CRITICAL_FROM_ANY() mock_port_exit_critical_from_any()
DECLARE_FAKE_VOID_FUNC(mock_port_exit_critical_from_any)

// tband_port.h:
// bool mock_port_backend_stream_data(const uint8_t* buf, size_t len);
// #define tband_portBACKEND_STREAM_DATA(buf, len) mock_port_backend_stream_data(buf, len)
DECLARE_FAKE_VALUE_FUNC(bool, mock_port_backend_stream_data, const uint8_t *, size_t)

// Mock backend:
// tband.c:
// extern bool tband_submit_to_backend(uint8_t *buf, size_t len, bool is_metadata);
#if (tband_configUSE_BACKEND_EXTERNAL == 1)
DECLARE_FAKE_VALUE_FUNC(bool, tband_submit_to_backend, uint8_t *, size_t, bool)
#endif /* (tband_configUSE_BACKEND_EXTERNAL == 1) */

void reset_tband_port_mocks(void);

#endif /* TBAND_PORT_MOCKS_H_ */
