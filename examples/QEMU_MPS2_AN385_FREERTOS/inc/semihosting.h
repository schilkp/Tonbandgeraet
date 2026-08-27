/**
 * @file semihosting.h
 * @brief Minimal raw ARM semihosting calls.
 */
#ifndef SEMIHOSTING_H_
#define SEMIHOSTING_H_

#include <stddef.h>
#include <stdint.h>

// Open a file on the host. `mode` uses the fopen()-style semihosting mode
// table (0="r", 1="rb", ..., 4="w", 5="wb", ...). Returns a handle, or -1 on
// failure.
int32_t semihosting_open(const char *path, uint32_t mode);

// Write `len` bytes of `buf` to the host file `handle`. Returns the number of
// bytes that could *not* be written (0 on full success).
uint32_t semihosting_write(int32_t handle, const void *buf, uint32_t len);

// Close a host file previously opened with semihosting_open().
void semihosting_close(int32_t handle);

// Write a single character to the host's debug console.
void semihosting_writec(char c);

// End the program, reporting `code` as the host process' exit status. Never
// returns.
void semihosting_exit(int code) __attribute__((noreturn));

#endif /* SEMIHOSTING_H_ */
