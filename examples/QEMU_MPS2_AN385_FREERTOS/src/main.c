/**
 * @file main.c
 * @brief Tracing port and entry point for the QEMU "mps2-an385" (Cortex-M3) example.
 * @author Philipp Schilk, 2024-2026
 *
 * The trace snapshot is written out through ARM semihosting calls.
 */
#include "app.h"
#include "semihosting.h"

#include <stdatomic.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

//===----------------------------------------------------------------------===//
// Port Utils
//===----------------------------------------------------------------------===//

// For demo purposes: Monotonic fake timestamp.
static atomic_uint_least32_t ts_counter = 0;

uint64_t traceport_timestamp(void) {
  return (uint64_t)atomic_fetch_add_explicit(&ts_counter, 1, memory_order_relaxed);
}

void traceport_snapshot_done(void) {
  int32_t handle = semihosting_open("trace.bin", 5 /* "wb" */);
  if (handle < 0) semihosting_exit(1);

  const uint8_t *buf = (void *)tband_get_metadata_buf(0);
  uint32_t len = (uint32_t)tband_get_metadata_buf_amnt(0);
  if (semihosting_write(handle, buf, len) != 0) semihosting_exit(1);

  buf = (void *)tband_get_core_snapshot_buf(0);
  len = (uint32_t)tband_get_core_snapshot_buf_amnt(0);
  if (semihosting_write(handle, buf, len) != 0) semihosting_exit(1);

  semihosting_close(handle);

  semihosting_exit(0);
}

//===----------------------------------------------------------------------===//
// Main Function
//===----------------------------------------------------------------------===//

int main(void) {
  if (rtos_init() != 0) semihosting_exit(1);
  vTaskStartScheduler();

  // Only reached if the scheduler could not start.
  semihosting_exit(1);
}
