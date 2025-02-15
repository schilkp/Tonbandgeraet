/**
 * @file test.c
 * @brief Encoding function unit tests.
 * @warning This file is generated. Do not edit. See `code_gen` folder in repo.
 *
 *         ____   ___    _   _  ___ _____   _____ ____ ___ _____
 *        |  _ \ / _ \  | \ | |/ _ \_   _| | ____|  _ \_ _|_   _|
 *        | | | | | | | |  \| | | | || |   |  _| | | | | |  | |
 *        | |_| | |_| | | |\  | |_| || |   | |___| |_| | |  | |
 *        |____/ \___/  |_| \_|\___/ |_|   |_____|____/___| |_|
 *
 * Note that the main purpose of this test suite is to exercises the generated
 * functions while they run under various sanititizers.
 */
#include <string.h>

#include "unity.h"
#include "unity_internals.h"

#include "tband.h"
#define tbandPROPER_INTERNAL_INCLUDE
#include "tband_encode.h"
#undef tbandPROPER_INTERNAL_INCLUDE

void util_print_array(uint8_t *in, size_t in_len, char *name) {
  printf("  %s: ", name);
  for (size_t i = 0; i < in_len; i++) {
    printf("0x%02x ", in[i]);
  }
  printf("\r\n");
}

void compare_arrays(uint8_t *in, size_t in_len, uint8_t *expect,
                    size_t expect_len, char *msg) {
  // COBS Frame expected value:
  size_t expected_framed_len = COBS_MAXLEN(expect_len);
  uint8_t expected_framed[expected_framed_len];
  memset(expected_framed, 0xAB, expected_framed_len);
  struct cobs_state cobs = cobs_start(expected_framed);
  for (size_t i = 0; i < expect_len; i++) {
    cobs_add_byte(&cobs, expect[i]);
  }
  expected_framed_len = cobs_finish(&cobs);

  printf("%s: \r\n", msg);
  util_print_array(in, in_len, "IS");
  util_print_array(expected_framed, expected_framed_len, "EXPECTED");

  TEST_ASSERT_EQUAL_INT(expected_framed_len, in_len);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(expected_framed, in, expect_len);
}

// ==== Base Encoder Tests =============================================================================================

void test_core_id(void){
  {
    // Min
    uint8_t buf[EVT_CORE_ID_MAXLEN] = {0};
    size_t len = encode_core_id(buf, 0x0, 0x0);
    uint8_t expected[] = {0x0, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_CORE_ID_MAXLEN] = {0};
    size_t len = encode_core_id(buf, UINT64_MAX, UINT32_MAX);
    uint8_t expected[] = {0x0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_dropped_evt_cnt(void){
  {
    // Min
    uint8_t buf[EVT_DROPPED_EVT_CNT_MAXLEN] = {0};
    size_t len = encode_dropped_evt_cnt(buf, 0x0, 0x0);
    uint8_t expected[] = {0x1, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_DROPPED_EVT_CNT_MAXLEN] = {0};
    size_t len = encode_dropped_evt_cnt(buf, UINT64_MAX, UINT32_MAX);
    uint8_t expected[] = {0x1, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_ts_resolution_ns(void){
  {
    // Min
    uint8_t buf[EVT_TS_RESOLUTION_NS_MAXLEN] = {0};
    size_t len = encode_ts_resolution_ns(buf, 0x0);
    uint8_t expected[] = {0x2, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_TS_RESOLUTION_NS_MAXLEN] = {0};
    size_t len = encode_ts_resolution_ns(buf, UINT64_MAX);
    uint8_t expected[] = {0x2, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_isr_name(void){
  {
    // Min
    uint8_t buf[EVT_ISR_NAME_MAXLEN] = {0};
    size_t len = encode_isr_name(buf, 0x0, "test");
    uint8_t expected[] = {0x3, 0x0, 0x74, 0x65, 0x73, 0x74};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_ISR_NAME_MAXLEN] = {0};
    size_t len = encode_isr_name(buf, UINT32_MAX, "test");
    uint8_t expected[] = {0x3, 0xff, 0xff, 0xff, 0xff, 0xf, 0x74, 0x65, 0x73, 0x74};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_isr_enter(void){
  {
    // Min
    uint8_t buf[EVT_ISR_ENTER_MAXLEN] = {0};
    size_t len = encode_isr_enter(buf, 0x0, 0x0);
    uint8_t expected[] = {0x4, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_ISR_ENTER_MAXLEN] = {0};
    size_t len = encode_isr_enter(buf, UINT64_MAX, UINT32_MAX);
    uint8_t expected[] = {0x4, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_isr_exit(void){
  {
    // Min
    uint8_t buf[EVT_ISR_EXIT_MAXLEN] = {0};
    size_t len = encode_isr_exit(buf, 0x0, 0x0);
    uint8_t expected[] = {0x5, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_ISR_EXIT_MAXLEN] = {0};
    size_t len = encode_isr_exit(buf, UINT64_MAX, UINT32_MAX);
    uint8_t expected[] = {0x5, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_evtmarker_name(void){
  {
    // Min
    uint8_t buf[EVT_EVTMARKER_NAME_MAXLEN] = {0};
    size_t len = encode_evtmarker_name(buf, 0x0, "test");
    uint8_t expected[] = {0x6, 0x0, 0x74, 0x65, 0x73, 0x74};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_EVTMARKER_NAME_MAXLEN] = {0};
    size_t len = encode_evtmarker_name(buf, UINT32_MAX, "test");
    uint8_t expected[] = {0x6, 0xff, 0xff, 0xff, 0xff, 0xf, 0x74, 0x65, 0x73, 0x74};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_evtmarker(void){
  {
    // Min
    uint8_t buf[EVT_EVTMARKER_MAXLEN] = {0};
    size_t len = encode_evtmarker(buf, 0x0, 0x0, "test");
    uint8_t expected[] = {0x7, 0x0, 0x0, 0x74, 0x65, 0x73, 0x74};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_EVTMARKER_MAXLEN] = {0};
    size_t len = encode_evtmarker(buf, UINT64_MAX, UINT32_MAX, "test");
    uint8_t expected[] = {0x7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf, 0x74, 0x65, 0x73, 0x74};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_evtmarker_begin(void){
  {
    // Min
    uint8_t buf[EVT_EVTMARKER_BEGIN_MAXLEN] = {0};
    size_t len = encode_evtmarker_begin(buf, 0x0, 0x0, "test");
    uint8_t expected[] = {0x8, 0x0, 0x0, 0x74, 0x65, 0x73, 0x74};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_EVTMARKER_BEGIN_MAXLEN] = {0};
    size_t len = encode_evtmarker_begin(buf, UINT64_MAX, UINT32_MAX, "test");
    uint8_t expected[] = {0x8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf, 0x74, 0x65, 0x73, 0x74};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_evtmarker_end(void){
  {
    // Min
    uint8_t buf[EVT_EVTMARKER_END_MAXLEN] = {0};
    size_t len = encode_evtmarker_end(buf, 0x0, 0x0);
    uint8_t expected[] = {0x9, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_EVTMARKER_END_MAXLEN] = {0};
    size_t len = encode_evtmarker_end(buf, UINT64_MAX, UINT32_MAX);
    uint8_t expected[] = {0x9, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_valmarker_name(void){
  {
    // Min
    uint8_t buf[EVT_VALMARKER_NAME_MAXLEN] = {0};
    size_t len = encode_valmarker_name(buf, 0x0, "test");
    uint8_t expected[] = {0xa, 0x0, 0x74, 0x65, 0x73, 0x74};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_VALMARKER_NAME_MAXLEN] = {0};
    size_t len = encode_valmarker_name(buf, UINT32_MAX, "test");
    uint8_t expected[] = {0xa, 0xff, 0xff, 0xff, 0xff, 0xf, 0x74, 0x65, 0x73, 0x74};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_valmarker(void){
  {
    // Min
    uint8_t buf[EVT_VALMARKER_MAXLEN] = {0};
    size_t len = encode_valmarker(buf, 0x0, 0x0, 0x0);
    uint8_t expected[] = {0xb, 0x0, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_VALMARKER_MAXLEN] = {0};
    size_t len = encode_valmarker(buf, UINT64_MAX, UINT32_MAX, INT64_MIN + 1);
    uint8_t expected[] = {0xb, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

// ==== FreeRTOS Encoder Tests =========================================================================================

void test_freertos_task_switched_in(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_TASK_SWITCHED_IN_MAXLEN] = {0};
    size_t len = encode_freertos_task_switched_in(buf, 0x0, 0x0);
    uint8_t expected[] = {0x54, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_TASK_SWITCHED_IN_MAXLEN] = {0};
    size_t len = encode_freertos_task_switched_in(buf, UINT64_MAX, UINT32_MAX);
    uint8_t expected[] = {0x54, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_task_to_rdy_state(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_TASK_TO_RDY_STATE_MAXLEN] = {0};
    size_t len = encode_freertos_task_to_rdy_state(buf, 0x0, 0x0);
    uint8_t expected[] = {0x55, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_TASK_TO_RDY_STATE_MAXLEN] = {0};
    size_t len = encode_freertos_task_to_rdy_state(buf, UINT64_MAX, UINT32_MAX);
    uint8_t expected[] = {0x55, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_task_resumed(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_TASK_RESUMED_MAXLEN] = {0};
    size_t len = encode_freertos_task_resumed(buf, 0x0, 0x0);
    uint8_t expected[] = {0x56, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_TASK_RESUMED_MAXLEN] = {0};
    size_t len = encode_freertos_task_resumed(buf, UINT64_MAX, UINT32_MAX);
    uint8_t expected[] = {0x56, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_task_resumed_from_isr(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_TASK_RESUMED_FROM_ISR_MAXLEN] = {0};
    size_t len = encode_freertos_task_resumed_from_isr(buf, 0x0, 0x0);
    uint8_t expected[] = {0x57, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_TASK_RESUMED_FROM_ISR_MAXLEN] = {0};
    size_t len = encode_freertos_task_resumed_from_isr(buf, UINT64_MAX, UINT32_MAX);
    uint8_t expected[] = {0x57, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_task_suspended(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_TASK_SUSPENDED_MAXLEN] = {0};
    size_t len = encode_freertos_task_suspended(buf, 0x0, 0x0);
    uint8_t expected[] = {0x58, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_TASK_SUSPENDED_MAXLEN] = {0};
    size_t len = encode_freertos_task_suspended(buf, UINT64_MAX, UINT32_MAX);
    uint8_t expected[] = {0x58, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_curtask_delay(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_CURTASK_DELAY_MAXLEN] = {0};
    size_t len = encode_freertos_curtask_delay(buf, 0x0, 0x0);
    uint8_t expected[] = {0x59, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_CURTASK_DELAY_MAXLEN] = {0};
    size_t len = encode_freertos_curtask_delay(buf, UINT64_MAX, UINT32_MAX);
    uint8_t expected[] = {0x59, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_curtask_delay_until(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_CURTASK_DELAY_UNTIL_MAXLEN] = {0};
    size_t len = encode_freertos_curtask_delay_until(buf, 0x0, 0x0);
    uint8_t expected[] = {0x5a, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_CURTASK_DELAY_UNTIL_MAXLEN] = {0};
    size_t len = encode_freertos_curtask_delay_until(buf, UINT64_MAX, UINT32_MAX);
    uint8_t expected[] = {0x5a, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_task_priority_set(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_TASK_PRIORITY_SET_MAXLEN] = {0};
    size_t len = encode_freertos_task_priority_set(buf, 0x0, 0x0, 0x0);
    uint8_t expected[] = {0x5b, 0x0, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_TASK_PRIORITY_SET_MAXLEN] = {0};
    size_t len = encode_freertos_task_priority_set(buf, UINT64_MAX, UINT32_MAX, UINT32_MAX);
    uint8_t expected[] = {0x5b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_task_priority_inherit(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_TASK_PRIORITY_INHERIT_MAXLEN] = {0};
    size_t len = encode_freertos_task_priority_inherit(buf, 0x0, 0x0, 0x0);
    uint8_t expected[] = {0x5c, 0x0, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_TASK_PRIORITY_INHERIT_MAXLEN] = {0};
    size_t len = encode_freertos_task_priority_inherit(buf, UINT64_MAX, UINT32_MAX, UINT32_MAX);
    uint8_t expected[] = {0x5c, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_task_priority_disinherit(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_TASK_PRIORITY_DISINHERIT_MAXLEN] = {0};
    size_t len = encode_freertos_task_priority_disinherit(buf, 0x0, 0x0, 0x0);
    uint8_t expected[] = {0x5d, 0x0, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_TASK_PRIORITY_DISINHERIT_MAXLEN] = {0};
    size_t len = encode_freertos_task_priority_disinherit(buf, UINT64_MAX, UINT32_MAX, UINT32_MAX);
    uint8_t expected[] = {0x5d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_task_created(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_TASK_CREATED_MAXLEN] = {0};
    size_t len = encode_freertos_task_created(buf, 0x0, 0x0);
    uint8_t expected[] = {0x5e, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_TASK_CREATED_MAXLEN] = {0};
    size_t len = encode_freertos_task_created(buf, UINT64_MAX, UINT32_MAX);
    uint8_t expected[] = {0x5e, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_task_name(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_TASK_NAME_MAXLEN] = {0};
    size_t len = encode_freertos_task_name(buf, 0x0, "test");
    uint8_t expected[] = {0x5f, 0x0, 0x74, 0x65, 0x73, 0x74};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_TASK_NAME_MAXLEN] = {0};
    size_t len = encode_freertos_task_name(buf, UINT32_MAX, "test");
    uint8_t expected[] = {0x5f, 0xff, 0xff, 0xff, 0xff, 0xf, 0x74, 0x65, 0x73, 0x74};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_task_is_idle_task(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_TASK_IS_IDLE_TASK_MAXLEN] = {0};
    size_t len = encode_freertos_task_is_idle_task(buf, 0x0, 0x0);
    uint8_t expected[] = {0x60, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_TASK_IS_IDLE_TASK_MAXLEN] = {0};
    size_t len = encode_freertos_task_is_idle_task(buf, UINT32_MAX, UINT32_MAX);
    uint8_t expected[] = {0x60, 0xff, 0xff, 0xff, 0xff, 0xf, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_task_is_timer_task(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_TASK_IS_TIMER_TASK_MAXLEN] = {0};
    size_t len = encode_freertos_task_is_timer_task(buf, 0x0);
    uint8_t expected[] = {0x61, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_TASK_IS_TIMER_TASK_MAXLEN] = {0};
    size_t len = encode_freertos_task_is_timer_task(buf, UINT32_MAX);
    uint8_t expected[] = {0x61, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_task_deleted(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_TASK_DELETED_MAXLEN] = {0};
    size_t len = encode_freertos_task_deleted(buf, 0x0, 0x0);
    uint8_t expected[] = {0x62, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_TASK_DELETED_MAXLEN] = {0};
    size_t len = encode_freertos_task_deleted(buf, UINT64_MAX, UINT32_MAX);
    uint8_t expected[] = {0x62, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_queue_created(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_QUEUE_CREATED_MAXLEN] = {0};
    size_t len = encode_freertos_queue_created(buf, 0x0, 0x0);
    uint8_t expected[] = {0x63, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_QUEUE_CREATED_MAXLEN] = {0};
    size_t len = encode_freertos_queue_created(buf, UINT64_MAX, UINT32_MAX);
    uint8_t expected[] = {0x63, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_queue_name(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_QUEUE_NAME_MAXLEN] = {0};
    size_t len = encode_freertos_queue_name(buf, 0x0, "test");
    uint8_t expected[] = {0x64, 0x0, 0x74, 0x65, 0x73, 0x74};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_QUEUE_NAME_MAXLEN] = {0};
    size_t len = encode_freertos_queue_name(buf, UINT32_MAX, "test");
    uint8_t expected[] = {0x64, 0xff, 0xff, 0xff, 0xff, 0xf, 0x74, 0x65, 0x73, 0x74};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_queue_kind(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_QUEUE_KIND_MAXLEN] = {0};
    size_t len = encode_freertos_queue_kind(buf, 0x0, 0x0);
    uint8_t expected[] = {0x65, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_QUEUE_KIND_MAXLEN] = {0};
    size_t len = encode_freertos_queue_kind(buf, UINT32_MAX, UINT8_MAX);
    uint8_t expected[] = {0x65, 0xff, 0xff, 0xff, 0xff, 0xf, 0xff};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_queue_send(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_QUEUE_SEND_MAXLEN] = {0};
    size_t len = encode_freertos_queue_send(buf, 0x0, 0x0, 0x0);
    uint8_t expected[] = {0x66, 0x0, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_QUEUE_SEND_MAXLEN] = {0};
    size_t len = encode_freertos_queue_send(buf, UINT64_MAX, UINT32_MAX, UINT32_MAX);
    uint8_t expected[] = {0x66, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_queue_send_from_isr(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_QUEUE_SEND_FROM_ISR_MAXLEN] = {0};
    size_t len = encode_freertos_queue_send_from_isr(buf, 0x0, 0x0, 0x0);
    uint8_t expected[] = {0x67, 0x0, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_QUEUE_SEND_FROM_ISR_MAXLEN] = {0};
    size_t len = encode_freertos_queue_send_from_isr(buf, UINT64_MAX, UINT32_MAX, UINT32_MAX);
    uint8_t expected[] = {0x67, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_queue_overwrite(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_QUEUE_OVERWRITE_MAXLEN] = {0};
    size_t len = encode_freertos_queue_overwrite(buf, 0x0, 0x0, 0x0);
    uint8_t expected[] = {0x68, 0x0, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_QUEUE_OVERWRITE_MAXLEN] = {0};
    size_t len = encode_freertos_queue_overwrite(buf, UINT64_MAX, UINT32_MAX, UINT32_MAX);
    uint8_t expected[] = {0x68, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_queue_overwrite_from_isr(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_QUEUE_OVERWRITE_FROM_ISR_MAXLEN] = {0};
    size_t len = encode_freertos_queue_overwrite_from_isr(buf, 0x0, 0x0, 0x0);
    uint8_t expected[] = {0x69, 0x0, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_QUEUE_OVERWRITE_FROM_ISR_MAXLEN] = {0};
    size_t len = encode_freertos_queue_overwrite_from_isr(buf, UINT64_MAX, UINT32_MAX, UINT32_MAX);
    uint8_t expected[] = {0x69, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_queue_receive(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_QUEUE_RECEIVE_MAXLEN] = {0};
    size_t len = encode_freertos_queue_receive(buf, 0x0, 0x0, 0x0);
    uint8_t expected[] = {0x6a, 0x0, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_QUEUE_RECEIVE_MAXLEN] = {0};
    size_t len = encode_freertos_queue_receive(buf, UINT64_MAX, UINT32_MAX, UINT32_MAX);
    uint8_t expected[] = {0x6a, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_queue_receive_from_isr(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_QUEUE_RECEIVE_FROM_ISR_MAXLEN] = {0};
    size_t len = encode_freertos_queue_receive_from_isr(buf, 0x0, 0x0, 0x0);
    uint8_t expected[] = {0x6b, 0x0, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_QUEUE_RECEIVE_FROM_ISR_MAXLEN] = {0};
    size_t len = encode_freertos_queue_receive_from_isr(buf, UINT64_MAX, UINT32_MAX, UINT32_MAX);
    uint8_t expected[] = {0x6b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_queue_reset(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_QUEUE_RESET_MAXLEN] = {0};
    size_t len = encode_freertos_queue_reset(buf, 0x0, 0x0);
    uint8_t expected[] = {0x6c, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_QUEUE_RESET_MAXLEN] = {0};
    size_t len = encode_freertos_queue_reset(buf, UINT64_MAX, UINT32_MAX);
    uint8_t expected[] = {0x6c, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_curtask_block_on_queue_peek(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_CURTASK_BLOCK_ON_QUEUE_PEEK_MAXLEN] = {0};
    size_t len = encode_freertos_curtask_block_on_queue_peek(buf, 0x0, 0x0, 0x0);
    uint8_t expected[] = {0x6d, 0x0, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_CURTASK_BLOCK_ON_QUEUE_PEEK_MAXLEN] = {0};
    size_t len = encode_freertos_curtask_block_on_queue_peek(buf, UINT64_MAX, UINT32_MAX, UINT32_MAX);
    uint8_t expected[] = {0x6d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_curtask_block_on_queue_send(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_CURTASK_BLOCK_ON_QUEUE_SEND_MAXLEN] = {0};
    size_t len = encode_freertos_curtask_block_on_queue_send(buf, 0x0, 0x0, 0x0);
    uint8_t expected[] = {0x6e, 0x0, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_CURTASK_BLOCK_ON_QUEUE_SEND_MAXLEN] = {0};
    size_t len = encode_freertos_curtask_block_on_queue_send(buf, UINT64_MAX, UINT32_MAX, UINT32_MAX);
    uint8_t expected[] = {0x6e, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_curtask_block_on_queue_receive(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_CURTASK_BLOCK_ON_QUEUE_RECEIVE_MAXLEN] = {0};
    size_t len = encode_freertos_curtask_block_on_queue_receive(buf, 0x0, 0x0, 0x0);
    uint8_t expected[] = {0x6f, 0x0, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_CURTASK_BLOCK_ON_QUEUE_RECEIVE_MAXLEN] = {0};
    size_t len = encode_freertos_curtask_block_on_queue_receive(buf, UINT64_MAX, UINT32_MAX, UINT32_MAX);
    uint8_t expected[] = {0x6f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_queue_cur_length(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_QUEUE_CUR_LENGTH_MAXLEN] = {0};
    size_t len = encode_freertos_queue_cur_length(buf, 0x0, 0x0, 0x0);
    uint8_t expected[] = {0x70, 0x0, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_QUEUE_CUR_LENGTH_MAXLEN] = {0};
    size_t len = encode_freertos_queue_cur_length(buf, UINT64_MAX, UINT32_MAX, UINT32_MAX);
    uint8_t expected[] = {0x70, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_task_evtmarker_name(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_TASK_EVTMARKER_NAME_MAXLEN] = {0};
    size_t len = encode_freertos_task_evtmarker_name(buf, 0x0, 0x0, "test");
    uint8_t expected[] = {0x7a, 0x0, 0x0, 0x74, 0x65, 0x73, 0x74};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_TASK_EVTMARKER_NAME_MAXLEN] = {0};
    size_t len = encode_freertos_task_evtmarker_name(buf, UINT32_MAX, UINT32_MAX, "test");
    uint8_t expected[] = {0x7a, 0xff, 0xff, 0xff, 0xff, 0xf, 0xff, 0xff, 0xff, 0xff, 0xf, 0x74, 0x65, 0x73, 0x74};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_task_evtmarker(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_TASK_EVTMARKER_MAXLEN] = {0};
    size_t len = encode_freertos_task_evtmarker(buf, 0x0, 0x0, "test");
    uint8_t expected[] = {0x7b, 0x0, 0x0, 0x74, 0x65, 0x73, 0x74};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_TASK_EVTMARKER_MAXLEN] = {0};
    size_t len = encode_freertos_task_evtmarker(buf, UINT64_MAX, UINT32_MAX, "test");
    uint8_t expected[] = {0x7b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf, 0x74, 0x65, 0x73, 0x74};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_task_evtmarker_begin(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_TASK_EVTMARKER_BEGIN_MAXLEN] = {0};
    size_t len = encode_freertos_task_evtmarker_begin(buf, 0x0, 0x0, "test");
    uint8_t expected[] = {0x7c, 0x0, 0x0, 0x74, 0x65, 0x73, 0x74};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_TASK_EVTMARKER_BEGIN_MAXLEN] = {0};
    size_t len = encode_freertos_task_evtmarker_begin(buf, UINT64_MAX, UINT32_MAX, "test");
    uint8_t expected[] = {0x7c, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf, 0x74, 0x65, 0x73, 0x74};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_task_evtmarker_end(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_TASK_EVTMARKER_END_MAXLEN] = {0};
    size_t len = encode_freertos_task_evtmarker_end(buf, 0x0, 0x0);
    uint8_t expected[] = {0x7d, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_TASK_EVTMARKER_END_MAXLEN] = {0};
    size_t len = encode_freertos_task_evtmarker_end(buf, UINT64_MAX, UINT32_MAX);
    uint8_t expected[] = {0x7d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_task_valmarker_name(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_TASK_VALMARKER_NAME_MAXLEN] = {0};
    size_t len = encode_freertos_task_valmarker_name(buf, 0x0, 0x0, "test");
    uint8_t expected[] = {0x7e, 0x0, 0x0, 0x74, 0x65, 0x73, 0x74};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_TASK_VALMARKER_NAME_MAXLEN] = {0};
    size_t len = encode_freertos_task_valmarker_name(buf, UINT32_MAX, UINT32_MAX, "test");
    uint8_t expected[] = {0x7e, 0xff, 0xff, 0xff, 0xff, 0xf, 0xff, 0xff, 0xff, 0xff, 0xf, 0x74, 0x65, 0x73, 0x74};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

void test_freertos_task_valmarker(void){
  {
    // Min
    uint8_t buf[EVT_FREERTOS_TASK_VALMARKER_MAXLEN] = {0};
    size_t len = encode_freertos_task_valmarker(buf, 0x0, 0x0, 0x0);
    uint8_t expected[] = {0x7f, 0x0, 0x0, 0x0};
    compare_arrays(buf, len, expected, sizeof(expected), "MIN");
  }
  {
    // Max
    uint8_t buf[EVT_FREERTOS_TASK_VALMARKER_MAXLEN] = {0};
    size_t len = encode_freertos_task_valmarker(buf, UINT64_MAX, UINT32_MAX, INT64_MIN + 1);
    uint8_t expected[] = {0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1, 0xff, 0xff, 0xff, 0xff, 0xf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1};
    compare_arrays(buf, len, expected, sizeof(expected), "MAX");
  }
}

// ==== Main =======================================================================================

void setUp(void) {}

void tearDown(void) {}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_core_id);
  RUN_TEST(test_dropped_evt_cnt);
  RUN_TEST(test_ts_resolution_ns);
  RUN_TEST(test_isr_name);
  RUN_TEST(test_isr_enter);
  RUN_TEST(test_isr_exit);
  RUN_TEST(test_evtmarker_name);
  RUN_TEST(test_evtmarker);
  RUN_TEST(test_evtmarker_begin);
  RUN_TEST(test_evtmarker_end);
  RUN_TEST(test_valmarker_name);
  RUN_TEST(test_valmarker);
  RUN_TEST(test_freertos_task_switched_in);
  RUN_TEST(test_freertos_task_to_rdy_state);
  RUN_TEST(test_freertos_task_resumed);
  RUN_TEST(test_freertos_task_resumed_from_isr);
  RUN_TEST(test_freertos_task_suspended);
  RUN_TEST(test_freertos_curtask_delay);
  RUN_TEST(test_freertos_curtask_delay_until);
  RUN_TEST(test_freertos_task_priority_set);
  RUN_TEST(test_freertos_task_priority_inherit);
  RUN_TEST(test_freertos_task_priority_disinherit);
  RUN_TEST(test_freertos_task_created);
  RUN_TEST(test_freertos_task_name);
  RUN_TEST(test_freertos_task_is_idle_task);
  RUN_TEST(test_freertos_task_is_timer_task);
  RUN_TEST(test_freertos_task_deleted);
  RUN_TEST(test_freertos_queue_created);
  RUN_TEST(test_freertos_queue_name);
  RUN_TEST(test_freertos_queue_kind);
  RUN_TEST(test_freertos_queue_send);
  RUN_TEST(test_freertos_queue_send_from_isr);
  RUN_TEST(test_freertos_queue_overwrite);
  RUN_TEST(test_freertos_queue_overwrite_from_isr);
  RUN_TEST(test_freertos_queue_receive);
  RUN_TEST(test_freertos_queue_receive_from_isr);
  RUN_TEST(test_freertos_queue_reset);
  RUN_TEST(test_freertos_curtask_block_on_queue_peek);
  RUN_TEST(test_freertos_curtask_block_on_queue_send);
  RUN_TEST(test_freertos_curtask_block_on_queue_receive);
  RUN_TEST(test_freertos_queue_cur_length);
  RUN_TEST(test_freertos_task_evtmarker_name);
  RUN_TEST(test_freertos_task_evtmarker);
  RUN_TEST(test_freertos_task_evtmarker_begin);
  RUN_TEST(test_freertos_task_evtmarker_end);
  RUN_TEST(test_freertos_task_valmarker_name);
  RUN_TEST(test_freertos_task_valmarker);
  return UNITY_END();
}
