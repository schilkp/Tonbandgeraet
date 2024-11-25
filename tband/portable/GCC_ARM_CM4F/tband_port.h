/**
 * @file tband_port.h
 * @brief Tonbandgeraet bare-metal Cortex M4F port
 * @author Philipp Schilk, 2024
 * @note Copyright (c) 2024 Philipp Schilk. Released under the MIT license.
 *
 * https://github.com/schilkp/Tonbandgeraet
 */
#ifndef TBAND_PORT_H_

#ifndef tband_portMAX_INTERRUPT_PRIORITY
#define tband_portMAX_INTERRUPT_PRIORITY (0x00)
#endif

#define tband_portENTER_CRITICAL_FROM_ANY()                                                        \
  unsigned int tband_port_prev_basepri;                                                            \
  __asm volatile("mrs   %0, basepri      \n\t" /* Save previous basepri value */                   \
                 "mov   r1, %1           \n\t" /* Move new basepri value to r1 */                  \
                 "msr   basepri_max, r1  \n\t" /* Apply new basepri value from r1 */               \
                 : "=r"(tband_port_prev_basepri)                                                   \
                 : "i"(tband_portMAX_INTERRUPT_PRIORITY)                                           \
                 : "r1", "cc");

#define tband_portEXIT_CRITICAL_FROM_ANY()                                                         \
  __asm volatile("msr   basepri, %0  \n\t" : : "r"(tband_port_prev_basepri) :);

#endif /* TBAND_PORT_H_ */
