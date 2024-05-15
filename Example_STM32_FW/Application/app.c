/**
 * @file app.c
 * @brief Application Source File.
 * @author Philipp Schilk, 2024
 */
#include "app.h"
#include "main.h"

#define UNUSED(X) (void)X

#define TASK1_STACK_SIZE 200
TaskHandle_t task1 = 0;                    // Task handle.
StaticTask_t task1_buffer = {0};           // Structure that will hold the TCB of the task being created.
StackType_t task1_stack[TASK1_STACK_SIZE]; // Buffer that the task being created will use as its stack.

#define TASK2_STACK_SIZE 200
TaskHandle_t task2 = 0;                    // Task handle.
StaticTask_t task2_buffer = {0};           // Structure that will hold the TCB of the task being created.
StackType_t task2_stack[TASK2_STACK_SIZE]; // Buffer that the task being created will use as its stack.

SemaphoreHandle_t mutex1 = 0;          // Mutex handle
StaticSemaphore_t mutex1_buffer = {0}; // Mutex buffer.

SemaphoreHandle_t mutex2 = 0;          // Mutex handle
StaticSemaphore_t mutex2_buffer = {0}; // Mutex buffer.

#define Q1_QUEUE_LENGTH 10
#define Q1_ITEM_SIZE    sizeof(uint64_t)
QueueHandle_t q1 = 0;                                      // Handle.
static StaticQueue_t q1_buffer = {0};                      // buffer.
static uint8_t q1_storage[Q1_QUEUE_LENGTH * Q1_ITEM_SIZE]; // Storage

// ==== Task 1 =================================================================

void task1_entry(void *args) {
  UNUSED(args);

  int cnt = 0;
  while (1) {
    vTaskDelay(10);
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin); //
    xSemaphoreTake(mutex1, portMAX_DELAY);
    vTaskDelay(20);
    xSemaphoreGive(mutex1);

    xSemaphoreTakeRecursive(mutex2, 0);
    xSemaphoreTakeRecursive(mutex2, 0);
    xSemaphoreTakeRecursive(mutex2, 0);
    xSemaphoreTakeRecursive(mutex2, 0);
    vTaskDelay(1);
    xSemaphoreGiveRecursive(mutex2);
    xSemaphoreGiveRecursive(mutex2);
    xSemaphoreGiveRecursive(mutex2);
    xSemaphoreGiveRecursive(mutex2);

    uint64_t val = 0;
    xQueueSend(q1, &val, 0);
    xQueueSend(q1, &val, 0);
    xQueueSend(q1, &val, 0);
    xQueueSend(q1, &val, 0);
    vTaskDelay(1);
    xQueueReceive(q1, &val, 0);
    xQueueReceive(q1, &val, 0);
    xQueueReceive(q1, &val, 0);
    xQueueReceive(q1, &val, 0);

    if (cnt == 3) {
      vTaskResume(task2);
    }
    cnt++;
  }
}

// ==== Task 2 =================================================================

void task2_entry(void *args) {
  UNUSED(args);
  vTaskSuspend(0);

  while (1) {
    vTaskDelay(5);
    HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);

    xSemaphoreTake(mutex1, portMAX_DELAY);
    HAL_Delay(10);
    xSemaphoreGive(mutex1);
  }
}

// ==== Initialization =========================================================

/**
 * @brief Initialize all FreeRTOS tasks and resources (queues, mutexes, semaphores..)
 *
 * @return 0 if successful.
 */
int rtos_init(void) {

  task1 = xTaskCreateStatic(task1_entry,      /* Function that implements the task. */
                            "mytask1",        /* Text name for the task. */
                            TASK1_STACK_SIZE, /* Number of indexes in the Stack array. */
                            0,                /* Parameter passed into the task. */
                            2,                /* Priority at which the task is created. */
                            task1_stack,      /* Array to use as the task's stack. */
                            &task1_buffer);   /* Variable to hold the task's data structure. */

  if (task1 == 0) return 1;

  task2 = xTaskCreateStatic(task2_entry,      /* Function that implements the task. */
                            "mytask2",        /* Text name for the task. */
                            TASK2_STACK_SIZE, /* Number of indexes in the Stack array. */
                            0,                /* Parameter passed into the task. */
                            1,                /* Priority at which the task is created. */
                            task2_stack,      /* Array to use as the task's stack. */
                            &task2_buffer);   /* Variable to hold the task's data structure. */
  if (task2 == 0) return 1;

  mutex1 = xSemaphoreCreateMutexStatic(&mutex1_buffer);
  if (mutex1 == 0) return 1;
  trace_mutex_name(mutex1, "mymutex1");

  mutex2 = xSemaphoreCreateRecursiveMutexStatic(&mutex2_buffer);
  if (mutex2 == 0) return 1;
  trace_mutex_name(mutex2, "recmutex");

  q1 = xQueueCreateStatic(Q1_QUEUE_LENGTH, Q1_ITEM_SIZE, q1_storage, &q1_buffer);
  if (q1 == 0) return 1;
  trace_queue_name(q1, "my_q1");

  return 0;
}
