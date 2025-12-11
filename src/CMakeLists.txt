# Archivos fuente de la librería
set(LIB_SOURCES
    cpu/cpu.cpp
    cpu/addressing.cpp
    cpu/instructions.cpp
    mem/mem.cpp
    util/logger.cpp
    devices/apple_io.cpp
    devices/file_device.cpp
)

# Crear la librería estática
add_library(cpu6502_lib STATIC ${LIB_SOURCES})

# Especificar directorios de inclusión para la librería
target_include_directories(cpu6502_lib PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

# Crear el ejecutable de demostración
add_executable(cpu_demo main/cpu_demo.cpp)

# Enlazar el ejecutable con la librería
target_link_libraries(cpu_demo cpu6502_lib)

# Establecer el nombre de salida del ejecutable
set_target_properties(cpu_demo PROPERTIES OUTPUT_NAME cpu_demo)

# Crear el ejecutable de ejemplo de Apple IO
add_executable(apple_io_demo ${PROJECT_SOURCE_DIR}/examples/apple_io_demo.cpp)

# Enlazar el ejecutable con la librería
target_link_libraries(apple_io_demo cpu6502_lib)

# Especificar directorios de inclusión para el demo
target_include_directories(apple_io_demo PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# Establecer el nombre de salida del ejecutable
set_target_properties(apple_io_demo PROPERTIES OUTPUT_NAME apple_io_demo)

# Crear el ejecutable de ejemplo de FileDevice
add_executable(file_device_demo ${PROJECT_SOURCE_DIR}/examples/file_device_demo.cpp)

# Enlazar el ejecutable con la librería
target_link_libraries(file_device_demo cpu6502_lib)

# Especificar directorios de inclusión para el demo
target_include_directories(file_device_demo PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# Establecer el nombre de salida del ejecutable
set_target_properties(file_device_demo PROPERTIES OUTPUT_NAME file_device_demo)
