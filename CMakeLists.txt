cmake_minimum_required(VERSION 3.10)

# Nombre del proyecto
project(cpu6502)

# Establecer el estándar de C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Habilitar pruebas
enable_testing()

# Buscar Google Test
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Archivos fuente
set(SOURCES main_6502.cpp cpu.cpp mem.cpp)
set(TEST_SOURCES test.cpp)

# Añadir el ejecutable principal
add_executable(cpu6502 ${SOURCES})

# Añadir el ejecutable de test
add_executable(runTests ${TEST_SOURCES} cpu.cpp mem.cpp)

# Enlazar Google Test con tu ejecutable de test
target_link_libraries(runTests ${GTEST_LIBRARIES} pthread)

# Añadir pruebas
add_test(NAME runTests COMMAND runTests)