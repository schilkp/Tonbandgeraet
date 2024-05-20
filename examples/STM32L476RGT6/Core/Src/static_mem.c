#include "FreeRTOS.h"

#if (configSUPPORT_STATIC_ALLOCATION == 1)
/*
  vApplicationGetIdleTaskMemory gets called when configSUPPORT_STATIC_ALLOCATION
  equals to 1 and is required for static memory allocation support.
*/
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize) {
  /* Idle task control block and stack */
  static StaticTask_t Idle_TCB;
  static StackType_t Idle_Stack[configMINIMAL_STACK_SIZE];

  *ppxIdleTaskTCBBuffer = &Idle_TCB;
  *ppxIdleTaskStackBuffer = &Idle_Stack[0];
  *pulIdleTaskStackSize = (uint32_t)configMINIMAL_STACK_SIZE;
}

/*
  vApplicationGetTimerTaskMemory gets called when configSUPPORT_STATIC_ALLOCATION
  equals to 1 and is required for static memory allocation support.
*/
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize) {
  /* Timer task control block and stack */
  static StaticTask_t Timer_TCB;
  static StackType_t Timer_Stack[configTIMER_TASK_STACK_DEPTH];

  *ppxTimerTaskTCBBuffer = &Timer_TCB;
  *ppxTimerTaskStackBuffer = &Timer_Stack[0];
  *pulTimerTaskStackSize = (uint32_t)configTIMER_TASK_STACK_DEPTH;
}
#endif
