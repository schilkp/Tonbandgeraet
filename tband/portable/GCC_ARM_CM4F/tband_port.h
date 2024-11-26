/**
 * @file tband_port.h
 * @brief Tonbandgeraet bare-metal Cortex M4F port
 * @author Philipp Schilk, 2024
 * @note Copyright (c) 2024 Philipp Schilk. Released under the MIT license.
 *
 * https://github.com/schilkp/Tonbandgeraet
 */
#ifndef TBAND_PORT_H_

#define tband_portENTER_CRITICAL_FROM_ANY()                                                        \
  unsigned int tband_port_prev_primask;                                                            \
  __asm volatile("mrs   %0, primask  \n\t" /* Save previous primask value */                       \
                 "cpsid i            \n\t" /* Disable interrupts */                                \
                 : "=r"(tband_port_prev_primask)                                                   \
                 :                                                                                 \
                 : "r1", "cc");

#define tband_portEXIT_CRITICAL_FROM_ANY()                                                         \
  __asm volatile("msr   primask, %0  \n\t" : : "r"(tband_port_prev_primask) : "memory");

#endif /* TBAND_PORT_H_ */
