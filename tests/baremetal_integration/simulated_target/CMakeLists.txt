cmake_minimum_required(VERSION 3.12)

project("Tonbandgerät Simulated Bare-metal Target" C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_BUILD_TYPE Debug)

add_executable(simtarget)

add_compile_options(-Wall -Wextra -Wpedantic -fanalyzer -fsanitize=undefined -fsanitize=address -fno-sanitize-recover=all)
add_link_options(-Wall -Wextra -Wpedantic -fanalyzer -fsanitize=undefined -fsanitize=address -fno-sanitize-recover=all)

# Mocking framework:
target_include_directories(simtarget PUBLIC ../../resources/fff)

# tband:
target_include_directories(simtarget PUBLIC ../../../tband/inc/)
target_sources(simtarget PRIVATE ../../../tband/src/tband.c)
target_sources(simtarget PRIVATE ../../../tband/src/tband_backend.c)

# Local files:
target_include_directories(simtarget PUBLIC .)
target_sources(simtarget PRIVATE ./main.c)
target_sources(simtarget PRIVATE ./mocks.c)
