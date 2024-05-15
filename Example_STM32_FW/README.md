# FreeRTOS STM32 Project Template

STM32 Project with FreeRTOS integrated manually (i.e. not through CubeMX, and without CMSIS RTOS) to avoid
problems if FreeRTOS ever gets dropped from CubeMX (which has been warned about) and to allow direct editing
of the FreeRTOSConfig.h file (instead of through CubeMX).

## Setup Guide from Scratch:
- Generate a scratch CubeMX project with FreeRTOS enabled.
- Generate a new empty CubeMX project with the following CubeMX settings:
    - HAL Timebase: Any timer except systick
    - Disable code generation for PendSV and SVCall IRQs (They are defind in the FreeRTOS port sources)
    - Set PendSV priority to 15.
    - Set SVCall priority to 0.
- Add the FreeRTOS sources from the scratch project to the new project.
- In the FreeRTOSConfig file, disable dynamic memory allocation.
- In the `stm32xx_it.c` interrupt source file, add the following external definitions
  ```c
  /* CMSIS SysTick interrupt handler prototype */
  extern void SysTick_Handler     (void);
  /* FreeRTOS tick timer interrupt handler prototype */
  extern void xPortSysTickHandler (void);
  ```
  And the the following to the generated systick handler:
  ```c
  SysTick->CTRL;

  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    /* Call tick handler */
    xPortSysTickHandler();
  }
  ```
- At the end of ARM_CM4F/port.c, add the following snippet (taken from CMSIS RTOSv2):
  ```c
  #if (configSUPPORT_STATIC_ALLOCATION == 1)
  /* External Idle and Timer task static memory allocation functions */
  extern void vApplicationGetIdleTaskMemory  (StaticTask_t **ppxIdleTaskTCBBuffer,  StackType_t **ppxIdleTaskStackBuffer,  uint32_t *pulIdleTaskStackSize);
  extern void vApplicationGetTimerTaskMemory (StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize);

  /*
    vApplicationGetIdleTaskMemory gets called when configSUPPORT_STATIC_ALLOCATION
    equals to 1 and is required for static memory allocation support.
  */
  void vApplicationGetIdleTaskMemory (StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
    /* Idle task control block and stack */
    static StaticTask_t Idle_TCB;
    static StackType_t  Idle_Stack[configMINIMAL_STACK_SIZE];

    *ppxIdleTaskTCBBuffer   = &Idle_TCB;
    *ppxIdleTaskStackBuffer = &Idle_Stack[0];
    *pulIdleTaskStackSize   = (uint32_t)configMINIMAL_STACK_SIZE;
  }

  /*
    vApplicationGetTimerTaskMemory gets called when configSUPPORT_STATIC_ALLOCATION
    equals to 1 and is required for static memory allocation support.
  */
  void vApplicationGetTimerTaskMemory (StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize) {
    /* Timer task control block and stack */
    static StaticTask_t Timer_TCB;
    static StackType_t  Timer_Stack[configTIMER_TASK_STACK_DEPTH];

    *ppxTimerTaskTCBBuffer   = &Timer_TCB;
    *ppxTimerTaskStackBuffer = &Timer_Stack[0];
    *pulTimerTaskStackSize   = (uint32_t)configTIMER_TASK_STACK_DEPTH;
  }
  #endif
  ```
- Delete all CMSIS RTOS files
