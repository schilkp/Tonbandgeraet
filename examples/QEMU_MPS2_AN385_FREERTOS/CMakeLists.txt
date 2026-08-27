cmake_minimum_required(VERSION 3.22)

set(CMAKE_C_STANDARD 11)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CMAKE_PROJECT_NAME QEMU_MPS2_AN385_FREERTOS)

include("cmake/gcc-arm-none-eabi.cmake")

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

project(${CMAKE_PROJECT_NAME} LANGUAGES C)

add_executable(${CMAKE_PROJECT_NAME})

add_compile_options(-Wall -Wextra -Wpedantic -Werror)
add_link_options(-Wall -Wextra -Wpedantic -Werror)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # FreeRTOS Core:
    ../../FreeRTOS-Kernel/croutine.c
    ../../FreeRTOS-Kernel/event_groups.c
    ../../FreeRTOS-Kernel/list.c
    ../../FreeRTOS-Kernel/queue.c
    ../../FreeRTOS-Kernel/stream_buffer.c
    ../../FreeRTOS-Kernel/tasks.c
    ../../FreeRTOS-Kernel/timers.c

    # FreeRTOS port:
    ../../FreeRTOS-Kernel/portable/GCC/ARM_CM3/port.c

    # tband:
    ../../tband/src/tband.c
    ../../tband/src/tband_freertos.c
    ../../tband/src/tband_backend.c

    # Application:
    src/startup.c
    src/semihosting.c
    src/main.c
    src/app.c
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # FreeRTOS:
    ../../FreeRTOS-Kernel/include
    ../../FreeRTOS-Kernel/portable/GCC/ARM_CM3

    # tband:
    ../../tband/inc/
    ../../tband/portable/GCC_ARM_CM4F_FREERTOS/

    # Application:
    inc/
)
