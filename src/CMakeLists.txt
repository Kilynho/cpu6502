# Archivos fuente de la librería
set(LIB_SOURCES
    cpu/cpu.cpp
    cpu/addressing.cpp
    cpu/instructions.cpp
    mem/mem.cpp
    util/logger.cpp
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
