
cmake_minimum_required(VERSION 3.16)

project(TinyC
    VERSION 1.0.0
    DESCRIPTION "A miniature C++17 compiler and bytecode virtual machine"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Core compiler implementation.
add_library(tinyc_core
    src/lexer.cpp
    src/parser.cpp
    src/codegen.cpp
    src/vm.cpp
    src/optimizer.cpp
)

target_include_directories(tinyc_core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Enable useful compiler warnings.
if(MSVC)
    target_compile_options(tinyc_core PRIVATE /W4)
else()
    target_compile_options(
        tinyc_core PRIVATE
        -Wall -Wextra -Wpedantic
    )
endif()

# Command-line compiler.
add_executable(tinyc src/main.cpp)
target_link_libraries(tinyc PRIVATE tinyc_core)

# Automated test executable.
add_executable(
    compiler_tests
    tests/compiler_tests.cpp
)

target_link_libraries(compiler_tests PRIVATE tinyc_core)

# Register tests with CTest.
enable_testing()

add_test(
    NAME compiler_tests
    COMMAND compiler_tests
)
