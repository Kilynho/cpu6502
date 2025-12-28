# Archivos fuente de la librería
set(LIB_SOURCES
    cpu/cpu.cpp
    cpu/addressing.cpp
    cpu/instructions.cpp
    mem/mem.cpp
    util/logger.cpp
    debugger/debugger.cpp
    scripting/scripting_api.cpp
    devices/apple_io.cpp
    devices/file_device.cpp
    devices/text_screen.cpp
    devices/basic_audio.cpp
    devices/tcp_serial.cpp
    devices/basic_timer.cpp
    devices/pia.cpp
    devices/acia_device.cpp
    devices/via_port_device.cpp
    interrupt/interrupt_controller.cpp
    gui/emulator_gui.cpp
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

# Crear el ejecutable de ejemplo de TextScreen
add_executable(text_screen_demo ${PROJECT_SOURCE_DIR}/examples/text_screen_demo.cpp)

# Enlazar el ejecutable con la librería
target_link_libraries(text_screen_demo cpu6502_lib)

# Especificar directorios de inclusión para el demo
target_include_directories(text_screen_demo PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# Establecer el nombre de salida del ejecutable
set_target_properties(text_screen_demo PROPERTIES OUTPUT_NAME text_screen_demo)

# Crear el ejecutable de GUI demo
add_executable(gui_demo ${PROJECT_SOURCE_DIR}/examples/gui_demo.cpp)

# Enlazar el ejecutable con la librería y SDL2
target_link_libraries(gui_demo cpu6502_lib ${SDL2_LIBRARIES})

# Especificar directorios de inclusión para el demo
target_include_directories(gui_demo PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${SDL2_INCLUDE_DIRS}
)

# Establecer el nombre de salida del ejecutable
set_target_properties(gui_demo PROPERTIES OUTPUT_NAME gui_demo)

# Crear el ejecutable de screenshot demo
add_executable(screenshot_demo ${PROJECT_SOURCE_DIR}/examples/screenshot_demo.cpp)

# Enlazar el ejecutable con la librería y SDL2
target_link_libraries(screenshot_demo cpu6502_lib ${SDL2_LIBRARIES})

# Especificar directorios de inclusión para el demo
target_include_directories(screenshot_demo PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${SDL2_INCLUDE_DIRS}
)

# Establecer el nombre de salida del ejecutable
set_target_properties(screenshot_demo PROPERTIES OUTPUT_NAME screenshot_demo)

# Crear el ejecutable de audio demo
add_executable(audio_demo ${PROJECT_SOURCE_DIR}/examples/audio_demo.cpp)

# Enlazar el ejecutable con la librería
target_link_libraries(audio_demo cpu6502_lib)

# Especificar directorios de inclusión para el demo
target_include_directories(audio_demo PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# Establecer el nombre de salida del ejecutable
set_target_properties(audio_demo PROPERTIES OUTPUT_NAME audio_demo)

# Crear el ejecutable de TCP Serial demo
add_executable(tcp_serial_demo ${PROJECT_SOURCE_DIR}/examples/tcp_serial_demo.cpp)

# Enlazar el ejecutable con la librería
target_link_libraries(tcp_serial_demo cpu6502_lib)

# Especificar directorios de inclusión para el demo
target_include_directories(tcp_serial_demo PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# Establecer el nombre de salida del ejecutable
set_target_properties(tcp_serial_demo PROPERTIES OUTPUT_NAME tcp_serial_demo)

# Crear el ejecutable de Interrupt demo
add_executable(interrupt_demo ${PROJECT_SOURCE_DIR}/examples/interrupt_demo.cpp)

# Enlazar el ejecutable con la librería
target_link_libraries(interrupt_demo cpu6502_lib)

# Especificar directorios de inclusión para el demo
target_include_directories(interrupt_demo PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# Establecer el nombre de salida del ejecutable
set_target_properties(interrupt_demo PROPERTIES OUTPUT_NAME interrupt_demo)

# Crear el ejecutable de WOZMON demo
add_executable(wozmon_demo ${PROJECT_SOURCE_DIR}/examples/wozmon_demo.cpp)

# Enlazar el ejecutable con la librería
target_link_libraries(wozmon_demo cpu6502_lib)

# Especificar directorios de inclusión para el demo
target_include_directories(wozmon_demo PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# Establecer el nombre de salida del ejecutable
set_target_properties(wozmon_demo PROPERTIES OUTPUT_NAME wozmon_demo)

# Assemble canonical WOZMON and inject into src/wozmon/rom.bin before building demos
set(WOZMON_SRC ${PROJECT_SOURCE_DIR}/src/wozmon/wozmon.asm)
set(WOZMON_OBJ ${CMAKE_BINARY_DIR}/wozmon/wozmon.o)
set(WOZMON_BIN ${CMAKE_BINARY_DIR}/wozmon/wozmon.bin)

add_custom_command(
    OUTPUT ${WOZMON_BIN}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/wozmon
    COMMAND ca65 -D kilynho ${WOZMON_SRC} -o ${WOZMON_OBJ}
    COMMAND /bin/sh -c "ld65 -C ${PROJECT_SOURCE_DIR}/msbasic/wozmon.cfg ${WOZMON_OBJ} -o ${WOZMON_BIN}"
    COMMAND ${CMAKE_COMMAND} -E env python3 ${PROJECT_SOURCE_DIR}/tools/inject_wozmon.py ${WOZMON_BIN}
    COMMENT "Assembling WOZMON and injecting into src/wozmon/rom.bin"
    VERBATIM
)

add_custom_target(wozmon_build DEPENDS ${WOZMON_BIN})
add_dependencies(wozmon_demo wozmon_build)

# Crear el ejecutable de WOZMON Serial demo
add_executable(wozmon_serial_demo ${PROJECT_SOURCE_DIR}/examples/wozmon_serial_demo.cpp)

# Enlazar el ejecutable con la librería
target_link_libraries(wozmon_serial_demo cpu6502_lib)

# Especificar directorios de inclusión para el demo
target_include_directories(wozmon_serial_demo PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# Establecer el nombre de salida del ejecutable
set_target_properties(wozmon_serial_demo PROPERTIES OUTPUT_NAME wozmon_serial_demo)
# Crear el ejecutable de BASIC trace demo
add_executable(basic_trace_demo ${PROJECT_SOURCE_DIR}/examples/basic_trace_demo.cpp)

# Enlazar el ejecutable con la librería
target_link_libraries(basic_trace_demo cpu6502_lib)

# Especificar directorios de inclusión para el demo
target_include_directories(basic_trace_demo PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# Establecer el nombre de salida del ejecutable
set_target_properties(basic_trace_demo PROPERTIES OUTPUT_NAME basic_trace_demo)
