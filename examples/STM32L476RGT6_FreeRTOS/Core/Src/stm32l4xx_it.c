/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm32l4xx_it.c
 * @brief   Interrupt Service Routines.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_it.h"
#include "main.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* CMSIS SysTick interrupt handler prototype */
extern void SysTick_Handler(void);
/* FreeRTOS tick timer interrupt handler prototype */
extern void xPortSysTickHandler(void);

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim17;

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
 * @brief This function handles Non maskable interrupt.
 */
void NMI_Handler(void) {
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1) {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
 * @brief This function handles Hard fault interrupt.
 */
void HardFault_Handler(void) {
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1) {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
 * @brief This function handles Memory management fault.
 */
void MemManage_Handler(void) {
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1) {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
 * @brief This function handles Prefetch fault, memory access fault.
 */
void BusFault_Handler(void) {
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1) {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
 * @brief This function handles Undefined instruction or illegal state.
 */
void UsageFault_Handler(void) {
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1) {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
 * @brief This function handles Debug monitor.
 */
void DebugMon_Handler(void) {
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

static const char hex_lut[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                               '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

void dump_hex(uint8_t b) {
  static size_t i = 0;

  char msg[4] = "xx \n";

  uint8_t MSB = (b >> 4) & 0xF;
  uint8_t LSB = (b >> 0) & 0xF;

  msg[0] = hex_lut[MSB];
  msg[1] = hex_lut[LSB];

  if (i % 8 != 7) {
    HAL_UART_Transmit(&huart2, (uint8_t *)msg, 3, 10);
  } else {
    HAL_UART_Transmit(&huart2, (uint8_t *)msg, 4, 10);
  }
  i++;
}

/**
 * @brief This function handles System tick timer.
 */
void SysTick_Handler(void) {
  /* USER CODE BEGIN SysTick_IRQn 0 */
  tband_isr_enter(SysTick_IRQn + 15);
  /* Clear overflow flag */
  SysTick->CTRL;

  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    /* Call tick handler */
    xPortSysTickHandler();
  }

  if (TRACING_SNAPSHOT_DONE) {
    if (tband_tracing_finished()) {
      __disable_irq();

      const uint8_t *buf = (uint8_t *)tband_get_metadata_buf(0);
      size_t len = tband_get_metadata_buf_amnt(0);

      for (size_t i = 0; i < len; i++) {
        dump_hex(buf[i]);
      }

      buf = (uint8_t *)tband_get_core_snapshot_buf(0);
      len = tband_get_core_snapshot_buf_amnt(0);

      for (size_t i = 0; i < len; i++) {
        dump_hex(buf[i]);
      }

      Error_Handler();
    }
  }

  /* USER CODE END SysTick_IRQn 0 */

  /* USER CODE BEGIN SysTick_IRQn 1 */
  tband_isr_exit(SysTick_IRQn + 15);
  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32L4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32l4xx.s).                    */
/******************************************************************************/

/**
 * @brief This function handles TIM1 trigger and commutation interrupts and TIM17 global interrupt.
 */
void TIM1_TRG_COM_TIM17_IRQHandler(void) {
  /* USER CODE BEGIN TIM1_TRG_COM_TIM17_IRQn 0 */

  /* USER CODE END TIM1_TRG_COM_TIM17_IRQn 0 */
  HAL_TIM_IRQHandler(&htim17);
  /* USER CODE BEGIN TIM1_TRG_COM_TIM17_IRQn 1 */

  /* USER CODE END TIM1_TRG_COM_TIM17_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
