# Buscar Google Test
# Primero intentar con el paquete del sistema
find_package(GTest QUIET)

if(NOT GTest_FOUND)
    # Si no se encuentra, usar el submodulo local
    message(STATUS "Google Test no encontrado en el sistema, usando submodulo local")
    
    # Añadir googletest como subdirectorio
    add_subdirectory(${PROJECT_SOURCE_DIR}/lib/googletest ${CMAKE_BINARY_DIR}/googletest EXCLUDE_FROM_ALL)
    
    # Crear alias para mantener compatibilidad
    if(TARGET gtest)
        add_library(GTest::GTest ALIAS gtest)
        add_library(GTest::Main ALIAS gtest_main)
    endif()
endif()

# Archivos fuente de test
set(TEST_SOURCES
    test_main.cpp
    instruction_handlers_test.cpp
    test_apple_io.cpp
    test_file_device.cpp
)

# Crear ejecutable de test
add_executable(runTests ${TEST_SOURCES})

# Enlazar con la librería y Google Test
target_link_libraries(runTests 
    cpu6502_lib
    GTest::GTest
    GTest::Main
    pthread
)

# Especificar directorios de inclusión
target_include_directories(runTests PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# Añadir tests a CTest
add_test(NAME cpu6502_tests COMMAND runTests)

# Propiedades adicionales para el ejecutable de test
set_target_properties(runTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
