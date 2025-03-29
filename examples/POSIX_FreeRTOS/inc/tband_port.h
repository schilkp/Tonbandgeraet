/*h
 * @file tband_port.h
 * @brief Tonbandgeraet FreeRTOS Cortex M4F port
 * @author Philipp Schilk, 2024
 * @note Copyright (c) 2024 Philipp Schilk. Released under the MIT license.
 *
 * https://github.com/schilkp/Tonbandgeraet
 */
#ifndef TBAND_PORT_H_

#include "FreeRTOS.h"
#include "task.h"

#define tband_portENTER_CRITICAL_FROM_ANY()                                                        \
    taskENTER_CRITICAL();                                                                          \

#define tband_portEXIT_CRITICAL_FROM_ANY()                                                         \
    taskEXIT_CRITICAL();                                                                           \

#endif /* TBAND_PORT_H_ */
