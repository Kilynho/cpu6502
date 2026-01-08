# Archivos fuente de la librería

set(LIB_SOURCES
    cpu/cpu.cpp
    cpu/addressing.cpp
    cpu/instructions.cpp
    mem/mem.cpp
    util/logger.cpp
    debugger/debugger.cpp
    scripting/scripting_api.cpp
    devices/c64_io.cpp
    devices/file_device.cpp
    devices/text_screen.cpp
    devices/basic_audio.cpp
    devices/tcp_serial.cpp
    devices/basic_timer.cpp
    interrupt/interrupt_controller.cpp
    gui/emulator_gui.cpp
    cpu/system_map.cpp
)



# Crear la librería estática
add_library(cpu6502_lib STATIC ${LIB_SOURCES})

# Find SDL2 package
find_package(SDL2 REQUIRED)


# Especificar directorios de inclusión para la librería
target_include_directories(cpu6502_lib PUBLIC
    ${PROJECT_SOURCE_DIR}/include
    ${SDL2_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/lib/pybind11/include
)


# Link SDL2 and pybind11 to the library
target_link_libraries(cpu6502_lib PUBLIC ${SDL2_LIBRARIES} pybind11::module)

# (Eliminados todos los ejecutables de ejemplo en examples/)

# Crear el ejecutable principal `emulator` a partir de src/main.cpp
add_executable(emulator ${PROJECT_SOURCE_DIR}/src/main/main.cpp)

# Enlazar el ejecutable con la librería principal
target_link_libraries(emulator cpu6502_lib)

# Especificar directorios de inclusión para el ejecutable
target_include_directories(emulator PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# Establecer el nombre de salida del ejecutable
set_target_properties(emulator PROPERTIES OUTPUT_NAME emulator)
