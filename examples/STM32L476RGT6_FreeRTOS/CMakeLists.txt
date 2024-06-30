cmake_minimum_required(VERSION 3.22)

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME STM32L476RGT6_FreeRTOS)

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
)

# Add sources to executable
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
    ../../FreeRTOS-Kernel/portable/GCC/ARM_CM4F/port.c
    Core/Src/static_mem.c

    # tband:
    ../../tband/src/tband.c
    ../../tband/src/tband_backend.c

    # Application:
    Application/app.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # FreeRTOS:
    ../../FreeRTOS-Kernel/include
    ../../FreeRTOS-Kernel/portable/GCC/ARM_CM4F/

    # tband:
    ../../tband/inc/
    ../../tband/portable/GCC_ARM_CM4F_FREERTOS/

    # Application:
    Application
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-comment")
