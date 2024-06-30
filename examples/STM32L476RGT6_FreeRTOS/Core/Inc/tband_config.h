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

uint64_t traceport_timestamp(void);
#define tband_portTIMESTAMP()             traceport_timestamp()
#define tband_portTIMESTAMP_RESOLUTION_NS 1

// #define tband_configUSE_BACKEND_STREAMING 1
// bool traceport_handle_raw_evt(uint8_t *buf, size_t len);
// #define tband_portBACKEND_STREAM_DATA(_buf_, _len_) traceport_handle_raw_evt((_buf_), (_len_))

#define tband_configUSE_BACKEND_SNAPSHOT 1
extern volatile bool TRACING_SNAPSHOT_DONE;
void traceport_snapshot_done(void);
#define tband_portBACKEND_SNAPSHOT_DONE_CALLBACK() traceport_snapshot_done()

#endif /* TBAND_CONFIG_H_ */
