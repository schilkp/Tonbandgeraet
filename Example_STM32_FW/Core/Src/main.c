/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "main.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS_trace.pb.h"
#include "app.h"
#include "pb_encode.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

static void report_result(char *name, uint32_t *start_t, uint32_t *stop_t, uint32_t *msg_len,
                          size_t s) {
  printf("-- Result of %s --\r\n", name);
  for (size_t i = 0; i < s; i++) {
    printf("  CNT: %" PRIu32 " LEN: %" PRIu32 "\r\n", stop_t[i] - start_t[i], msg_len[i]);
  }
}

static inline size_t encode_u8(uint8_t *buf, uint8_t val) {
  buf[0] = val;
  return 1;
}

static inline size_t encode_u32(uint8_t *buf, uint32_t val) {
  for (size_t i = 0; i < 5; i++) {
    uint8_t bits = val & 0x7F;
    val = val >> 7;
    if (val == 0) {
      buf[i] = bits | 0x00;
      return i + 1;
    } else {
      buf[i] = bits | 0x80;
    }
  }
  return 5;
}

static inline size_t encode_u64(uint8_t *buf, uint64_t val) {
  for (size_t i = 0; i < 10; i++) {
    uint8_t bits = val & 0x7F;
    val = val >> 7;
    if (val == 0) {
      buf[i] = bits | 0x00;
      return i + 1;
    } else {
      buf[i] = bits | 0x80;
    }
  }
  return 10;
}

static inline size_t encode_str(uint8_t *buf, char *str) {
  size_t len = 0;

  while (*str != 0) {
    buf[len] = *str;
    len++;
    str++;
  }

  return len;
}

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  // Start tim2 (for tracing timestamps):
  HAL_TIM_Base_Start(&htim2);

  NVIC_SetPriority(SVCall_IRQn, 0U);

  // Start DWT
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  printf("START!\r\n");

  static const size_t CNT = 10;
  uint32_t start_t[CNT];
  uint32_t stop_t[CNT];
  uint32_t msg_len[CNT];

  volatile uint8_t OUT_BUF[100];
  volatile uint64_t ts = 0x12301231;
  volatile uint32_t id = 0x2123;

  // Nanopb: Empty evt (ts + kind) (task_delay)
  memset(start_t, 0, sizeof(start_t));
  memset(stop_t, 0, sizeof(stop_t));
  memset(msg_len, 0, sizeof(msg_len));

  for (size_t i = 0; i < CNT; i++) {
    start_t[i] = DWT->CYCCNT;
    uint8_t buf[100] = {0};
    TraceEvent evt = {
        .ts_ns = ts,
        .which_event = TraceEvent_task_delay_tag,
        .event.task_delay = true, // Value is dont-care
    };
    pb_ostream_t ostream = pb_ostream_from_buffer(buf, 100);
    (void)pb_encode(&ostream, TraceEvent_fields, &evt);
    size_t proto_len = ostream.bytes_written;
    msg_len[i] = proto_len;
    for (size_t j = 0; j < proto_len; j++) {
      OUT_BUF[j] = buf[j];
    }
    stop_t[i] = DWT->CYCCNT;
  }

  report_result("nanopb empty", start_t, stop_t, msg_len, CNT);

  // Nanopb: Basic evt (ts + kind + id)
  memset(start_t, 0, sizeof(start_t));
  memset(stop_t, 0, sizeof(stop_t));
  memset(msg_len, 0, sizeof(msg_len));

  for (size_t i = 0; i < CNT; i++) {
    start_t[i] = DWT->CYCCNT;
    uint8_t buf[100] = {0};
    TraceEvent evt = {
        .ts_ns = ts,
        .which_event = TraceEvent_task_create_tag,
        .event.task_create = id,
    };
    pb_ostream_t ostream = pb_ostream_from_buffer(buf, 100);
    (void)pb_encode(&ostream, TraceEvent_fields, &evt);
    size_t proto_len = ostream.bytes_written;
    msg_len[i] = proto_len;
    for (size_t j = 0; j < proto_len; j++) {
      OUT_BUF[j] = buf[j];
    }
    stop_t[i] = DWT->CYCCNT;
  }

  report_result("nanopb basic", start_t, stop_t, msg_len, CNT);

  // Nanopb: String evt (ts + kind + id + string)
  memset(start_t, 0, sizeof(start_t));
  memset(stop_t, 0, sizeof(stop_t));
  memset(msg_len, 0, sizeof(msg_len));

  for (size_t i = 0; i < CNT; i++) {
    start_t[i] = DWT->CYCCNT;
    uint8_t buf[100] = {0};
    TraceEvent evt = {
        .ts_ns = ts,
        .which_event = TraceEvent_task_name_tag,
        .event.task_name = {.id = id, .name = "name"},
    };
    pb_ostream_t ostream = pb_ostream_from_buffer(buf, 100);
    (void)pb_encode(&ostream, TraceEvent_fields, &evt);
    size_t proto_len = ostream.bytes_written;
    msg_len[i] = proto_len;
    for (size_t j = 0; j < proto_len; j++) {
      OUT_BUF[j] = buf[j];
    }
    stop_t[i] = DWT->CYCCNT;
  }

  report_result("nanopb string", start_t, stop_t, msg_len, CNT);

  // Custom: Empty evt (ts + kind) (task_delay)
  memset(start_t, 0, sizeof(start_t));
  memset(stop_t, 0, sizeof(stop_t));
  memset(msg_len, 0, sizeof(msg_len));

  for (size_t i = 0; i < CNT; i++) {
    start_t[i] = DWT->CYCCNT;
    uint8_t buf[100] = {0};
    size_t len = 0;

    len += encode_u8(buf + len, 19);
    len += encode_u64(buf + len, ts);

    msg_len[i] = len;
    for (size_t j = 0; j < len; j++) {
      OUT_BUF[j] = buf[j];
    }
    stop_t[i] = DWT->CYCCNT;
  }

  report_result("custom empty", start_t, stop_t, msg_len, CNT);

  // Custom: Basic evt (ts + kind + id)
  memset(start_t, 0, sizeof(start_t));
  memset(stop_t, 0, sizeof(stop_t));
  memset(msg_len, 0, sizeof(msg_len));

  for (size_t i = 0; i < CNT; i++) {
    start_t[i] = DWT->CYCCNT;
    uint8_t buf[100] = {0};
    size_t len = 0;

    len += encode_u8(buf + len, 19);  // kind
    len += encode_u64(buf + len, ts); // ts
    len += encode_u32(buf + len, id); // id

    msg_len[i] = len;
    for (size_t j = 0; j < len; j++) {
      OUT_BUF[j] = buf[j];
    }
    stop_t[i] = DWT->CYCCNT;
  }

  report_result("custom basic", start_t, stop_t, msg_len, CNT);

  // Custom: String evt (ts + kind + id + string)
  memset(start_t, 0, sizeof(start_t));
  memset(stop_t, 0, sizeof(stop_t));
  memset(msg_len, 0, sizeof(msg_len));

  for (size_t i = 0; i < CNT; i++) {
    start_t[i] = DWT->CYCCNT;
    uint8_t buf[100] = {0};
    size_t len = 0;

    len += encode_u8(buf + len, 19);      // kind
    len += encode_u64(buf + len, ts);     // ts
    len += encode_u32(buf + len, id);     // id
    len += encode_str(buf + len, "name"); // id

    msg_len[i] = len;
    for (size_t j = 0; j < len; j++) {
      OUT_BUF[j] = buf[j];
    }
    stop_t[i] = DWT->CYCCNT;
  }

  report_result("custom string", start_t, stop_t, msg_len, CNT);

  // if (rtos_init() != 0) Error_Handler();

  // trace_isr_name(SysTick_IRQn, "Tick");

  // vTaskStartScheduler();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
   */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType =
      RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
    Error_Handler();
  }
}

/**
 * @brief TIM2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM2_Init(void) {

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV4;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void) {

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED2_Pin */
  GPIO_InitStruct.Pin = LED2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LED1_Pin */
  GPIO_InitStruct.Pin = LED1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED1_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM17 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM17) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
