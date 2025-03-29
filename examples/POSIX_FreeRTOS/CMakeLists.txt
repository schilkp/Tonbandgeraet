cmake_minimum_required(VERSION 3.22)

set(CMAKE_C_STANDARD 11)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CMAKE_PROJECT_NAME POSIX_FreeRTOS)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

project(${CMAKE_PROJECT_NAME})
enable_language(C)

add_executable(${CMAKE_PROJECT_NAME})

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
    ../../FreeRTOS-Kernel/portable/ThirdParty/GCC/Posix/port.c
    ../../FreeRTOS-Kernel/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c

    # tband:
    ../../tband/src/tband.c
    ../../tband/src/tband_freertos.c
    ../../tband/src/tband_backend.c

    # Application:
    src/main.c
    src/app.c
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # FreeRTOS:
    ../../FreeRTOS-Kernel/include
    ../../FreeRTOS-Kernel/portable/ThirdParty/GCC/Posix

    # tband:
    ../../tband/inc/

    # Application:
    inc/
)
