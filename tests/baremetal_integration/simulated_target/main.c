#include <assert.h>
#include <stdio.h>

#include "mocks.h"
#include "tband.h"

int main() {

  mock_port_ts_reset();
  mock_port_ts_fake.return_val = 0;

  // Generate all metadata before starting snapshot to exercise metadata buffer:
  tband_gather_system_metadata();

  tband_isr_name(0, "ISR0");
  tband_isr_name(1, "ISR1");

  tband_evtmarker_name(0, "EVTM0");

  tband_valmarker_name(0, "VALM0");

  assert(tband_trigger_snapshot() == 0);

  mock_port_ts_fake.return_val = 10;
  tband_isr_enter(0);

  mock_port_ts_fake.return_val = 12;
  tband_isr_enter(1);

  mock_port_ts_fake.return_val = 14;
  tband_isr_exit(1);

  mock_port_ts_fake.return_val = 16;
  tband_isr_exit(0);

  mock_port_ts_fake.return_val = 20;
  tband_evtmarker(0, "EVENT@20");

  mock_port_ts_fake.return_val = 30;
  tband_evtmarker_begin(0, "EVENT@30");

  mock_port_ts_fake.return_val = 40;
  tband_evtmarker_end(0);

  assert(tband_tracing_finished() != true);

  assert(tband_stop_snapshot() == 0);

  FILE *ptr = fopen("trace.bin", "wb");
  assert(ptr != 0);

  const uint8_t *buf = (void *)tband_get_metadata_buf(0);
  size_t len = tband_get_metadata_buf_amnt(0);
  assert(fwrite(buf, 1, len, ptr) == len);

  buf = (void *)tband_get_core_snapshot_buf(0);
  len = tband_get_core_snapshot_buf_amnt(0);
  assert(fwrite(buf, 1, len, ptr) == len);

  fclose(ptr);

  return 0;
}
