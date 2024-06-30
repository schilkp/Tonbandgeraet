/**
 * @file portmacro.h
 * @brief FreeRTOS port testing stubs
 * @author Philipp Schilk, 2024
 */
#ifndef PORTMACRO_H_
#define PORTMACRO_H_

#define portBYTE_ALIGNMENT (4)
#define BaseType_t         int
#define UBaseType_t        unsigned int
#define StackType_t        int
#define TickType_t         int

UBaseType_t mock_port_get_core_id(void);
#define portGET_CORE_ID() mock_port_get_core_id()

#endif /* PORTMACRO_H_ */
