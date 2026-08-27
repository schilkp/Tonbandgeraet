/**
 * @file semihosting.c
 * @brief Minimal raw ARM semihosting calls.
 *
 * See "Semihosting for AArch32 and AArch64", ARM IHI 0071.
 */
#include "semihosting.h"

#define SEMIHOSTING_SYS_OPEN     0x01
#define SEMIHOSTING_SYS_CLOSE    0x02
#define SEMIHOSTING_SYS_WRITEC   0x03
#define SEMIHOSTING_SYS_WRITE    0x05
#define SEMIHOSTING_SYS_EXIT_EXT 0x20

#define SEMIHOSTING_ADP_STOPPED_APPLICATION_EXIT 0x20026u

static uint32_t semihosting_call(uint32_t op, void *arg) {
  register uint32_t r0 __asm__("r0") = op;
  register void *r1 __asm__("r1") = arg;
  __asm__ volatile("bkpt 0xAB" : "+r"(r0) : "r"(r1) : "memory");
  return r0;
}

static uint32_t str_len(const char *s) {
  uint32_t len = 0;
  while (s[len] != '\0') {
    len++;
  }
  return len;
}

int32_t semihosting_open(const char *path, uint32_t mode) {
  uint32_t block[3] = {(uint32_t)(uintptr_t)path, mode, str_len(path)};
  return (int32_t)semihosting_call(SEMIHOSTING_SYS_OPEN, block);
}

uint32_t semihosting_write(int32_t handle, const void *buf, uint32_t len) {
  uint32_t block[3] = {(uint32_t)handle, (uint32_t)(uintptr_t)buf, len};
  return semihosting_call(SEMIHOSTING_SYS_WRITE, block);
}

void semihosting_close(int32_t handle) {
  uint32_t block[1] = {(uint32_t)handle};
  (void)semihosting_call(SEMIHOSTING_SYS_CLOSE, block);
}

void semihosting_writec(char c) { (void)semihosting_call(SEMIHOSTING_SYS_WRITEC, &c); }

void semihosting_exit(int code) {
  uint32_t block[2] = {SEMIHOSTING_ADP_STOPPED_APPLICATION_EXIT, (uint32_t)code};
  (void)semihosting_call(SEMIHOSTING_SYS_EXIT_EXT, block);
  for (;;) {
  }
}
