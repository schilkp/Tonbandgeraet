/*h
 * @file frtrace_port.h
 * @brief FreeRTOS tracer Cortex M4F port
 * @author Philipp Schilk, 2024
 */
#ifndef FRTRACE_PORT_H_

#define frtrace_portKERNEL_ENTER_CRITICAL_FROM_ANY()                                               \
  bool frtraceport_in_irq = xPortIsInsideInterrupt();                                              \
  BaseType_t frtraceport_key = 0;                                                                  \
  if (frtraceport_in_irq) {                                                                        \
    frtraceport_key = taskENTER_CRITICAL_FROM_ISR();                                               \
  } else {                                                                                         \
    taskENTER_CRITICAL();                                                                          \
    (void)frtraceport_key;                                                                         \
  }

#define frtrace_portKERNEL_EXIT_CRITICAL_FROM_ANY()                                                \
  if (frtraceport_in_irq) {                                                                        \
    taskEXIT_CRITICAL_FROM_ISR(frtraceport_key);                                                   \
  } else {                                                                                         \
    taskEXIT_CRITICAL();                                                                           \
  }

#endif /* FRTRACE_PORT_H_ */
