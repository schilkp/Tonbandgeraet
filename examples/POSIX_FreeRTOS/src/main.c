#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "app.h"

#include "FreeRTOS.h"
#include "task.h"

//===----------------------------------------------------------------------===//
// Port Utils
//===----------------------------------------------------------------------===//

#ifdef CLOCK_MONOTONIC_RAW
// CLOCK_MONOTONIC_RAW is available
#define BEST_MONOTONIC_CLOCK CLOCK_MONOTONIC_RAW
#else
// Fall back to CLOCK_MONOTONIC
#define BEST_MONOTONIC_CLOCK CLOCK_MONOTONIC
#endif

uint64_t traceport_timestamp(void) {
  static struct timespec ts_start;
  static bool have_ts_start = false;
  if (!have_ts_start) {
    assert(clock_gettime(BEST_MONOTONIC_CLOCK, &ts_start) != -1);
    have_ts_start = true;
  }

  struct timespec ts;
  assert(clock_gettime(BEST_MONOTONIC_CLOCK, &ts) != -1);

  return (ts.tv_sec - ts_start.tv_sec) * 1000000000LL +
         (ts.tv_nsec - ts_start.tv_nsec);
}

void dump_hex(uint8_t b) {
  static size_t i = 0;
  if (i % 8 != 7) {
    printf("%02x ", b);
  } else {
    printf("%02x\n", b);
  }
  i++;
}

void traceport_snapshot_done(void) {
  const uint8_t *buf = (uint8_t *)tband_get_metadata_buf(0);
  size_t len = tband_get_metadata_buf_amnt(0);

  for (size_t i = 0; i < len; i++) {
    dump_hex(buf[i]);
  }

  buf = (uint8_t *)tband_get_core_snapshot_buf(0);
  len = tband_get_core_snapshot_buf_amnt(0);

  for (size_t i = 0; i < len; i++) {
    dump_hex(buf[i]);
  }

  printf("\n");

  exit(0);
}

//===----------------------------------------------------------------------===//
// Main Function
//===----------------------------------------------------------------------===//

int main() {
  assert(rtos_init() == 0);
  vTaskStartScheduler();
}
