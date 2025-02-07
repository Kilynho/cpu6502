# filepath: /home/kilynho/src/cpu6502/Makefile
# Compilador a utilizar
CXX = g++

# Opciones de compilación
CXXFLAGS = -std=c++17 -Wall -Wextra -g

# Nombre del ejecutable principal
TARGET = cpu6502

# Nombre del ejecutable de test
TEST_TARGET = runTests

# Archivos fuente
SOURCES = main_6502.cpp cpu.cpp mem.cpp
TEST_SOURCES = test.cpp cpu.cpp mem.cpp

# Archivos objeto generados a partir de los archivos fuente
OBJECTS = $(SOURCES:.cpp=.o)
TEST_OBJECTS = $(TEST_SOURCES:.cpp=.o)

# Regla principal: compilar todo
all: $(TARGET) $(TEST_TARGET)

# Regla para generar el ejecutable principal a partir de los archivos objeto
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Regla para generar el ejecutable de test a partir de los archivos objeto
$(TEST_TARGET): $(TEST_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lgtest -lgtest_main -pthread

# Regla para generar un archivo objeto a partir de un archivo fuente
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Regla para limpiar los archivos generados (ejecutable y objetos)
clean:
	rm -f $(TARGET) $(TEST_TARGET) $(OBJECTS) $(TEST_OBJECTS)

# Declarar las reglas 'all' y 'clean' como phony (no generan archivos)
.PHONY: all clean