# CPU 6502 Emulator

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![C++](https://img.shields.io/badge/C++-17-blue.svg)
![CI](https://github.com/Kilynho/cpu6502/workflows/CI/badge.svg)

Una implementación moderna de un emulador de CPU 6502 en C++. Este proyecto recrea el comportamiento del legendario procesador MOS Technology 6502, utilizado en sistemas clásicos como Apple II, Commodore 64, y NES.

## 🚀 Características

- **Emulación precisa** del conjunto de instrucciones 6502
- **Sistema de memoria** con soporte para ROM y RAM
- **Soporte para múltiples modos de direccionamiento**
- **Registro de ejecución** con tracing detallado
- **Tests integrados** para validación de instrucciones (27+ tests unitarios)
- **Arquitectura modular** y extensible con separación clara de componentes
- **Logger configurable** con niveles de log (ERROR, WARN, INFO, DEBUG)
- **Librería reutilizable** (libcpu6502) para integración en otros proyectos
- **Build system dual**: CMake y Make para máxima compatibilidad

## 📋 Requisitos

- **CMake** 3.10 o superior
- **g++ (GCC)** 7.5.0 o superior, o cualquier compilador compatible con C++17
- **Make** para la construcción del proyecto

## 🔧 Instalación y Configuración

### 1. Clonar el repositorio

```bash
git clone https://github.com/Kilynho/cpu6502.git
cd cpu6502
```

### 2. Inicializar submódulos (para GoogleTest)

```bash
git submodule update --init --recursive
```

### 3. Compilar el proyecto

El proyecto soporta dos sistemas de compilación para máxima flexibilidad:

#### Opción A: Usando Make (Recomendado)
```bash
make
```

Esto generará:
- `build/src/libcpu6502_lib.a` - Librería estática reutilizable
- `build/src/cpu_demo` - Ejecutable de demostración
- `build/runTests` - Suite de tests

#### Opción B: Usando CMake directamente
```bash
mkdir -p build
cd build
cmake ..
make
```

### 4. Ejecutar el emulador

```bash
# Desde la raíz del proyecto con Make
make demo

# O directamente
./build/src/cpu_demo
```

## 🎯 Uso

El emulador ejecuta programas escritos en código máquina 6502. Los programas se cargan en la memoria ROM (direcciones 0x8000-0xFFFF) y el emulador ejecuta las instrucciones secuencialmente.

### Ejemplo de uso básico:

```cpp
#include "cpu.hpp"
#include "mem.hpp"
#include "util/logger.hpp"

int main() {
    // Configurar nivel de log (opcional)
    util::LogSetLevel(util::LogLevel::INFO);
    
    CPU cpu;
    Mem mem;
    
    cpu.Reset(mem);
    
    // Cargar programa en memoria
    mem[0x8000] = CPU::INS_LDA_IM.opcode;  // LDA #$42
    mem[0x8001] = 0x42;
    
    // Ejecutar
    cpu.Execute(2, mem);
    
    return 0;
}
```

### Uso del Logger

El proyecto incluye un sistema de logging configurable:

```cpp
#include "util/logger.hpp"

// Establecer nivel de log
util::LogSetLevel(util::LogLevel::DEBUG);  // NONE, ERROR, WARN, INFO, DEBUG

// Usar funciones de log
util::LogError("Error message");
util::LogWarn("Warning message");
util::LogInfo("Info message");
util::LogDebug("Debug message");

// O usar macros
LOG_ERROR("Error: " << variable);
LOG_INFO("CPU initialized successfully");
```

## 🧪 Testing

El proyecto incluye una suite completa de tests unitarios usando Google Test para validar el comportamiento de las instrucciones.

### Ejecutar los tests

#### Opción A: Usando Make con CTest
```bash
make test
```

#### Opción B: Ejecutar tests directamente
```bash
make runTests
# O manualmente
./build/runTests
```

#### Opción C: Desde build directory con CMake
```bash
cd build
ctest --output-on-failure
# O
./runTests
```

### Cobertura de Tests

Los tests incluyen:
- **Instrucciones LDA**: Todos los modos de direccionamiento (Immediate, Zero Page, Zero Page X, Absolute, Absolute X, Absolute Y)
- **Instrucción LDX**: Modo inmediato
- **Instrucción STA**: Almacenamiento en memoria
- **Instrucciones JSR/RTS**: Llamadas a subrutinas y manejo de pila
- **Casos límite**: Valores cero, 0xFF, límites de página
- **Comportamiento de flags**: Zero flag (Z) y Negative flag (N)
- **Llamadas anidadas**: Subrutinas anidadas y manejo correcto de la pila

### CI/CD Automatizado

El proyecto usa GitHub Actions para ejecutar automáticamente los tests en cada push y pull request. Puedes ver el estado de la build en el badge de CI en la parte superior del README.

## 📖 Arquitectura del Proyecto

El proyecto está organizado de forma modular para facilitar el mantenimiento y la reutilización:

```
cpu6502/
├── include/              # Headers públicos de la API
│   ├── cpu.hpp          # Interfaz pública de la CPU
│   ├── mem.hpp          # Interfaz pública de la memoria
│   └── util/
│       └── logger.hpp   # Sistema de logging
├── src/                  # Implementaciones
│   ├── cpu/
│   │   └── cpu.cpp      # Implementación de la CPU 6502
│   ├── mem/
│   │   └── mem.cpp      # Implementación del sistema de memoria
│   ├── util/
│   │   └── logger.cpp   # Implementación del logger
│   ├── main/
│   │   └── cpu_demo.cpp # Programa de demostración
│   └── CMakeLists.txt   # Configuración de build de src
├── tests/                # Suite de tests
│   ├── test_main.cpp    # Tests unitarios (27+ tests)
│   └── CMakeLists.txt   # Configuración de tests
├── examples/             # Binarios y ejemplos de referencia
│   └── main_6502_legacy # Binario de referencia
├── lib/                  # Librerías externas
│   └── googletest/      # Framework de testing (submódulo)
├── CMakeLists.txt        # Configuración principal de CMake
└── Makefile              # Wrapper de Make para CMake
```

### Componentes Principales

- **libcpu6502_lib.a**: Librería estática que contiene CPU, Memoria y Logger
- **cpu_demo**: Ejecutable de demostración del emulador
- **runTests**: Suite completa de tests unitarios

## 📚 Documentación

El proyecto incluye documentación técnica completa:

### Guías de Desarrollo
- **[docs/instructions.md](docs/instructions.md)** - Guía completa para implementar y testear instrucciones
- **[docs/architecture.md](docs/architecture.md)** - Descripción de la arquitectura del emulador

### Generación de Documentación con Doxygen

Para generar la documentación del código:

```bash
cd docs
doxygen Doxyfile
```

La documentación se generará en `docs/doxygen/html/`. Abre `index.html` en tu navegador para verla.

### Comandos Útiles

```bash
# Compilar el proyecto
make

# Ejecutar tests
make test

# Ejecutar demo
make demo

# Generar documentación
cd docs && doxygen Doxyfile
```

## 🤝 Contribuir

¡Las contribuciones son bienvenidas! Por favor, lee [CONTRIBUTING.md](CONTRIBUTING.md) para conocer los detalles sobre nuestro código de conducta y el proceso para enviarnos pull requests.

## 📝 Changelog

Para ver el historial de cambios del proyecto, consulta [CHANGELOG.md](CHANGELOG.md).

## 🔒 Seguridad

Si descubres una vulnerabilidad de seguridad, por favor consulta [SECURITY.md](SECURITY.md) para saber cómo reportarla de manera responsable.

## 📄 Licencia

Este proyecto está licenciado bajo la Licencia MIT - ver el archivo [LICENSE](LICENSE) para más detalles.

## �� Instrucciones Soportadas

El emulador implementa **el conjunto completo de 151 instrucciones oficiales del 6502**:

### Load/Store Operations
- **LDA** (Load Accumulator): Immediate, Zero Page, Zero Page,X, Absolute, Absolute,X, Absolute,Y, Indirect,X, Indirect,Y
- **LDX** (Load X Register): Immediate, Zero Page, Zero Page,Y, Absolute, Absolute,Y
- **LDY** (Load Y Register): Immediate, Zero Page, Zero Page,X, Absolute, Absolute,X
- **STA** (Store Accumulator): Zero Page, Zero Page,X, Absolute, Absolute,X, Absolute,Y, Indirect,X, Indirect,Y
- **STX** (Store X Register): Zero Page, Zero Page,Y, Absolute
- **STY** (Store Y Register): Zero Page, Zero Page,X, Absolute

### Register Transfers
- **TAX, TAY, TXA, TYA** (Transfer between A, X, Y)
- **TSX, TXS** (Transfer between stack pointer and X)

### Stack Operations
- **PHA, PLA** (Push/Pull Accumulator)
- **PHP, PLP** (Push/Pull Processor Status)

### Logical Operations
- **AND, EOR, ORA** (Bitwise operations): All addressing modes
- **BIT** (Bit test): Zero Page, Absolute

### Arithmetic Operations
- **ADC, SBC** (Add/Subtract with Carry): All addressing modes

### Increments & Decrements
- **INC, DEC** (Memory): Zero Page, Zero Page,X, Absolute, Absolute,X
- **INX, INY, DEX, DEY** (Registers)

### Shifts & Rotates
- **ASL, LSR** (Arithmetic/Logical Shift): Accumulator and Memory modes
- **ROL, ROR** (Rotate Left/Right): Accumulator and Memory modes

### Jumps & Calls
- **JMP** (Jump): Absolute, Indirect
- **JSR** (Jump to Subroutine)
- **RTS** (Return from Subroutine)

### Branches (Conditional)
- **BCC, BCS** (Branch on Carry Clear/Set)
- **BEQ, BNE** (Branch on Equal/Not Equal)
- **BMI, BPL** (Branch on Minus/Plus)
- **BVC, BVS** (Branch on Overflow Clear/Set)

### Comparisons
- **CMP** (Compare Accumulator): All addressing modes
- **CPX, CPY** (Compare X/Y): Immediate, Zero Page, Absolute

### Status Flag Changes
- **CLC, SEC** (Clear/Set Carry)
- **CLD, SED** (Clear/Set Decimal Mode)
- **CLI, SEI** (Clear/Set Interrupt Disable)
- **CLV** (Clear Overflow)

### System Functions
- **BRK** (Break/Interrupt)
- **RTI** (Return from Interrupt)
- **NOP** (No Operation)

Para más detalles sobre cada instrucción, consulta `docs/instructions.md`.

## 👥 Autores

* **Kilynho** - *Trabajo inicial* - [Kilynho](https://github.com/Kilynho)

## 🙏 Agradecimientos

- Inspirado en el legendario procesador MOS Technology 6502
- Comunidad de desarrolladores de emuladores retro
- Documentación técnica del 6502

## 📚 Recursos

- [6502 Instruction Reference](http://www.6502.org/tutorials/6502opcodes.html)
- [6502.org - The 6502 Microprocessor Resource](http://www.6502.org/)
- [Visual 6502](http://www.visual6502.org/)

---

**Nota:** Este es un proyecto educativo y de aprendizaje. No pretende ser una emulación perfecta del hardware original.
