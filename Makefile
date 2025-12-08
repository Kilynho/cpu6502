# Compilador a utilizar
CXX = g++

# Opciones de compilación
CXXFLAGS = -std=c++17 -Wall -Wextra -g

# Directorio de salida de binarios/objetos
BINDIR := build
OBJDIR := $(BINDIR)/obj

# Nombre del ejecutable principal (en build/)
TARGET := $(BINDIR)/cpu6502

# Nombre del ejecutable de test (en build/)
TEST_TARGET := $(BINDIR)/runTests

# Archivos fuente
SOURCES = main_6502.cpp cpu.cpp mem.cpp
TEST_SOURCES = test.cpp cpu.cpp mem.cpp

# Archivos objeto generados a partir de los archivos fuente (en build/obj)
OBJECTS := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SOURCES))
TEST_OBJECTS := $(patsubst %.cpp,$(OBJDIR)/%.o,$(TEST_SOURCES))

# Regla principal: compilar todo
all: $(TARGET) $(TEST_TARGET)

# Regla para generar el ejecutable principal a partir de los archivos objeto
$(TARGET): $(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Regla para generar el ejecutable de test a partir de los archivos objeto
$(TEST_TARGET): $(TEST_OBJECTS)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lgtest -lgtest_main -pthread

# Regla para generar un archivo objeto a partir de un archivo fuente (coloca en build/obj)
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Regla para limpiar los archivos generados (ejecutable y objetos)
clean:
	rm -rf $(BINDIR)

# Declarar las reglas 'all' y 'clean' como phony (no generan archivos)
.PHONY: all clean