cmake_minimum_required(VERSION 3.10)

project(cpu6502)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Put executables directly in the build directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

enable_testing()

find_package(GTest REQUIRED)

# Source files
set(SOURCES main_6502.cpp cpu.cpp mem.cpp)
set(TEST_SOURCES test.cpp cpu.cpp mem.cpp)

add_executable(cpu6502 ${SOURCES})
add_executable(runTests ${TEST_SOURCES})

# Link using modern target name (GTest::gtest_main)
target_link_libraries(runTests GTest::gtest_main pthread)

add_test(NAME runTests COMMAND runTests)
