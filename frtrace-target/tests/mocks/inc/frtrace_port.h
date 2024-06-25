/**
 * @file frtrace_port.h
 * @brief frtrace port testing stubs
 * @author Philipp Schilk, 2024
 */
#ifndef FRTRACE_PORT_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define frtrace_portTIMESTAMP_RESOLUTION_NS 1

uint64_t mock_port_timestamp(void);
#define frtrace_portTIMESTAMP() mock_port_timestamp()

void mock_port_enter_critical_from_any(void);
#define frtrace_portKERNEL_ENTER_CRITICAL_FROM_ANY() mock_port_enter_critical_from_any()

void mock_port_exit_critical_from_any(void);
#define frtrace_portKERNEL_EXIT_CRITICAL_FROM_ANY() mock_port_exit_critical_from_any()

bool mock_port_backend_stream_data(const uint8_t* buf, size_t len);
#define frtrace_portBACKEND_STREAM_DATA(buf, len) mock_port_backend_stream_data(buf, len)

#endif /* FRTRACE_PORT_H_ */
