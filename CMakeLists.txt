cmake_minimum_required(VERSION 3.10)

# Nombre del proyecto
project(cpu6502)

# Establecer el estándar de C++
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Habilitar pruebas
enable_testing()

# Añadir Google Test
add_subdirectory(lib/googletest)

# Incluir directorios
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

# Archivos fuente
set(SOURCES main_6502.cpp cpu.cpp mem.cpp)
set(TEST_SOURCES test.cpp)

# Añadir el ejecutable principal
add_executable(cpu6502 ${SOURCES})

# Añadir el ejecutable de test
add_executable(runTests ${TEST_SOURCES} cpu.cpp mem.cpp)

# Enlazar Google Test con tu ejecutable de test
target_link_libraries(runTests gtest gtest_main)

# Añadir pruebas
add_test(NAME runTests COMMAND runTests)