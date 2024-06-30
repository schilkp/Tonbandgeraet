#include "function_mocks.h"

DEFINE_FFF_GLOBALS

// tband_port.h:
// uint64_t mock_port_timestamp(void);
// #define tband_portTIMESTAMP() mock_port_timestamp()
DEFINE_FAKE_VALUE_FUNC(uint64_t, mock_port_timestamp)

// tband_port.h:
// void mock_port_enter_critical_from_any(void);
// #define tband_portKERNEL_ENTER_CRITICAL_FROM_ANY() mock_port_enter_critical_from_any()
DEFINE_FAKE_VOID_FUNC(mock_port_enter_critical_from_any)

// tband_port.h:
// void mock_port_exit_critical_from_any(void);
// #define tband_portKERNEL_EXIT_CRITICAL_FROM_ANY() mock_port_exit_critical_from_any()
DEFINE_FAKE_VOID_FUNC(mock_port_exit_critical_from_any)

// tband_port.h:
// bool mock_port_backend_stream_data(const uint8_t* buf, size_t len);
// #define tband_portBACKEND_STREAM_DATA(buf, len) mock_port_backend_stream_data(buf, len)
DEFINE_FAKE_VALUE_FUNC(bool, mock_port_backend_stream_data, const uint8_t *, size_t)

// portmacro.h:
// UBaseType_t mock_port_get_core_id(void);
// #define portGET_CORE_ID() mock_port_get_core_id()
DEFINE_FAKE_VALUE_FUNC(UBaseType_t, mock_port_get_core_id)

// task.h:
// TaskHandle_t xTaskGetIdleTaskHandle( void ) PRIVILEGED_FUNCTION;
DEFINE_FAKE_VALUE_FUNC(TaskHandle_t, xTaskGetIdleTaskHandle)

// task.h:
// UBaseType_t uxTaskGetTaskNumber( TaskHandle_t xTask ) PRIVILEGED_FUNCTION;
DEFINE_FAKE_VALUE_FUNC(UBaseType_t, uxTaskGetTaskNumber, TaskHandle_t)

// task.h:
// void vTaskSetTaskNumber( TaskHandle_t xTask, const UBaseType_t uxHandle ) PRIVILEGED_FUNCTION;
DEFINE_FAKE_VOID_FUNC(vTaskSetTaskNumber, TaskHandle_t, const UBaseType_t)

// timers.h:
// TaskHandle_t xTimerGetTimerDaemonTaskHandle( void ) PRIVILEGED_FUNCTION;
DEFINE_FAKE_VALUE_FUNC(UBaseType_t, xTimerGetTimerDaemonTaskHandle)

// queue.h:
// UBaseType_t uxQueueGetQueueNumber( QueueHandle_t xQueue ) PRIVILEGED_FUNCTION;
DEFINE_FAKE_VALUE_FUNC(UBaseType_t, uxQueueGetQueueNumber, QueueHandle_t)

// queue.h:
// void vQueueSetQueueNumber( QueueHandle_t xQueue, UBaseType_t uxQueueNumber ) PRIVILEGED_FUNCTION;
DEFINE_FAKE_VOID_FUNC(vQueueSetQueueNumber, QueueHandle_t, UBaseType_t)

// Mock backend:
// tband.c:
// extern bool tband_submit_to_backend(uint8_t *buf, size_t len, bool is_metadata);
#if (tband_configUSE_BACKEND_EXTERNAL == 1)
DEFINE_FAKE_VALUE_FUNC(bool, tband_submit_to_backend, uint8_t *, size_t, bool)
#endif /* (tband_configUSE_BACKEND_EXTERNAL == 1) */

void reset_stubs(void) {
  mock_port_timestamp_reset();
  mock_port_timestamp_fake.return_val = 0;

  mock_port_enter_critical_from_any_reset();

  mock_port_exit_critical_from_any_reset();

  mock_port_backend_stream_data_reset();
  mock_port_backend_stream_data_fake.return_val = false;

  mock_port_get_core_id_reset();
  mock_port_get_core_id_fake.return_val = 0;

  xTaskGetIdleTaskHandle_reset();
  xTaskGetIdleTaskHandle_fake.return_val = 0;

  uxTaskGetTaskNumber_reset();
  uxTaskGetTaskNumber_fake.return_val = 0;

  vTaskSetTaskNumber_reset();

  xTimerGetTimerDaemonTaskHandle_reset();
  xTimerGetTimerDaemonTaskHandle_fake.return_val = 0;

  uxQueueGetQueueNumber_reset();
  uxQueueGetQueueNumber_fake.return_val = 0;

  vQueueSetQueueNumber_reset();

#if (tband_configUSE_BACKEND_EXTERNAL == 1)
  tband_submit_to_backend_reset();
  tband_submit_to_backend_fake.return_val = false;
#endif /* (tband_configUSE_BACKEND_EXTERNAL == 1) */
}
