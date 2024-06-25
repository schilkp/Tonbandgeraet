/**
 * @file app.c
 * @brief Application Source File.
 * @author Philipp Schilk, 2024
 */
#include "app.h"
#include "main.h"

#define UNUSED(X) (void)X

#define STACK_SIZE 200
TaskHandle_t task1 = 0;              // Task handle.
StaticTask_t task1_buffer = {0};     // Structure that will hold the TCB of the task being created.
StackType_t task1_stack[STACK_SIZE]; // Buffer that the task being created will use as its stack.

TaskHandle_t task2 = 0;              // Task handle.
StaticTask_t task2_buffer = {0};     // Structure that will hold the TCB of the task being created.
StackType_t task2_stack[STACK_SIZE]; // Buffer that the task being created will use as its stack.

TaskHandle_t task3 = 0;              // Task handle.
StaticTask_t task3_buffer = {0};     // Structure that will hold the TCB of the task being created.
StackType_t task3_stack[STACK_SIZE]; // Buffer that the task being created will use as its stack.

SemaphoreHandle_t mutex1 = 0;          // Mutex handle
StaticSemaphore_t mutex1_buffer = {0}; // Mutex buffer.

SemaphoreHandle_t mutex2 = 0;          // Mutex handle
StaticSemaphore_t mutex2_buffer = {0}; // Mutex buffer.

#define Q1_QUEUE_LENGTH 10
#define Q1_ITEM_SIZE    sizeof(uint64_t)
QueueHandle_t q1 = 0;                                      // Handle.
static StaticQueue_t q1_buffer = {0};                      // buffer.
static uint8_t q1_storage[Q1_QUEUE_LENGTH * Q1_ITEM_SIZE]; // Storage

// #define STREAM_BUFFER_SIZE_BYTES 1000
// StreamBufferHandle_t sb1 = 0;
// static uint8_t sb1_storage[STREAM_BUFFER_SIZE_BYTES + 1];
// StaticStreamBuffer_t sb1_buffer = {0};

#define GLOBAL_EVT_MARKER_ID 1
#define GLOBAL_VAL_MARKER_ID 1

// ==== Task 1 =================================================================

void task1_entry(void *args) {
  UNUSED(args);

  frtrace_gather_system_metadata();

  frtrace_task_valmarker_name(0, "T1Val");
  frtrace_task_evtmarker_name(0, "T1Evts");

  // trace_task_is_idle_task(xTaskGetIdleTaskHandle());
  // trace_task_is_timer_task(xTimerGetTimerDaemonTaskHandle());

  int cnt = 0;
  while (1) {

    frtrace_task_evtmarker(0, "PING");

    frtrace_task_evtmarker_begin(0, "Start 1");

    vTaskDelay(10);
    HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin); //
    xSemaphoreTake(mutex1, portMAX_DELAY);
    vTaskDelay(20);
    xSemaphoreGive(mutex1);

    frtrace_task_evtmarker_begin(0, "Start 2");

    xSemaphoreTakeRecursive(mutex2, 0);
    xSemaphoreTakeRecursive(mutex2, 0);
    xSemaphoreTakeRecursive(mutex2, 0);
    xSemaphoreTakeRecursive(mutex2, 0);
    vTaskDelay(1);
    xSemaphoreGiveRecursive(mutex2);
    xSemaphoreGiveRecursive(mutex2);
    xSemaphoreGiveRecursive(mutex2);
    xSemaphoreGiveRecursive(mutex2);

    frtrace_task_evtmarker_end(0);

    uint64_t val = 0;
    xQueueSend(q1, &val, 0);
    xQueueSend(q1, &val, 0);
    xQueueSend(q1, &val, 0);
    xQueueSend(q1, &val, 0);
    xQueueSend(q1, &val, 0);
    vTaskDelay(1);
    xQueueReceive(q1, &val, 0);
    xQueueReceive(q1, &val, 0);
    xQueueReset(q1);

    frtrace_task_valmarker(0, cnt);

    // if (cnt % 5 == 0) {
    //   for (size_t i = 0; i < 20; i++) {
    //     uint8_t data[] = {0, 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    //     // xStreamBufferSend(sb1, data, 16, 0);
    //   }
    // }

    if (cnt == 3) {
      vTaskResume(task2);
    }
    cnt++;

    frtrace_task_evtmarker_end(0);
  }
}

// ==== Task 2 =================================================================

void task2_entry(void *args) {
  UNUSED(args);
  vTaskSuspend(0);

  while (1) {
    vTaskDelay(5);
    HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);

    frtrace_evtmarker(GLOBAL_EVT_MARKER_ID, "T2 about to do something");

    frtrace_evtmarker_begin(GLOBAL_EVT_MARKER_ID, "T2 start something");
    xSemaphoreTake(mutex1, portMAX_DELAY);
    frtrace_evtmarker_end(GLOBAL_EVT_MARKER_ID);
    HAL_Delay(10);
    xSemaphoreGive(mutex1);
  }
}

// ==== Task 2 =================================================================

void task3_entry(void *args) {
  UNUSED(args);

  uint8_t cnt = 0;

  while (1) {
    vTaskDelay(25);
    HAL_Delay(1);
    // xStreamBufferReceive(sb1, rx_buf, 3, portMAX_DELAY);
    frtrace_valmarker(GLOBAL_VAL_MARKER_ID, cnt);
    cnt++;
  }
}

// ==== Initialization =========================================================

/**
 * @brief Initialize all FreeRTOS tasks and resources (queues, mutexes, semaphores..)
 *
 * @return 0 if successful.
 */
int rtos_init(void) {

  task1 = xTaskCreateStatic(task1_entry,    /* Function that implements the task. */
                            "mytask1",      /* Text name for the task. */
                            STACK_SIZE,     /* Number of indexes in the Stack array. */
                            0,              /* Parameter passed into the task. */
                            2,              /* Priority at which the task is created. */
                            task1_stack,    /* Array to use as the task's stack. */
                            &task1_buffer); /* Variable to hold the task's data structure. */

  if (task1 == 0) return 1;

  task2 = xTaskCreateStatic(task2_entry,    /* Function that implements the task. */
                            "mytask2",      /* Text name for the task. */
                            STACK_SIZE,     /* Number of indexes in the Stack array. */
                            0,              /* Parameter passed into the task. */
                            1,              /* Priority at which the task is created. */
                            task2_stack,    /* Array to use as the task's stack. */
                            &task2_buffer); /* Variable to hold the task's data structure. */
  if (task2 == 0) return 1;

  task3 = xTaskCreateStatic(task3_entry,    /* Function that implements the task. */
                            "mytask3",      /* Text name for the task. */
                            STACK_SIZE,     /* Number of indexes in the Stack array. */
                            0,              /* Parameter passed into the task. */
                            1,              /* Priority at which the task is created. */
                            task3_stack,    /* Array to use as the task's stack. */
                            &task3_buffer); /* Variable to hold the task's data structure. */
  if (task3 == 0) return 1;

  mutex1 = xSemaphoreCreateMutexStatic(&mutex1_buffer);
  if (mutex1 == 0) return 1;
  frtrace_mutex_name(mutex1, "mymutex1");

  mutex2 = xSemaphoreCreateRecursiveMutexStatic(&mutex2_buffer);
  if (mutex2 == 0) return 1;
  frtrace_mutex_name(mutex2, "recmutex");

  q1 = xQueueCreateStatic(Q1_QUEUE_LENGTH, Q1_ITEM_SIZE, q1_storage, &q1_buffer);
  if (q1 == 0) return 1;
  frtrace_queue_name(q1, "my_q1");

  // sb1 = xStreamBufferCreateStatic(STREAM_BUFFER_SIZE_BYTES, 1, sb1_storage, &sb1_buffer);
  // if (sb1 == 0) return 1;
  // frtrace_stream_buffer_name(sb1, "sb1");

  if (frtrace_trigger_snapshot() != 0) {
    Error_Handler();
  }

  frtrace_evtmarker_name(GLOBAL_EVT_MARKER_ID, "MyEvts");
  frtrace_valmarker_name(GLOBAL_VAL_MARKER_ID, "MyVals");

  return 0;
}
