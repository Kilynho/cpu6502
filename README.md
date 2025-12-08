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
- **Tests integrados** para validación de instrucciones
- **Arquitectura modular** y extensible

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

### 2. Compilar el proyecto

#### Usando Make:
```bash
make
```

#### Usando CMake:
```bash
mkdir -p build
cd build
cmake ..
make
```

### 3. Ejecutar el emulador

```bash
./main_6502
```

## 🎯 Uso

El emulador ejecuta programas escritos en código máquina 6502. Los programas se cargan en la memoria ROM (direcciones 0x8000-0xFFFF) y el emulador ejecuta las instrucciones secuencialmente.

### Ejemplo de uso básico:

```cpp
#include "cpu.hpp"
#include "mem.hpp"

int main() {
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

## 🧪 Testing

El proyecto incluye una suite completa de tests unitarios usando Google Test para validar el comportamiento de las instrucciones.

### Ejecutar los tests

#### Usando CMake:
```bash
cd build
./runTests
```

#### Usando Make:
```bash
make
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

```
cpu6502/
├── cpu.cpp           # Implementación de la CPU 6502
├── cpu.hpp           # Definiciones de la CPU
├── mem.cpp           # Sistema de memoria
├── mem.hpp           # Definiciones de memoria
├── main_6502.cpp     # Punto de entrada principal
├── test.cpp          # Suite de tests
├── CMakeLists.txt    # Configuración de CMake
└── Makefile          # Configuración de Make
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

El emulador actualmente soporta las siguientes instrucciones:

- **LDA** (Load Accumulator): Immediate, Zero Page, Zero Page X, Absolute, Absolute X, Absolute Y
- **LDX** (Load X Register): Immediate
- **STA** (Store Accumulator): Immediate
- **JSR** (Jump to Subroutine)
- **RTS** (Return from Subroutine)

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
