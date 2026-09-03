/**
 * @file tband_atomics.h
 * @brief Tonbandgeraet: Atomic primitives (spinlocks, atomic flags/counters).
 * @note Copyright (c) 2024-2026 Philipp Schilk. Released under the MIT license.
 * @author Philipp Schilk, 2024-2026
 *
 * https://github.com/schilkp/Tonbandgeraet
 */
#ifndef TBAND_ATOMICS_H_
#define TBAND_ATOMICS_H_

#ifndef tbandPROPER_INTERNAL_INCLUDE
#error                                                                                             \
  "This internal header file is not a public API and should not be included. Include tband.h instead."
#endif /* tbandPROPER_INTERNAL_INCLUDE */

#include <stdbool.h>
#include <stdint.h>

// On single-core ports, any static shared state that is only accessed from within per-core
// critical sections does not actually require any further explicit protection
// (such as being marked as volatile, or using stdatomic to manage it).
//
// This header provides utility functions which are cheap, inline, pure C implementations
// on single core ports, and fall-back on stdatomic/volatile etc for multicore ports.
//
// Note that this is only safe, if all such state is accessed only from within
// per-core critical sections!

#if (tband_portNUMBER_OF_CORES > 1)
#include <stdatomic.h>
#endif /* tband_portNUMBER_OF_CORES > 1 */

// ==== tband_smp_volatile =====================================================
// `volatile` modifier which is a no-op on single-core builds, but actually
// `volatile` on multi-core builds.
//
// Generally useful for slightly improved single core performance of variables
// only accessed from per-core critical sections.

#if (tband_portNUMBER_OF_CORES > 1)
#define tband_smp_volatile volatile
#else /* tband_portNUMBER_OF_CORES > 1 */
#define tband_smp_volatile
#endif /* tband_portNUMBER_OF_CORES > 1 */

// ==== Spinlock ================================================================
// A spinlock used for cross-core mutual exclusion.
//
// On single-core builds, no other core can contend for it, so these functions
// are just no-ops.
//
// Must only be acquired/released from within a (per-core) critical section!

#if (tband_portNUMBER_OF_CORES > 1)
typedef atomic_flag tband_spinlock;
#define tband_spinlock_INIT ATOMIC_FLAG_INIT
#else /* tband_portNUMBER_OF_CORES > 1 */
typedef uint8_t tband_spinlock;
#define tband_spinlock_INIT (0)
#endif /* tband_portNUMBER_OF_CORES > 1 */

// ! Must be called from within a (per-core) critical section !
static inline bool tband_spinlock_try_acquire(volatile tband_spinlock *lock) {
#if (tband_portNUMBER_OF_CORES > 1)
  // We acquired the spinlock iff the previous spinlock value (return value
  // of test_and_set_explicit) was false meaning we were the one to set it to
  // true.
  return !atomic_flag_test_and_set_explicit(lock, memory_order_acquire);
#else  /* tband_portNUMBER_OF_CORES > 1 */
  (void)lock;
  return true;
#endif /* tband_portNUMBER_OF_CORES > 1 */
}

// ! Must be called from within a (per-core) critical section !
static inline void tband_spinlock_acquire(volatile tband_spinlock *lock) {
#if (tband_portNUMBER_OF_CORES > 1)
  // Spin until we were the ones to set the lock to true. (Previously lock
  // value/return value of `test_and_set_explicit` is false).
  while (atomic_flag_test_and_set_explicit(lock, memory_order_acquire)) {
  }
#else  /* tband_portNUMBER_OF_CORES > 1 */
  (void)lock;
#endif /* tband_portNUMBER_OF_CORES > 1 */
}

// ! Must be called from within a (per-core) critical section !
static inline void tband_spinlock_release(volatile tband_spinlock *lock) {
#if (tband_portNUMBER_OF_CORES > 1)
  atomic_flag_clear_explicit(lock, memory_order_release);
#else  /* tband_portNUMBER_OF_CORES > 1 */
  (void)lock;
#endif /* tband_portNUMBER_OF_CORES > 1 */
}

// ==== tband_smp_atomic_* =====================================================
// Atomic bool/int types + access functions, which may only be accessed from
// (per-core) critical sections!
//
// Because of this limitation, on single-core ports, we can use non-atomic
// C-style variable access. On multi-core ports, we require `stdatomic` and
// use the actual atomic primitives it provides.

#if (tband_portNUMBER_OF_CORES > 1)
typedef atomic_bool tband_smp_atomic_bool;
#else  /* tband_portNUMBER_OF_CORES > 1 */
typedef bool tband_smp_atomic_bool;
#endif /* tband_portNUMBER_OF_CORES > 1 */

// ! Must be called from within a (per-core) critical section !
static inline bool tband_smp_atomic_bool_load(const volatile tband_smp_atomic_bool *a) {
#if (tband_portNUMBER_OF_CORES > 1)
  return atomic_load(a);
#else  /* tband_portNUMBER_OF_CORES > 1 */
  return *a;
#endif /* tband_portNUMBER_OF_CORES > 1 */
}

// ! Must be called from within a (per-core) critical section !
static inline void tband_smp_atomic_bool_store(volatile tband_smp_atomic_bool *a, bool v) {
#if (tband_portNUMBER_OF_CORES > 1)
  atomic_store(a, v);
#else  /* tband_portNUMBER_OF_CORES > 1 */
  *a = v;
#endif /* tband_portNUMBER_OF_CORES > 1 */
}

// ! Must be called from within a (per-core) critical section !
static inline bool tband_smp_atomic_bool_exchange(volatile tband_smp_atomic_bool *a, bool v) {
#if (tband_portNUMBER_OF_CORES > 1)
  return atomic_exchange(a, v);
#else  /* tband_portNUMBER_OF_CORES > 1 */
  bool old = *a;
  *a = v;
  return old;
#endif /* tband_portNUMBER_OF_CORES > 1 */
}

#if (tband_portNUMBER_OF_CORES > 1)
typedef atomic_uint_least32_t tband_smp_atomic_u32;
#else  /* tband_portNUMBER_OF_CORES > 1 */
typedef uint32_t tband_smp_atomic_u32;
#endif /* tband_portNUMBER_OF_CORES > 1 */

// ! Must be called from within a (per-core) critical section !
static inline uint32_t tband_smp_atomic_u32_load(const volatile tband_smp_atomic_u32 *a) {
#if (tband_portNUMBER_OF_CORES > 1)
  return atomic_load(a);
#else  /* tband_portNUMBER_OF_CORES > 1 */
  return *a;
#endif /* tband_portNUMBER_OF_CORES > 1 */
}

// ! Must be called from within a (per-core) critical section !
static inline uint32_t tband_smp_atomic_u32_fetch_add(volatile tband_smp_atomic_u32 *a,
                                                      uint32_t v) {
#if (tband_portNUMBER_OF_CORES > 1)
  return atomic_fetch_add(a, v);
#else  /* tband_portNUMBER_OF_CORES > 1 */
  uint32_t old = *a;
  *a = (uint32_t)(old + v);
  return old;
#endif /* tband_portNUMBER_OF_CORES > 1 */
}

#endif /* TBAND_ATOMICS_H_ */
