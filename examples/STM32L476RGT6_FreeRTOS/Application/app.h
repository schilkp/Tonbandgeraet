/**
 * @file app.h
 * @brief Application Header.
 * @author Philipp Schilk, 2024
 */
#ifndef APP_H_
#define APP_H_

/**
 * @brief Initialize all FreeRTOS tasks and resources (queues, mutexes, semaphores..)
 *
 * @return 0 if successful.
 */
int rtos_init(void);

#endif /* APP_H_ */
