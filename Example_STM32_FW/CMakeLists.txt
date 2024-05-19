cmake_minimum_required(VERSION 3.22)

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    # set(CMAKE_BUILD_TYPE "Debug")
    set(CMAKE_BUILD_TYPE "Release")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME FreeRTOS_Integration)

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
    Libs/FreeRTOS/croutine.c
    Libs/FreeRTOS/event_groups.c
    Libs/FreeRTOS/list.c
    Libs/FreeRTOS/queue.c
    Libs/FreeRTOS/stream_buffer.c
    Libs/FreeRTOS/tasks.c
    Libs/FreeRTOS/timers.c

    # FreeRTOS port:
    Libs/FreeRTOS/portable/GCC/ARM_CM4F/port.c
    Core/Src/static_mem.c

    # Tracing:
    ../tracer/protos/pb_common.c
    ../tracer/protos/pb_encode.c
    ../tracer/protos/FreeRTOS_trace.pb.c

    # Tracing backend:
    # Core/Src/tracing_backend.c

    # Application:
    # Application/app.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # FreeRTOS:
    Libs/FreeRTOS/include
    Libs/FreeRTOS/portable/GCC/ARM_CM4F

    # Tracing:
    ../tracer/
    ../tracer/protos

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
