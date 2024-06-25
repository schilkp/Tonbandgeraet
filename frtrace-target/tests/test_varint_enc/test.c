#include <string.h>

#include "unity.h"
#include "unity_internals.h"

#include "FreeRTOS.h"

#include "frtrace_encode.h"

void util_print_array(uint8_t *in, size_t in_len, char *name) {
  printf("%s: ", name);
  for (size_t i = 0; i < in_len; i++) {
    printf("0x%02x ", in[i]);
  }
  printf("\r\n");
}

void util_u32_testcase(uint32_t in, uint8_t *expect, size_t expect_len) {
  uint8_t outbuf_expect[10];
  memset(outbuf_expect, 0xAB, 10);
  struct cobs_state cobs = cobs_start(outbuf_expect);
  for (size_t i = 0; i < expect_len; i++) {
    cobs_add_byte(&cobs, expect[i]);
  }
  expect_len = cobs_finish(&cobs);

  // Perform Varint encoding:
  uint8_t outbuf_varint[10];
  memset(outbuf_varint, 0xAB, 10);
  cobs = cobs_start(outbuf_varint);
  encode_u32(&cobs, in);
  size_t len_varint = cobs_finish(&cobs);

  util_print_array(outbuf_expect, expect_len, "EXP");
  util_print_array(outbuf_varint, len_varint, "OUT");

  TEST_ASSERT_EQUAL_INT(expect_len, len_varint);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(outbuf_expect, outbuf_varint, expect_len);
}

void util_u64_testcase(uint64_t in, uint8_t *expect, size_t expect_len) {

  // COBS frame expected value:
  uint8_t outbuf_expect[20];
  memset(outbuf_expect, 0xAB, 20);
  struct cobs_state cobs = cobs_start(outbuf_expect);
  for (size_t i = 0; i < expect_len; i++) {
    cobs_add_byte(&cobs, expect[i]);
  }
  expect_len = cobs_finish(&cobs);

  // Perform Varint encoding:
  uint8_t outbuf_varint[20];
  memset(outbuf_varint, 0xAB, 10);
  cobs = cobs_start(outbuf_varint);
  encode_u64(&cobs, in);
  size_t len_varint = cobs_finish(&cobs);

  util_print_array(outbuf_expect, expect_len, "EXP");
  util_print_array(outbuf_varint, len_varint, "OUT");

  TEST_ASSERT_EQUAL_INT(expect_len, len_varint);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(outbuf_expect, outbuf_varint, expect_len);
}

void util_s64_testcase(int64_t in, uint8_t *expect, size_t expect_len) {

  // COBS frame expected value:
  uint8_t outbuf_expect[20];
  memset(outbuf_expect, 0xAB, 20);
  struct cobs_state cobs = cobs_start(outbuf_expect);
  for (size_t i = 0; i < expect_len; i++) {
    cobs_add_byte(&cobs, expect[i]);
  }
  expect_len = cobs_finish(&cobs);

  // Perform Varint encoding:
  uint8_t outbuf_varint[20];
  memset(outbuf_varint, 0xAB, 10);
  cobs = cobs_start(outbuf_varint);
  encode_s64(&cobs, in);
  size_t len_varint = cobs_finish(&cobs);

  util_print_array(outbuf_expect, expect_len, "EXP");
  util_print_array(outbuf_varint, len_varint, "OUT");

  TEST_ASSERT_EQUAL_INT(expect_len, len_varint);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(outbuf_expect, outbuf_varint, expect_len);
}

// ======== Tests ==================================================================================

static void test_varint_u32_zero(void) {
  uint8_t expect[] = {0x0};
  util_u32_testcase(0, expect, sizeof(expect));
}

static void test_varint_u32_ff(void) {
  uint8_t expect[] = {0xFF, 0x1};
  util_u32_testcase(0xFF, expect, sizeof(expect));
}

static void test_varint_u32_ffffffff(void) {
  uint8_t expect[] = {0xFF, 0xFF, 0xFF, 0xFF, 0x0F};
  util_u32_testcase(0xFFFFFFFF, expect, sizeof(expect));
}

static void test_varint_u32_onehot(void) {
  {
    uint8_t expect[] = {0x80, 0x01};
    util_u32_testcase(0x80, expect, sizeof(expect));
  }
  {
    uint8_t expect[] = {0x80, 0x02};
    util_u32_testcase(0x100, expect, sizeof(expect));
  }
  {
    uint8_t expect[] = {0x80, 0x40};
    util_u32_testcase(0x1 << 13, expect, sizeof(expect));
  }
  {
    uint8_t expect[] = {0x80, 0x80, 0x01};
    util_u32_testcase(0x1 << 14, expect, sizeof(expect));
  }
  {
    uint8_t expect[] = {0x80, 0x80, 0x80, 0x80, 0x08};
    util_u32_testcase(0x1U << 31, expect, sizeof(expect));
  }
}

static void test_varint_u64_zero(void) {
  uint8_t expect[] = {0x0};
  util_u64_testcase(0, expect, sizeof(expect));
}

static void test_varint_u64_ff(void) {
  uint8_t expect[] = {0xFF, 0x1};
  util_u64_testcase(0xFF, expect, sizeof(expect));
}

static void test_varint_u64_ffffffff(void) {
  uint8_t expect[] = {0xFF, 0xFF, 0xFF, 0xFF, 0x0F};
  util_u64_testcase(0xFFFFFFFF, expect, sizeof(expect));
}

static void test_varint_u64_ffffffffffffffff(void) {
  uint8_t expect[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01};
  util_u64_testcase(0xFFFFFFFFFFFFFFFF, expect, sizeof(expect));
}

static void test_varint_u64_onehot(void) {
  {
    uint8_t expect[] = {0x80, 0x01};
    util_u64_testcase(0x80, expect, sizeof(expect));
  }
  {
    uint8_t expect[] = {0x80, 0x02};
    util_u64_testcase(0x100, expect, sizeof(expect));
  }
  {
    uint8_t expect[] = {0x80, 0x40};
    util_u64_testcase(0x1 << 13, expect, sizeof(expect));
  }
  {
    uint8_t expect[] = {0x80, 0x80, 0x01};
    util_u64_testcase(0x1 << 14, expect, sizeof(expect));
  }
  {
    uint8_t expect[] = {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x01};
    util_u64_testcase(((uint64_t)0x1) << 63, expect, sizeof(expect));
  }
}

static void test_varint_s64_zero(void) {
  uint8_t expect[] = {0x0};
  util_s64_testcase(0, expect, sizeof(expect));
}

static void test_varint_s64_one(void) {
  uint8_t expect[] = {0x2};
  util_s64_testcase(1, expect, sizeof(expect));
}

static void test_varint_s64_neg_one(void) {
  uint8_t expect[] = {0x3};
  util_s64_testcase(-1, expect, sizeof(expect));
}

static void test_varint_s64_max(void) {
  uint8_t expect[] = {0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01};
  util_s64_testcase(INT64_MAX, expect, sizeof(expect));
}

static void test_varint_s64_min(void) {
  uint8_t expect[] = {0x01};
  util_s64_testcase(INT64_MIN, expect, sizeof(expect));
}

static void test_varint_s64_min_p_1(void) {
  uint8_t expect[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01};
  util_s64_testcase(INT64_MIN + 1, expect, sizeof(expect));
}

// ======== Main ===================================================================================

void setUp(void) {}

void tearDown(void) {}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_varint_u32_zero);
  RUN_TEST(test_varint_u32_ff);
  RUN_TEST(test_varint_u32_ffffffff);
  RUN_TEST(test_varint_u32_onehot);
  RUN_TEST(test_varint_u64_zero);
  RUN_TEST(test_varint_u64_ff);
  RUN_TEST(test_varint_u64_ffffffff);
  RUN_TEST(test_varint_u64_ffffffffffffffff);
  RUN_TEST(test_varint_u64_onehot);
  RUN_TEST(test_varint_s64_zero);
  RUN_TEST(test_varint_s64_one);
  RUN_TEST(test_varint_s64_neg_one);
  RUN_TEST(test_varint_s64_max);
  RUN_TEST(test_varint_s64_min);
  RUN_TEST(test_varint_s64_min_p_1);
  return UNITY_END();
}
