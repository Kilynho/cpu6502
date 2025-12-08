cmake_minimum_required(VERSION 3.10)

# Nombre del proyecto
project(cpu6502 VERSION 0.2.0 LANGUAGES CXX)

# Establecer el estándar de C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Opciones de compilación
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Añadir el directorio include para que sea visible en todo el proyecto
include_directories(${PROJECT_SOURCE_DIR}/include)

# Habilitar pruebas
enable_testing()

# Añadir subdirectorios
add_subdirectory(src)
add_subdirectory(tests)

# Instalación (opcional)
# install(TARGETS cpu6502_lib cpu_demo DESTINATION bin)
# install(DIRECTORY include/ DESTINATION include)
