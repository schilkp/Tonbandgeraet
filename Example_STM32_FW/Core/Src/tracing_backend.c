/**
 * @file tracing_backend.c
 * @brief Tracing Backend
 * @author Philipp Schilk, 2024
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// FreeRTOS
#include "FreeRTOS.h"
#include "main.h"
#include "portmacro.h"
#include "task.h"

#define TRACE_BUFFER_SIZE 20000

volatile uint8_t TRACE_BUFFER[TRACE_BUFFER_SIZE] = {0};
volatile size_t trace_buffer_at = 1;

char hex_lut[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

bool traceport_handle_raw_evt(uint8_t *buf, size_t len) {

  bool did_drop = false;

  // Enter critical section
  bool in_irq = xPortIsInsideInterrupt();
  BaseType_t key = 0;

  if (in_irq) {
    key = taskENTER_CRITICAL_FROM_ISR();
  } else {
    taskENTER_CRITICAL();
  }

  if (trace_buffer_at + len < TRACE_BUFFER_SIZE) {
    for (size_t i = 0; i < len; i++) {
      TRACE_BUFFER[trace_buffer_at + i] = buf[i];
    }
    trace_buffer_at += len;

  } else {
    did_drop = true;

    for (size_t i = 0; i < TRACE_BUFFER_SIZE; i++) {
      char msg[4] = "xx \n";

      uint8_t MSB = (TRACE_BUFFER[i] >> 4) & 0xF;
      uint8_t LSB = (TRACE_BUFFER[i] >> 0) & 0xF;

      msg[0] = hex_lut[MSB];
      msg[1] = hex_lut[LSB];

      if (i % 8 != 7) {
        HAL_UART_Transmit(&huart2, (uint8_t *)msg, 3, 10);
      } else {
        HAL_UART_Transmit(&huart2, (uint8_t *)msg, 4, 10);
      }
    }

    Error_Handler();
  }

  if (in_irq) {
    taskEXIT_CRITICAL_FROM_ISR(key);
  } else {
    taskEXIT_CRITICAL();
  }

  return did_drop;
}

uint64_t traceport_timestamp_ns(void) {
  // Timer value (counting at 80/4 = 20Mhz)
  uint64_t t = htim2.Instance->CNT;
  return t * 50;
}
