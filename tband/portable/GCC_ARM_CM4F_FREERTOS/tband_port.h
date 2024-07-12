/*h
 * @file tband_port.h
 * @brief Tonbandgeraet FreeRTOS Cortex M4F port
 * @author Philipp Schilk, 2024
 * @note Copyright (c) 2024 Philipp Schilk. Released under the MIT license.
 */
#ifndef TBAND_PORT_H_

#include "FreeRTOS.h"
#include "task.h"

#define tband_portENTER_CRITICAL_FROM_ANY()                                                        \
  bool tband_port_in_irq = xPortIsInsideInterrupt();                                               \
  BaseType_t tband_port_key = 0;                                                                   \
  if (tband_port_in_irq) {                                                                         \
    tband_port_key = taskENTER_CRITICAL_FROM_ISR();                                                \
  } else {                                                                                         \
    taskENTER_CRITICAL();                                                                          \
    (void)tband_port_key;                                                                          \
  }

#define tband_portEXIT_CRITICAL_FROM_ANY()                                                         \
  if (tband_port_in_irq) {                                                                         \
    taskEXIT_CRITICAL_FROM_ISR(tband_port_key);                                                    \
  } else {                                                                                         \
    taskEXIT_CRITICAL();                                                                           \
  }

#endif /* TBAND_PORT_H_ */
