# Changelog

Todos los cambios notables en este proyecto serán documentados en este archivo.

El formato está basado en [Keep a Changelog](https://keepachangelog.com/es-ES/1.0.0/),
y este proyecto adhiere a [Semantic Versioning](https://semver.org/lang/es/).

## [Unreleased]

### Añadido
- Suite completa de tests unitarios con Google Test
  - Tests para todas las variantes de LDA (IM, ZP, ZPX, ABS, ABSX, ABSY)
  - Tests para LDX en modo inmediato
  - Tests para STA con diferentes valores
  - Tests para JSR/RTS con validación de pila
  - Tests de casos límite (cero, 0xFF, límites de página)
  - Tests de comportamiento de flags (Z, N)
  - Tests de llamadas a subrutinas anidadas
  - Total de 30+ tests unitarios
- CI/CD con GitHub Actions (.github/workflows/ci.yml)
  - Compilación automática en cada push y PR
  - Ejecución automática de tests
  - Soporte para CMake y Make
- Badge de CI en README.md mostrando estado de build
- Documentación completa del proyecto (README.md, CONTRIBUTING.md, CHANGELOG.md)
- Archivo LICENSE con licencia MIT
- Política de seguridad (SECURITY.md)
- Templates de GitHub para issues (bugs y features)
- Template de GitHub para pull requests
- Badges en README para licencia, versión de C++ y estado de build

### Cambiado
- Mejorada la documentación del README con secciones detalladas
- Añadidas instrucciones completas de instalación y uso
- Expandida la sección de testing en README.md con descripción detallada de cobertura
- Actualizado CONTRIBUTING.md con información sobre tests y CI

## [0.1.0] - 2024-12-08

### Añadido
- Implementación inicial de la CPU 6502
- Sistema de memoria con soporte para ROM y RAM
- Instrucciones básicas del 6502:
  - LDA (Load Accumulator): Immediate, Zero Page, Zero Page X, Absolute, Absolute X, Absolute Y
  - LDX (Load X Register): Immediate
  - STA (Store Accumulator): Immediate
  - JSR (Jump to Subroutine)
  - RTS (Return from Subroutine)
- Sistema de flags del procesador (Zero, Negative, Carry, etc.)
- Cálculo automático de ciclos de reloj
- Tracing de ejecución con información detallada
- Tests unitarios básicos
- Configuración de build con Make y CMake
- Soporte para múltiples modos de direccionamiento
- Funcionalidad de reset del CPU
- Sistema de stack pointer

### Características Técnicas
- Emulación de 64KB de memoria direccionable
- ROM en direcciones 0x8000-0xFFFF
- RAM en direcciones 0x0000-0x7FFF
- Stack en 0x0100-0x01FF
- Precisión de ciclos de reloj
- Manejo correcto de límites de página

## Tipos de Cambios

- `Añadido` para nuevas características
- `Cambiado` para cambios en funcionalidad existente
- `Obsoleto` para características que serán removidas
- `Eliminado` para características eliminadas
- `Corregido` para corrección de bugs
- `Seguridad` para vulnerabilidades de seguridad

## Roadmap Futuro

### [0.2.0] - Planificado
- Soporte completo para instrucciones aritméticas (ADC, SBC)
- Instrucciones lógicas (AND, ORA, EOR)
- Instrucciones de incremento/decremento (INC, DEC, INX, INY, DEX, DEY)
- Instrucciones de comparación (CMP, CPX, CPY)
- Sistema completo de flags

### [0.3.0] - Planificado
- Instrucciones de branch condicional (BEQ, BNE, BCS, BCC, BMI, BPL, BVS, BVC)
- Instrucciones de bit (BIT)
- Instrucciones de shift y rotate (ASL, LSR, ROL, ROR)

### [0.4.0] - Planificado
- Instrucciones de stack (PHA, PLA, PHP, PLP)
- Instrucciones de transferencia (TAX, TAY, TXA, TYA, TSX, TXS)
- Manejo de interrupciones (IRQ, NMI)
- Instrucción BRK y manejo de break

### [1.0.0] - Objetivo
- Conjunto completo de instrucciones del 6502
- Emulación precisa del timing
- Documentación completa de todas las instrucciones
- Suite de tests exhaustiva
- Ejemplos de programas para ejecutar
- Soporte para cargar binarios desde archivos
- Debugger interactivo básico

---

## Enlaces

- [Repositorio del Proyecto](https://github.com/Kilynho/cpu6502)
- [Reportar un Bug](https://github.com/Kilynho/cpu6502/issues/new?template=bug_report.md)
- [Solicitar Feature](https://github.com/Kilynho/cpu6502/issues/new?template=feature_request.md)
- [Guía de Contribución](CONTRIBUTING.md)

---

**Nota:** Las fechas y versiones son tentativas y pueden cambiar según el progreso del desarrollo.
