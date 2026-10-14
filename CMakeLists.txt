cmake_minimum_required(VERSION 3.21)
project(Calculator C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(calculator_core bigint.c calculator.c)
target_include_directories(calculator_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
add_executable(Calculator main.c)
target_link_libraries(Calculator PRIVATE calculator_core)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(calculator_core PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(Calculator PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(calculator_tests tests/test_calculator.c)
    target_link_libraries(calculator_tests PRIVATE calculator_core)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(calculator_tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    add_test(NAME arithmetic COMMAND calculator_tests)
    add_test(NAME file_io COMMAND ${CMAKE_COMMAND}
            -DCALCULATOR=$<TARGET_FILE:Calculator>
            -DTEST_DIRECTORY=${CMAKE_CURRENT_BINARY_DIR}/file-tests
            -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_files.cmake)
endif()
