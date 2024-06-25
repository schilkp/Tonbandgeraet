#ifndef FUNCTION_MOCKS_H_
#define FUNCTION_MOCKS_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Mocking framework:
#include "fff.h"

// Types, Prototypes, etc:
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

// frtrace_port.h:
// uint64_t mock_port_timestamp(void);
// #define frtrace_portTIMESTAMP() mock_port_timestamp()
DECLARE_FAKE_VALUE_FUNC(uint64_t, mock_port_timestamp)

// frtrace_port.h:
// void mock_port_enter_critical_from_any(void);
// #define frtrace_portKERNEL_ENTER_CRITICAL_FROM_ANY() mock_port_enter_critical_from_any()
DECLARE_FAKE_VOID_FUNC(mock_port_enter_critical_from_any)

// frtrace_port.h:
// void mock_port_exit_critical_from_any(void);
// #define frtrace_portKERNEL_EXIT_CRITICAL_FROM_ANY() mock_port_exit_critical_from_any()
DECLARE_FAKE_VOID_FUNC(mock_port_exit_critical_from_any)

// frtrace_port.h:
// bool mock_port_backend_stream_data(const uint8_t* buf, size_t len);
// #define frtrace_portBACKEND_STREAM_DATA(buf, len) mock_port_backend_stream_data(buf, len)
DECLARE_FAKE_VALUE_FUNC(bool, mock_port_backend_stream_data, const uint8_t *, size_t)

// portmacro.h:
// UBaseType_t mock_port_get_core_id(void);
// #define portGET_CORE_ID() mock_port_get_core_id()
DECLARE_FAKE_VALUE_FUNC(UBaseType_t, mock_port_get_core_id)

// task.h:
// TaskHandle_t xTaskGetIdleTaskHandle( void ) PRIVILEGED_FUNCTION;
DECLARE_FAKE_VALUE_FUNC(TaskHandle_t, xTaskGetIdleTaskHandle)

// task.h:
// UBaseType_t uxTaskGetTaskNumber( TaskHandle_t xTask ) PRIVILEGED_FUNCTION;
DECLARE_FAKE_VALUE_FUNC(UBaseType_t, uxTaskGetTaskNumber, TaskHandle_t)

// task.h:
// void vTaskSetTaskNumber( TaskHandle_t xTask, const UBaseType_t uxHandle ) PRIVILEGED_FUNCTION;
DECLARE_FAKE_VOID_FUNC(vTaskSetTaskNumber, TaskHandle_t, const UBaseType_t)

// timers.h:
// TaskHandle_t xTimerGetTimerDaemonTaskHandle( void ) PRIVILEGED_FUNCTION;
DECLARE_FAKE_VALUE_FUNC(UBaseType_t, xTimerGetTimerDaemonTaskHandle)

// queue.h:
// UBaseType_t uxQueueGetQueueNumber( QueueHandle_t xQueue ) PRIVILEGED_FUNCTION;
DECLARE_FAKE_VALUE_FUNC(UBaseType_t, uxQueueGetQueueNumber, QueueHandle_t)

// queue.h:
// void vQueueSetQueueNumber( QueueHandle_t xQueue, UBaseType_t uxQueueNumber ) PRIVILEGED_FUNCTION;
DECLARE_FAKE_VOID_FUNC(vQueueSetQueueNumber, QueueHandle_t, UBaseType_t)

// Mock backend:
// frtrace.c:
// extern bool frtrace_submit_to_backend(uint8_t *buf, size_t len, bool is_metadata);
#if (frtrace_configUSE_BACKEND_EXTERNAL == 1)
DECLARE_FAKE_VALUE_FUNC(bool, frtrace_submit_to_backend, uint8_t *, size_t, bool)
#endif /* (frtrace_configUSE_BACKEND_EXTERNAL == 1) */

void reset_stubs(void);

#endif /* FUNCTION_MOCKS_H_ */
