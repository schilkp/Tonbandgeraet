cmake_minimum_required(VERSION 3.12)

project("Tracer Tests" C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_BUILD_TYPE Debug)

add_compile_definitions(UNITY_INCLUDE_EXEC_TIME)
add_compile_options(-Wall -Wextra -Wpedantic)

# Compile test framework to static lib:
add_library(TestFramework STATIC Unity/unity.c)
target_include_directories(TestFramework PUBLIC Unity)

# Util function to add a new test file.
function(add_unit_test TEST_SOURCE)
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WLE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} PUBLIC TestFramework)
    target_include_directories(${TEST_NAME} PUBLIC ../core/)
endfunction()

add_unit_test(test_cobs_framing.c)
add_unit_test(test_varint_enc.c)
