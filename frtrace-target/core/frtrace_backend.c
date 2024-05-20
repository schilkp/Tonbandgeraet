/**
 * @file frtrace_backend.c
 * @brief FreeRTOS Tracer Backends
 * @author Philipp Schilk, 2024
 */

#include "FreeRTOS.h"

#if (frtrace_configENABLE == 1)

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

#include "task.h"

#include "frtrace_encode.h"

// ===== BACKWARDS-COMPATABILITY ===============================================

// FIXME add versionc check

#ifndef configNUMBER_OF_CORES
#define configNUMBER_OF_CORES 1
#endif /* configNUMBER_OF_CORES */

#ifndef portGET_CORE_ID
#define portGET_CORE_ID() 0
#endif /* portGET_CORE_ID */

// ===== MACRO MAGIC ===========================================================

// The tracer stores all per-core information in static arrays of structs of length portGET_CORE_ID.
// These often contain spinlocks, which are just a wrapper around atomic_flag, which needs
// to be initialised using ATOMIC_FLAG_INIT (FRTRACE_SPINLOCK_INIT). Since C has no mechanism
// for initialising an array of some length N to a non-zero value with a static initialiser, this
// macro hack repeats the desired initialiser once for every core. Not pretty, but it works.

// clang-format off
#if (configNUMBER_OF_CORES == 1)
  #define CORE_ARRAY_INIT(...) { __VA_ARGS__  }
#elif (configNUMBER_OF_CORES == 2)
  #define CORE_ARRAY_INIT(...) { __VA_ARGS__ , __VA_ARGS__  }
#elif (configNUMBER_OF_CORES == 3)
  #define CORE_ARRAY_INIT(...) { __VA_ARGS__ , __VA_ARGS__ , __VA_ARGS__  }
#elif (configNUMBER_OF_CORES == 4)
  #define CORE_ARRAY_INIT(...) { __VA_ARGS__ , __VA_ARGS__ , __VA_ARGS__ , __VA_ARGS__  }
#else
  #error Are you seriously using FreeRTOS SMP with more than 4 cores?! Wow..
#endif
// clang-format on

// ==== SPINLOCKS ==============================================================

#define FRTRACE_SPINLOCK_INIT ATOMIC_FLAG_INIT
typedef atomic_flag frtrace_spinlock;

// Must be called from a (per-core) critical section!
static inline bool frtrace_spinlock_try_acquire(volatile frtrace_spinlock *lock) {
#if (configNUMBER_OF_CORES > 1)
  return atomic_flag_test_and_set_explicit(lock, memory_order_acquire);
#else  /* configNUMBER_OF_CORES > 1 */
  (void)lock;
  return true;
#endif /* configNUMBER_OF_CORES > 1 */
}

// Must be called from a (per-core) critical section!
static inline void frtrace_spinlock_acquire(volatile frtrace_spinlock *lock) {
#if (configNUMBER_OF_CORES > 1)
  while(!atomic_flag_test_and_set_explicit(lock, memory_order_acquire){}
#else  /* configNUMBER_OF_CORES > 1 */
  (void)lock;
#endif /* configNUMBER_OF_CORES > 1 */
}

// Must be called from a (per-core) critical section!
static inline void frtrace_spinlock_release(volatile frtrace_spinlock *lock) {
#if (configNUMBER_OF_CORES > 1)
  atomic_flag_clear_explicit(lock, memory_order_release);
#else  /* configNUMBER_OF_CORES > 1 */
  (void)lock;
#endif /* configNUMBER_OF_CORES > 1 */
}

//                      ____ ___  __  __ __  __  ___  _   _
//                     / ___/ _ \|  \/  |  \/  |/ _ \| \ | |
// ================== | |  | | | | |\/| | |\/| | | | |  \| | ===================
// ================== | |__| |_| | |  | | |  | | |_| | |\  | ===================
//                     \____\___/|_|  |_|_|  |_|\___/|_| \_|

// ==== GLOBAL TRACING_ENABLED FLAG & BACKEND SPINLOCKS ========================

// Global tracing enabled flag.
// Spinlock must be held when enabling/disabling tracing, but not to check if tracing is enabled.
static volatile atomic_bool tracing_enabled = false;
static volatile frtrace_spinlock tracing_enabled_spinlock = FRTRACE_SPINLOCK_INIT;

// Per-core backend spinlocks. Must be held by each core backend while actively handling a trace
// event/ modifying its state. In particular, each backend must do the following for each event:
//   - Check that tracing_enabled is set, otherwise drop the event.
//   - Acquire its backend spinlock.
//   - Check that tracing_enabled is set again, and only handle the evnt if so.
//   - Release its backend spinlock.
// clang-format off
static volatile frtrace_spinlock backend_spinlocks[configNUMBER_OF_CORES] = CORE_ARRAY_INIT(
  FRTRACE_SPINLOCK_INIT
);
// clang-format on

// Internal implementation. Checks that tracing is disabled and all backends have completed
// handling all events. *Must* be called from a critical section and while tracing_enabled_spinlock
// is held!
static bool impl_tracing_finished() {
  // This relies the backends only checking the global tracing_enabled flag once
  // they already hold their backend spinlock.
  //
  // It first checks that this tracing_enabled_spinlock is false. If so, since it requires
  // tracing_enabled_spinlock to be held while calling, it is guaranteed that tracing_enabled will
  // remain false until this function retuns. Should tracing_enabled be true, the backends are
  // clearly not finished and we return.
  //
  // Now, with tracing_enabled disabled, only backends that are already holding their backend
  // spinlock could still be working based on having checked the global flag previously before it
  // was disabled. Thus, if this function determines that no backend spinlock is locked, all
  // backends are guaranteed to be finished and will not modify their state any further.
  //
  // While the above is necessary for all backend work to be finished, it can still fail after
  // tracing has been disabled and everything has completed: Backends will continue to acquire and
  // release their backend spinlocks to check the tracing_enabled every time they encounter a trace
  // event. If this function were to be called while this is happening, it would not be able to
  // conclude that all backends have finished. By additionally requring all backends to check the
  // global flag also before acquiring their spinlock, they will not continue to lock and unlock
  // their spinlocks after acquisition has been disabled and all work has completed.
  if (atomic_load(&tracing_enabled)) {
    return false;
  }

  for (unsigned int core_id = 0; core_id < configNUMBER_OF_CORES; core_id++) {
    if (frtrace_spinlock_try_acquire(&backend_spinlocks[core_id])) {
      // Backend not busy.
      frtrace_spinlock_release(&backend_spinlocks[core_id]);
    } else {
      // Backend still busy - not all backends are done.
      return false;
    }
  }

  return true;
}

// Internal implementation. Checks that tracing is disabled and that the backend for the given
// core has finished handling any last events. *Must* be called from a critical section and while
// tracing_enabled_spinlock is held!
static bool impl_backend_finished(unsigned int core_id) {
  // See impl_tracing_finished for implementation explaination.

  if (atomic_load(&tracing_enabled)) {
    return false;
  }

  if (frtrace_spinlock_try_acquire(&backend_spinlocks[core_id])) {
    // Backend not busy.
    frtrace_spinlock_release(&backend_spinlocks[core_id]);
    return true;
  } else {
    // Backend still busy.
    return false;
  }
}

bool frtrace_tracing_enabled() { return atomic_load(&tracing_enabled); }

bool frtrace_tracing_finished() {
  frtrace_portKERNEL_ENTER_CRITICAL_FROM_ANY();
  frtrace_spinlock_acquire(&tracing_enabled_spinlock);

  bool is_completed = impl_tracing_finished();

  frtrace_spinlock_release(&tracing_enabled_spinlock);
  frtrace_portKERNEL_EXIT_CRITICAL_FROM_ANY();

  return is_completed;
}

bool frtrace_tracing_backend_finished(unsigned int core_id) {
  frtrace_portKERNEL_ENTER_CRITICAL_FROM_ANY();
  frtrace_spinlock_acquire(&tracing_enabled_spinlock);

  bool is_completed = impl_backend_finished(core_id);

  frtrace_spinlock_release(&tracing_enabled_spinlock);
  frtrace_portKERNEL_EXIT_CRITICAL_FROM_ANY();

  return is_completed;
}

// ==== Metadata Buffer ========================================================

// FIXME: Should metadata buffer be per-core? Wastes quite some space because every core needs a
// large buffer.. Should this be configurable? Maybe instead provide mechanism for
// externally-allocated buffers that can be different sizes?

#if (frtrace_configUSE_METADATA_BUF == 1)

struct metadata_buf {
  frtrace_spinlock spinlock;
  uint8_t buf[frtrace_configMETADATA_BUF_SIZE + 1];
  size_t idx;
  bool did_ovf;
};

// clang-format off
static volatile struct metadata_buf metadata_bufs[configNUMBER_OF_CORES] = CORE_ARRAY_INIT({
  .spinlock = FRTRACE_SPINLOCK_INIT,
  .buf = {0},
  .idx = 1,
  .did_ovf = false
});
// clang-format on

// Append data to the metadata buffer. Must be called from a (per-core) critical section!
static void append_to_metadata_buf(uint8_t *buf, size_t len) {
  unsigned int core_id = portGET_CORE_ID();

  frtrace_spinlock_acquire(&metadata_bufs[core_id].spinlock);

  size_t idx = metadata_bufs[core_id].idx;
  size_t buf_size = frtrace_configMETADATA_BUF_SIZE;

  if (idx < buf_size && (buf_size - idx) >= len) {
    // Data fits into buffer.
    for (size_t i = 0; i < len; i++) {
      metadata_bufs[core_id].buf[idx + i] = buf[i];
    }
    metadata_bufs[core_id].idx += len;
  } else {
    // Data does not fit into buffer.
    metadata_bufs[core_id].did_ovf = true;
  }
  frtrace_spinlock_release(&metadata_bufs[core_id].spinlock);
}

// Public API to get pointer to metadata buffer.
const volatile uint8_t *frtrace_get_metadata_buf(unsigned int core_id) {
  return (volatile uint8_t *)metadata_bufs[core_id].buf;
}

// Public API to get current metadata buffer fill level.
size_t frtrace_get_metadata_buf_amnt(unsigned int core_id) {

  frtrace_portKERNEL_ENTER_CRITICAL_FROM_ANY();
  frtrace_spinlock_acquire(&metadata_bufs[core_id].spinlock);

  size_t metadata_buf_amnt = metadata_bufs[core_id].idx;

  frtrace_spinlock_release(&metadata_bufs[core_id].spinlock);
  frtrace_portKERNEL_EXIT_CRITICAL_FROM_ANY();

  return metadata_buf_amnt;
}

#endif /* frtrace_configUSE_METADATA_BUF == 1  */

//             ____ _____ ____  _____    _    __  __ ___ _   _  ____
//            / ___|_   _|  _ \| ____|  / \  |  \/  |_ _| \ | |/ ___|
// ========== \___ \ | | | |_) |  _|   / _ \ | |\/| || ||  \| | |  _  ==========
// ==========  ___) || | |  _ <| |___ / ___ \| |  | || || |\  | |_| | ==========
//            |____/ |_| |_| \_\_____/_/   \_\_|  |_|___|_| \_|\____|

#if (frtrace_configUSE_BACKEND_STREAMING == 1)

#ifndef frtrace_portBACKEND_STREAM_DATA
#error "frtrace_portBACKEND_STREAM_DATA is not defined but required for the stream backend!"
#endif /* frtrace_portBACKEND_STREAM_DATA */

// Tracer backend API. Must only be called by tracer hooks. Must be called from
// (per-core) critical section.
bool frtrace_submit_to_backend(uint8_t *buf, size_t len, bool is_metadata) {

  // If the metadata buffer is enabled and this is a piece of metadata
  // information, add it to the metadata buffer:
#if (frtrace_configUSE_METADATA_BUF == 1)
  if (is_metadata) {
    append_to_metadata_buf(buf, len);
  }
#else  /* frtrace_configUSE_METADATA_BUF == 1 */
  (void)(is_metadata);
#endif /* frtrace_configUSE_METADATA_BUF == 1 */

  bool did_drop = false;
  unsigned int core_id = portGET_CORE_ID();

  // Global flag is checked before and after backend spinlock is acquired.
  // This is required for impl_tracing_finished to work correctly! See function for more details.
  if (atomic_load(&tracing_enabled)) {
    frtrace_spinlock_acquire(&backend_spinlocks[core_id]);
    if (atomic_load(&tracing_enabled)) {
      did_drop = frtrace_portBACKEND_STREAM_DATA(buf, len);
    }
    frtrace_spinlock_release(&backend_spinlocks[core_id]);
  }

  return did_drop;
}

int frtrace_start_streaming() {
  int err = 0;
  frtrace_portKERNEL_ENTER_CRITICAL_FROM_ANY();
  frtrace_spinlock_acquire(&tracing_enabled_spinlock);

  // Check if tracing is already active:
  bool tracing_finished = impl_tracing_finished();
  if (!tracing_finished) {
    err = -1;
    goto end;
  }

  // Send data from metadata buffers, if enabled:
#if (frtrace_configUSE_METADATA_BUF == 1)

  bool did_drop = false;

  for (unsigned int core_id = 0; core_id < configNUMBER_OF_CORES; core_id++) {

    frtrace_spinlock_acquire(&metadata_bufs[core_id].spinlock);
    size_t metadata_amnt = metadata_bufs[core_id].idx;
    frtrace_spinlock_release(&metadata_bufs[core_id].spinlock);

    uint8_t *buf = (uint8_t *)metadata_bufs[core_id].buf;
    if (metadata_amnt > 0) {
      uint8_t core_id_msg[CORE_ID_MAXLEN] = {0};
      size_t core_id_msg_len = encode_core_id(core_id_msg, 0, core_id);
      did_drop |= frtrace_portBACKEND_STREAM_DATA(core_id_msg, core_id_msg_len);
      did_drop |= frtrace_portBACKEND_STREAM_DATA(buf, metadata_amnt);
    }
    if (did_drop) break;
  }

  // Reset to current core:
  uint8_t core_id_msg[CORE_ID_MAXLEN] = {0};
  size_t core_id_msg_len = encode_core_id(core_id_msg, 0, portGET_CORE_ID());
  did_drop |= frtrace_portBACKEND_STREAM_DATA(core_id_msg, core_id_msg_len);

  if (did_drop) {
    err = -2;
    goto end;
  }

#endif /* frtrace_configUSE_METADATA_BUF == 1 */

  atomic_store(&tracing_enabled, true);

end:
  frtrace_spinlock_release(&tracing_enabled_spinlock);
  frtrace_portKERNEL_EXIT_CRITICAL_FROM_ANY();

  return err;
}

int frtrace_stop_streaming() {
  int err = 0;
  frtrace_portKERNEL_ENTER_CRITICAL_FROM_ANY();
  frtrace_spinlock_acquire(&tracing_enabled_spinlock);

  bool was_enabled = atomic_exchange(&tracing_enabled, false);
  if (!was_enabled) {
    err = -1;
  }

  frtrace_spinlock_release(&tracing_enabled_spinlock);
  frtrace_portKERNEL_EXIT_CRITICAL_FROM_ANY();

  return err;
}

#endif /* frtrace_configUSE_BACKEND_STREAMING == 1  */

//               ____  _   _    _    ____  ____  _   _  ___ _____
//              / ___|| \ | |  / \  |  _ \/ ___|| | | |/ _ \_   _|
// ============ \___ \|  \| | / _ \ | |_) \___ \| |_| | | | || |   =============
// ============  ___) | |\  |/ ___ \|  __/ ___) |  _  | |_| || |   =============
//              |____/|_| \_/_/   \_\_|   |____/|_| |_|\___/ |_|

#if (frtrace_configUSE_BACKEND_SNAPSHOT == 1)

#ifndef frtrace_portBACKEND_SNAPSHOT_DONE_CALLBACK
#define frtrace_portBACKEND_SNAPSHOT_DONE_CALLBACK()
#endif /* frtrace_portBACKEND_SNAPSHOT_DONE_CALLBACK */

// Per-core snapshot backend state:
struct frtrace_snapshot_backend {
  uint8_t buf[frtrace_configBACKEND_SNAPSHOT_BUF_SIZE + 1];
  size_t idx;
};

// clang-format off
static volatile struct frtrace_snapshot_backend snapshot_backends[configNUMBER_OF_CORES] = CORE_ARRAY_INIT({
  .buf = {0},
  .idx = 1,
});
// clang-format on

// Tracer backend API. Must only be called by tracer hooks. Must be called from
// (per-core) critical section.
bool frtrace_submit_to_backend(uint8_t *buf, size_t len, bool is_metadata) {

  // If the metadata buffer is enabled and this is a piece of metadata
  // information, add it to the metadata buffer:
#if (frtrace_configUSE_METADATA_BUF == 1)
  if (is_metadata) {
    append_to_metadata_buf(buf, len);
  }
#else  /* frtrace_configUSE_METADATA_BUF == 1 */
  (void)(is_metadata);
#endif /* frtrace_configUSE_METADATA_BUF == 1 */

  bool buffer_full = false;
  unsigned int core_id = portGET_CORE_ID();

  // Global flag is checked before and after backend spinlock is acquired.
  // This is required for impl_tracing_finished to work correctly! See function for more details.
  if (atomic_load(&tracing_enabled)) {
    frtrace_spinlock_acquire(&backend_spinlocks[core_id]);

    if (atomic_load(&tracing_enabled)) {
      size_t idx = snapshot_backends[core_id].idx;
      size_t buf_size = frtrace_configBACKEND_SNAPSHOT_BUF_SIZE;
      if (idx < buf_size && (buf_size - idx) >= len) {
        // Space in buffer. Append.
        for (size_t i = 0; i < len; i++) {
          snapshot_backends[core_id].buf[idx + i] = buf[i];
        }
        snapshot_backends[core_id].idx += len;
      } else {
        // Buffer full
        buffer_full = true;
      }
    }

    frtrace_spinlock_release(&backend_spinlocks[core_id]);
  }

  if (buffer_full) {
    // Stop tracing since buffer has filled.
    // First, acquire tracing_enabled_spinlock to be allowed to modify tracing_enabled:
    frtrace_spinlock_acquire(&tracing_enabled_spinlock);

    bool was_enabled = atomic_exchange(&tracing_enabled, false);

    // Only call callback if we were the ones to actually disable snapshot acquisition,
    // preventing the callback being called multiple times:
    if (was_enabled) {
      frtrace_portBACKEND_SNAPSHOT_DONE_CALLBACK();
    }

    frtrace_spinlock_release(&tracing_enabled_spinlock);
  }

  // Snapshot backend does not drop data: Once the buffer is full, tracing
  // is finished.
  return false;
}

int frtrace_trigger_snapshot() {
  int err = 0;

  frtrace_portKERNEL_ENTER_CRITICAL_FROM_ANY();
  frtrace_spinlock_acquire(&tracing_enabled_spinlock);

  // Check if tracing is already active:
  bool tracing_not_active = impl_tracing_finished();
  if (!tracing_not_active) {
    err = -1;
    goto end;
  }

  // Enable tracing:
  atomic_store(&tracing_enabled, true);

end:
  frtrace_spinlock_release(&tracing_enabled_spinlock);
  frtrace_portKERNEL_EXIT_CRITICAL_FROM_ANY();

  return err;
}

int frtrace_stop_snapshot() {
  int err = 0;
  frtrace_portKERNEL_ENTER_CRITICAL_FROM_ANY();
  frtrace_spinlock_acquire(&tracing_enabled_spinlock);

  bool was_enabled = atomic_exchange(&tracing_enabled, false);
  if (!was_enabled) {
    err = -1;
  }

  frtrace_spinlock_release(&tracing_enabled_spinlock);
  frtrace_portKERNEL_EXIT_CRITICAL_FROM_ANY();

  return err;
}

// FIXME ENSURE WE NEVER TRY TO ACQUIRE TRACING_ACTIVE WHILE HOLDING BACKEND
// SPINLOCk

int frtrace_reset_snapshot() {
  int err = 0;

  frtrace_portKERNEL_ENTER_CRITICAL_FROM_ANY();
  frtrace_spinlock_acquire(&tracing_enabled_spinlock);

  // Check if tracing is already active:
  bool tracing_not_active = !impl_tracing_finished();
  if (!tracing_not_active) {
    err = -1;
    goto end;
  }

  for (unsigned int core_id = 0; core_id < configNUMBER_OF_CORES; core_id++) {
    frtrace_spinlock_acquire(&backend_spinlocks[core_id]);

    // Dont need to clear whole buffer - it won't get read if the idx is reset.
    // The first element is reset to zero, but this is technically unecessary:
    // it should never be written to!
    snapshot_backends[core_id].idx = 1;
    snapshot_backends[core_id].buf[0] = 0;

    frtrace_spinlock_release(&backend_spinlocks[core_id]);
  }

end:
  frtrace_spinlock_release(&tracing_enabled_spinlock);
  frtrace_portKERNEL_EXIT_CRITICAL_FROM_ANY();

  return err;
}

const volatile uint8_t *frtrace_get_core_snapshot_buf(unsigned int core_id) {
  return snapshot_backends[core_id].buf;
}

size_t frtrace_get_core_snapshot_buf_amnt(unsigned int core_id) {
  size_t amnt = 0;
  frtrace_portKERNEL_ENTER_CRITICAL_FROM_ANY();
  frtrace_spinlock_acquire(&tracing_enabled_spinlock);

  if (impl_backend_finished(core_id)) {
    frtrace_spinlock_acquire(&backend_spinlocks[core_id]);
    amnt = snapshot_backends[core_id].idx;
    frtrace_spinlock_release(&backend_spinlocks[core_id]);
  }

  frtrace_spinlock_release(&tracing_enabled_spinlock);
  frtrace_portKERNEL_EXIT_CRITICAL_FROM_ANY();
  return amnt;
}

#endif /* frtrace_configUSE_BACKEND_SNAPSHOT == 1  */

//         ____   ___  ____ _____   __  __  ___  ____ _____ _____ __  __
//        |  _ \ / _ \/ ___|_   _| |  \/  |/ _ \|  _ \_   _| ____|  \/  |
// ====== | |_) | | | \___ \ | |   | |\/| | | | | |_) || | |  _| | |\/| | ======
// ====== |  __/| |_| |___) || |   | |  | | |_| |  _ < | | | |___| |  | | ======
//        |_|    \___/|____/ |_|   |_|  |_|\___/|_| \_\|_| |_____|_|  |_|
//

#if (frtrace_configUSE_BACKEND_POST_MORTEM == 1)

// Tracer backend API. Must only be called by tracer hooks. Must be called from
// (per-core) critical section.
bool frtrace_submit_to_backend(uint8_t *buf, size_t len, bool is_metadata) {
#error TODO
}

#endif /* frtrace_configUSE_BACKEND_POST_MORTEM == 1  */

#endif /* frtrace_configENABLE == 1 */
