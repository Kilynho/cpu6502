cmake_minimum_required(VERSION 3.10)

project(cpu6502)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Put executables directly in the build directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

enable_testing()

# Add subdirectories
add_subdirectory(src)
add_subdirectory(tests)
