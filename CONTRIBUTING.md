# Guía de Contribución

¡Gracias por tu interés en contribuir al proyecto CPU 6502 Emulator! Este documento proporciona pautas y mejores prácticas para contribuir al proyecto.

## 📋 Tabla de Contenidos

- [Código de Conducta](#código-de-conducta)
- [¿Cómo puedo contribuir?](#cómo-puedo-contribuir)
- [Proceso de Pull Request](#proceso-de-pull-request)
- [Guía de Estilo](#guía-de-estilo)
- [Estructura del Proyecto](#estructura-del-proyecto)
- [Testing](#testing)

## 📜 Código de Conducta

Este proyecto se adhiere a un código de conducta que esperamos que todos los participantes sigan. Al participar, te comprometes a mantener un ambiente respetuoso y acogedor para todos.

### Nuestros Estándares

**Ejemplos de comportamiento que contribuye a crear un ambiente positivo:**
- Usar un lenguaje acogedor e inclusivo
- Ser respetuoso con diferentes puntos de vista y experiencias
- Aceptar críticas constructivas con gracia
- Enfocarse en lo que es mejor para la comunidad
- Mostrar empatía hacia otros miembros de la comunidad

**Ejemplos de comportamiento inaceptable:**
- Uso de lenguaje o imágenes sexualizadas
- Comentarios insultantes o despectivos (trolling)
- Acoso público o privado
- Publicar información privada de otros sin permiso explícito
- Otra conducta que razonablemente podría considerarse inapropiada

## 🤝 ¿Cómo puedo contribuir?

### Reportar Bugs

Los bugs se rastrean como [GitHub Issues](https://github.com/Kilynho/cpu6502/issues). Antes de crear un nuevo issue:

1. **Verifica** si el bug ya ha sido reportado
2. **Usa la plantilla** de bug report
3. **Proporciona detalles** específicos y reproducibles

#### Información necesaria para reportar un bug:
- Descripción clara del problema
- Pasos para reproducir el comportamiento
- Comportamiento esperado vs. comportamiento actual
- Capturas de pantalla si es aplicable
- Versión del compilador y sistema operativo
- Logs relevantes

### Sugerir Mejoras

Las mejoras también se rastrean como GitHub Issues. Para sugerir una mejora:

1. **Usa la plantilla** de feature request
2. **Explica claramente** el problema que resuelve
3. **Describe la solución** que te gustaría ver
4. **Considera alternativas** que hayas evaluado

### Tu Primera Contribución de Código

¿No estás seguro por dónde empezar? Puedes buscar issues etiquetados como:

- `good first issue` - Issues que deberían requerir solo unas pocas líneas de código
- `help wanted` - Issues que pueden ser más complejos pero necesitan ayuda

### Implementar Nuevas Instrucciones

Si deseas implementar nuevas instrucciones del 6502:

1. Consulta la [documentación oficial del 6502](http://www.6502.org/tutorials/6502opcodes.html)
2. Sigue el patrón establecido en `cpu.cpp` para instrucciones existentes
3. Define la instrucción en `cpu.hpp`
4. Implementa la lógica en `cpu.cpp`
5. Añade tests en `test.cpp`
6. Actualiza la documentación en `README.md`

## 🔄 Proceso de Pull Request

1. **Fork** el repositorio y crea tu rama desde `main`
   ```bash
   git checkout -b feature/nombre-descriptivo
   ```

2. **Realiza tus cambios** siguiendo la guía de estilo

3. **Añade tests** para tu código nuevo

4. **Asegúrate** de que todos los tests pasen
   ```bash
   make test
   ```

5. **Compila** el proyecto sin errores ni warnings
   ```bash
   make clean
   make
   ```

6. **Documenta** tus cambios en el código y actualiza README si es necesario

7. **Commit** tus cambios con mensajes descriptivos
   ```bash
   git commit -m "feat: descripción breve del cambio"
   ```

8. **Push** a tu fork
   ```bash
   git push origin feature/nombre-descriptivo
   ```

9. **Abre un Pull Request** usando la plantilla proporcionada

### Formato de Mensajes de Commit

Usamos el formato [Conventional Commits](https://www.conventionalcommits.org/):

```
<tipo>(<alcance>): <descripción>

[cuerpo opcional]

[footer opcional]
```

**Tipos:**
- `feat`: Nueva característica
- `fix`: Corrección de bug
- `docs`: Cambios en documentación
- `style`: Cambios de formato (no afectan el código)
- `refactor`: Refactorización de código
- `test`: Añadir o modificar tests
- `chore`: Cambios en el proceso de build o herramientas

**Ejemplos:**
```
feat(cpu): añadir soporte para instrucción ADC
fix(mem): corregir desbordamiento en lectura de memoria
docs(readme): actualizar instrucciones de instalación
test(cpu): añadir tests para instrucciones de branch
```

## 🎨 Guía de Estilo

### Estilo de Código C++

- **Indentación**: 4 espacios (no tabs)
- **Llaves**: Estilo K&R (llave de apertura en la misma línea)
- **Nombres de variables**: camelCase para variables locales, PascalCase para tipos
- **Nombres de funciones**: PascalCase
- **Constantes**: UPPER_SNAKE_CASE
- **Longitud de línea**: Máximo 100 caracteres

### Ejemplo:

```cpp
// Correcto
void CPU::ExecuteInstruction(Mem& mem) {
    const Byte opcode = FetchByte(mem);
    
    if (opcode == INS_LDA_IM.opcode) {
        const Byte value = FetchByte(mem);
        A = value;
        SetZeroAndNegativeFlags(A);
    }
}

// Incorrecto
void CPU::ExecuteInstruction(Mem& mem){
  byte opcode=FetchByte(mem);
  if(opcode==INS_LDA_IM.opcode){
    byte value=FetchByte(mem);
    A=value;
    SetZeroAndNegativeFlags(A);
  }
}
```

### Comentarios

- Escribe comentarios en español o inglés consistentemente
- Comenta el "por qué", no el "qué"
- Usa comentarios de documentación para funciones públicas

```cpp
// Correcto
// Ajusta el program counter cuando se cruza un límite de página
// para emular el ciclo extra del hardware original
if (PageCrossed(oldPC, PC)) {
    cycles++;
}

// Incorrecto
// Incrementa cycles si hay page cross
if (PageCrossed(oldPC, PC)) {
    cycles++;  // suma 1 a cycles
}
```

## 🏗️ Estructura del Proyecto

```
cpu6502/
├── .github/              # Templates y workflows de GitHub
│   ├── ISSUE_TEMPLATE/   # Plantillas de issues
│   └── pull_request_template.md
├── build/                # Archivos de build (ignorado en git)
├── lib/                  # Librerías externas
├── cpu.hpp              # Declaraciones de la CPU
├── cpu.cpp              # Implementación de la CPU
├── mem.hpp              # Declaraciones de memoria
├── mem.cpp              # Implementación de memoria
├── main_6502.cpp        # Programa principal
├── test.cpp             # Suite de tests
├── CMakeLists.txt       # Configuración CMake
├── Makefile             # Configuración Make
├── README.md            # Documentación principal
├── CONTRIBUTING.md      # Esta guía
├── CHANGELOG.md         # Historial de cambios
├── LICENSE              # Licencia MIT
└── SECURITY.md          # Política de seguridad
```

## 🧪 Testing

### Ejecutar Tests

```bash
make test
# o
./test
```

### Escribir Tests

Los tests deben:
- Ser independientes entre sí
- Tener nombres descriptivos
- Probar un solo comportamiento
- Incluir casos edge

Ejemplo:

```cpp
void TestLDAImmediate() {
    CPU cpu;
    Mem mem;
    cpu.Reset(mem);
    
    // Arrange
    mem[0x8000] = CPU::INS_LDA_IM.opcode;
    mem[0x8001] = 0x42;
    
    // Act
    cpu.Execute(2, mem);
    
    // Assert
    assert(cpu.A == 0x42);
    assert(!cpu.Z);  // Zero flag should be clear
    assert(!cpu.N);  // Negative flag should be clear
}
```

## 📝 Documentación

- Actualiza el README.md si añades nuevas características
- Documenta funciones públicas con comentarios
- Añade ejemplos de uso cuando sea apropiado
- Mantén el CHANGELOG.md actualizado

## ❓ Preguntas

Si tienes preguntas, puedes:

1. Revisar la [documentación existente](README.md)
2. Buscar en [issues existentes](https://github.com/Kilynho/cpu6502/issues)
3. Abrir un nuevo issue con la etiqueta `question`

## 🎉 Reconocimientos

¡Todas las contribuciones son valoradas! Los contribuidores serán reconocidos en:
- El archivo README.md
- Las release notes
- El CHANGELOG.md

---

¡Gracias por contribuir al CPU 6502 Emulator! 🚀
