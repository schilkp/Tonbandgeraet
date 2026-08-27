set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_PROCESSOR          arm)

set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_C_COMPILER_ID GNU)

# arm-none-eabi- must be part of path environment
set(TOOLCHAIN_PREFIX                arm-none-eabi-)

set(CMAKE_C_COMPILER                ${TOOLCHAIN_PREFIX}gcc)

set(CMAKE_EXECUTABLE_SUFFIX_C       ".elf")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Target flags
set(TARGET_FLAGS "-mthumb -mcpu=cortex-m3")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TARGET_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -fdata-sections -ffunction-sections")
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g3")
endif()
if(CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -g0")
endif()

set(CMAKE_C_LINK_FLAGS "${TARGET_FLAGS}")
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -T \"${CMAKE_SOURCE_DIR}/mps2_an385.ld\"")
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -nostartfiles --specs=nano.specs --specs=nosys.specs")
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,-Map=${CMAKE_PROJECT_NAME}.map -Wl,--gc-sections")
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--print-memory-usage")
