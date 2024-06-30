#ifndef TBAND_CONFIG_H_
#define TBAND_CONFIG_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Tracing port:
#define tband_configENABLE                1
#define tband_configISR_TRACE_ENABLE      1
#define tband_configFREERTOS_TRACE_ENABLE 1
#define tband_configTRACE_DROP_CNT_EVERY 100

#define tband_configUSE_BACKEND_SNAPSHOT 1
// extern volatile bool TRACING_SNAPSHOT_DONE;
// void traceport_snapshot_done(void);
// #define tband_portBACKEND_SNAPSHOT_DONE_CALLBACK() traceport_snapshot_done()

#endif /* TBAND_CONFIG_H_ */
