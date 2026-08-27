/**
 * @file startup.c
 * @brief Minimal startup code and vector table for the QEMU "mps2-an385" (Cortex-M3) machine.
 */
#include <stdint.h>

#include "semihosting.h"

extern uint32_t _estack;
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

extern int main(void);

void Reset_Handler(void);
static void Default_Handler(void);
static void HardFault_Handler(void);

extern void vPortSVCHandler(void);
extern void xPortPendSVHandler(void);
extern void xPortSysTickHandler(void);

// ==== Vector Table ============================================================

// Cast via uintptr_t to avoid -Wpedantic warnings about object/function pointer
// conversion.
#define VEC(f) ((uint32_t)(uintptr_t)(f))

// clang-format off
__attribute__((section(".isr_vector"), used))
const uint32_t vector_table[] = {
    VEC(&_estack),               // Initial stack pointer.
    VEC(Reset_Handler),          // Reset
    VEC(Default_Handler),        // NMI
    VEC(HardFault_Handler),      // HardFault
    VEC(Default_Handler),        // MemManage
    VEC(Default_Handler),        // BusFault
    VEC(Default_Handler),        // UsageFault
    0, 0, 0, 0,                  // Reserved
    VEC(vPortSVCHandler),        // SVCall
    VEC(Default_Handler),        // DebugMonitor
    0,                           // Reserved
    VEC(xPortPendSVHandler),     // PendSV
    VEC(xPortSysTickHandler),    // SysTick
};
// clang-format on

// ==== Reset Handler ============================================================

void Reset_Handler(void) {
  // Copy .data from flash to RAM.
  uint32_t *src = &_sidata;
  uint32_t *dst = &_sdata;
  while (dst < &_edata) {
    *dst++ = *src++;
  }

  // Zero-initialize .bss.
  dst = &_sbss;
  while (dst < &_ebss) {
    *dst++ = 0;
  }

  main();

  // main() only returns via an explicit semihosting_exit(), which never
  // returns either - this is unreachable, but kept as a safety net.
  semihosting_exit(1);
}

// ==== Fault Handlers ===========================================================

static void Default_Handler(void) { semihosting_exit(1); }

static void HardFault_Handler(void) { semihosting_exit(1); }
