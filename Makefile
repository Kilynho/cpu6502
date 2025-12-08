# Makefile wrapper para CMake
# Este Makefile proporciona compatibilidad con el flujo de trabajo anterior
# pero utiliza CMake internamente para la compilación

BUILD_DIR = build

# Regla principal: compilar todo con CMake
all: cmake_build

# Configurar y compilar con CMake
cmake_build:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. && $(MAKE)
	@echo ""
	@echo "==================================="
	@echo "Build completado exitosamente!"
	@echo "Ejecutables:"
	@echo "  - Librería: $(BUILD_DIR)/src/libcpu6502_lib.a"
	@echo "  - Demo: $(BUILD_DIR)/src/cpu_demo"
	@echo "  - Tests: $(BUILD_DIR)/runTests"
	@echo "==================================="

# Ejecutar tests
test: cmake_build
	@echo "Ejecutando tests con CTest..."
	@cd $(BUILD_DIR) && ctest --output-on-failure

# Ejecutar tests directamente
runTests: cmake_build
	@$(BUILD_DIR)/runTests

# Ejecutar el demo
demo: cmake_build
	@$(BUILD_DIR)/src/cpu_demo

# Limpiar archivos generados
clean:
	@rm -rf $(BUILD_DIR)
	@rm -f *.o cpu6502 main_6502 runTests cpu_demo
	@echo "Limpieza completada"

# Recompilar desde cero
rebuild: clean all

# Ayuda
help:
	@echo "Makefile para CPU 6502 Emulator"
	@echo ""
	@echo "Objetivos disponibles:"
	@echo "  all       - Compilar todo el proyecto (default)"
	@echo "  test      - Compilar y ejecutar tests con CTest"
	@echo "  runTests  - Compilar y ejecutar tests directamente"
	@echo "  demo      - Compilar y ejecutar el demo"
	@echo "  clean     - Limpiar archivos generados"
	@echo "  rebuild   - Limpiar y recompilar desde cero"
	@echo "  help      - Mostrar esta ayuda"

# Declarar reglas phony
.PHONY: all cmake_build test runTests demo clean rebuild help
