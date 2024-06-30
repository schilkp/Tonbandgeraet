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
 */
#include <string.h>

#include "unity.h"
#include "unity_internals.h"

#include "FreeRTOS.h"

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
