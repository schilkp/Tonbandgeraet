/**
 * @file tband_port.h
 * @brief tband port testing stubs
 * @author Philipp Schilk, 2024
 */
#ifndef tband_port_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define tband_portTIMESTAMP_RESOLUTION_NS 1

uint64_t mock_port_timestamp(void);
#define tband_portTIMESTAMP() mock_port_timestamp()

void mock_port_enter_critical_from_any(void);
#define tband_portKERNEL_ENTER_CRITICAL_FROM_ANY() mock_port_enter_critical_from_any()

void mock_port_exit_critical_from_any(void);
#define tband_portKERNEL_EXIT_CRITICAL_FROM_ANY() mock_port_exit_critical_from_any()

bool mock_port_backend_stream_data(const uint8_t* buf, size_t len);
#define tband_portBACKEND_STREAM_DATA(buf, len) mock_port_backend_stream_data(buf, len)

#endif /* tband_port_H_ */
