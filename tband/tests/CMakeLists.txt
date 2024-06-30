cmake_minimum_required(VERSION 3.12)

project("Tonbandgerät Tests" C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_BUILD_TYPE Debug)

add_compile_definitions(UNITY_INCLUDE_EXEC_TIME)
add_compile_options(-Wall -Wextra -Wpedantic -fanalyzer -fsanitize=undefined -fsanitize=address -fno-sanitize-recover=all)
add_link_options(-Wall -Wextra -Wpedantic -fanalyzer -fsanitize=undefined -fsanitize=address -fno-sanitize-recover=all)

# Compile test framework to static lib:
add_library(TestFramework STATIC Unity/unity.c)
target_include_directories(TestFramework PUBLIC Unity)

# Util function to add a new test file.
function(add_unit_test TEST_FOLDER)
    # New test executable:
    add_executable(${TEST_FOLDER})
    target_sources(${TEST_FOLDER} PRIVATE ${TEST_FOLDER}/test.c)
    target_include_directories(${TEST_FOLDER} PUBLIC ${TEST_FOLDER})

    # Unity test framework:
    target_link_libraries(${TEST_FOLDER} PUBLIC TestFramework)

    # FreeRTOS headers:
    target_include_directories(${TEST_FOLDER} PUBLIC ../../FreeRTOS-Kernel/include)

    # tband:
    target_include_directories(${TEST_FOLDER} PUBLIC ../inc/)
    target_sources(${TEST_FOLDER} PRIVATE ../src/tband.c)
    target_sources(${TEST_FOLDER} PRIVATE ../src/tband_backend.c)

    # Testing stubs/mocks:
    target_include_directories(${TEST_FOLDER} PUBLIC mocks/inc/)
    target_sources(${TEST_FOLDER} PRIVATE mocks/src/function_mocks.c)

endfunction()

add_unit_test(test_cobs_framing)
add_unit_test(test_varint_enc)
add_unit_test(test_encoding_funcs)
