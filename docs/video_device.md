# Video Device - Documentación

## Introducción

El módulo de video del emulador CPU 6502 proporciona capacidades de pantalla de texto mediante la clase `TextScreen`, que simula una pantalla clásica de 40 columnas x 24 líneas similar a las computadoras de 8 bits de los años 80 (Apple II, Commodore 64, etc.).

## Arquitectura

### Jerarquía de Clases

```
IODevice (interfaz base)
    └── VideoDevice (interfaz para dispositivos de video)
            └── TextScreen (implementación de pantalla de texto)
```

### Interfaz VideoDevice

La interfaz `VideoDevice` define los métodos básicos que todo dispositivo de video debe implementar:

```cpp
class VideoDevice : public IODevice {
public:
    virtual void refresh() = 0;                    // Refresca la pantalla
    virtual void clear() = 0;                      // Limpia la pantalla
    virtual std::string getBuffer() const = 0;     // Obtiene el buffer para debugging
    virtual uint16_t getWidth() const = 0;         // Ancho en caracteres/píxeles
    virtual uint16_t getHeight() const = 0;        // Alto en caracteres/píxeles
};
```

## TextScreen - Pantalla de Texto

### Especificaciones

- **Dimensiones:** 40 columnas × 24 líneas (960 caracteres)
- **Conjunto de caracteres:** ASCII imprimible (0x20-0x7E)
- **Características especiales:**
  - Auto-scroll cuando se llena la pantalla
  - Control de posición del cursor
  - Soporte para caracteres de control (\\n, \\r, \\t, \\b)

### Mapa de Memoria

La TextScreen se mapea en las siguientes direcciones de memoria:

| Dirección | Descripción | Tipo | Rango |
|-----------|-------------|------|-------|
| `$FC00-$FFFB` | Buffer de video (960 bytes) | R/W | 0x00-0xFF |
| `$FFFC` | Columna del cursor | R/W | 0-39 |
| `$FFFD` | Fila del cursor | R/W | 0-23 |
| `$FFFE` | Registro de control | R/W | Ver bits abajo |
| `$FFFF` | Puerto de salida de caracteres | W | 0x00-0xFF |

#### Organización del Buffer de Video

El buffer de video está organizado en filas:
- Fila 0: `$FC00-$FC27` (bytes 0-39)
- Fila 1: `$FC28-$FC4F` (bytes 40-79)
- ...
- Fila 23: `$FFD8-$FFFB` (bytes 920-959)

**Fórmula de dirección:** `dirección = $FC00 + (fila × 40) + columna`

#### Registro de Control ($FFFE)

| Bit | Nombre | Descripción |
|-----|--------|-------------|
| 0 | AUTO_SCROLL | 1 = auto-scroll habilitado, 0 = deshabilitado |
| 1 | CLEAR_SCREEN | Escribir 1 limpia la pantalla (se resetea automáticamente) |
| 2-6 | Reservado | No utilizado |
| 7 | CURSOR_VISIBLE | 1 = cursor visible, 0 = invisible (para uso futuro con GUI) |

## Uso desde C++

### Inicialización

```cpp
#include "cpu.hpp"
#include "mem.hpp"
#include "devices/text_screen.hpp"
#include <memory>

int main() {
    Mem mem;
    CPU cpu;
    auto textScreen = std::make_shared<TextScreen>();
    
    // Reiniciar CPU y registrar el dispositivo
    cpu.Reset(mem);
    cpu.registerIODevice(textScreen);
    
    // Ahora la CPU puede acceder a la pantalla mediante memoria mapeada
}
```

### API C++ Directa

La clase `TextScreen` proporciona métodos C++ para control directo:

```cpp
// Escribir caracteres
textScreen->writeCharAtCursor('H');
textScreen->writeCharAtCursor('i');

// Control del cursor
textScreen->setCursorPosition(10, 5);  // Columna 10, fila 5
uint8_t col, row;
textScreen->getCursorPosition(col, row);

// Limpieza
textScreen->clear();

// Auto-scroll
textScreen->setAutoScroll(true);
bool enabled = textScreen->getAutoScroll();

// Obtener contenido (para debugging/testing)
std::string buffer = textScreen->getBuffer();
std::cout << buffer << std::endl;
```

## Uso desde Código 6502

### Ejemplo 1: Escribir "Hello, World!"

```asm
        ; Escribir mediante el puerto de caracteres ($FFFF)
        LDA #'H'
        STA $FFFF
        LDA #'e'
        STA $FFFF
        LDA #'l'
        STA $FFFF
        LDA #'l'
        STA $FFFF
        LDA #'o'
        STA $FFFF
```

### Ejemplo 2: Posicionar Cursor y Escribir

```asm
        ; Posicionar cursor en columna 10, fila 5
        LDA #10
        STA $FFFC      ; Columna
        LDA #5
        STA $FFFD      ; Fila
        
        ; Escribir 'X' en esa posición
        LDA #'X'
        STA $FFFF
```

### Ejemplo 3: Escritura Directa en Buffer de Video

```asm
        ; Escribir 'A' en fila 2, columna 15
        ; Dirección = $FC00 + (2 × 40) + 15 = $FC00 + 80 + 15 = $FC4F
        LDA #'A'
        STA $FC4F
```

### Ejemplo 4: Limpiar Pantalla

```asm
        ; Limpiar pantalla mediante registro de control
        LDA #$02       ; Bit 1 = CLEAR_SCREEN
        STA $FFFE
```

### Ejemplo 5: Escribir Línea de Texto con Salto

```asm
        ; Escribir "Line 1" seguido de nueva línea
        LDA #'L'
        STA $FFFF
        LDA #'i'
        STA $FFFF
        LDA #'n'
        STA $FFFF
        LDA #'e'
        STA $FFFF
        LDA #' '
        STA $FFFF
        LDA #'1'
        STA $FFFF
        LDA #$0A       ; '\n' = 0x0A
        STA $FFFF
```

## Caracteres de Control

La TextScreen soporta los siguientes caracteres de control:

| Carácter | Código | Descripción |
|----------|--------|-------------|
| `\n` | 0x0A | Nueva línea: mueve el cursor al inicio de la siguiente línea |
| `\r` | 0x0D | Retorno de carro: mueve el cursor al inicio de la línea actual |
| `\t` | 0x09 | Tabulación: avanza a la siguiente posición múltiplo de 8 |
| `\b` | 0x08 | Backspace: retrocede el cursor una posición (sin borrar) |

## Auto-Scroll

Cuando el auto-scroll está habilitado (bit 0 de `$FFFE` = 1):
- Al escribir más allá de la última línea, todo el contenido se desplaza una línea hacia arriba
- La primera línea se pierde
- La última línea queda vacía y lista para nueva escritura
- El cursor se posiciona al inicio de la última línea

Cuando está deshabilitado:
- Al escribir más allá de la última línea, el cursor vuelve al inicio (0,0)

## Integración con Sistemas Futuros

### Preparación para GUI (SDL/OpenGL)

La arquitectura de `VideoDevice` está diseñada para permitir futuras extensiones:

1. **Método refresh():** Actualmente no hace nada, pero en una implementación con GUI real actualizaría la ventana SDL/OpenGL.

2. **Bit CURSOR_VISIBLE:** Permite controlar la visibilidad del cursor en implementaciones gráficas.

3. **Separación de lógica y presentación:** El buffer de video está completamente separado de su visualización, facilitando añadir backends gráficos.

### Ejemplo de Integración Futura

```cpp
class GraphicalTextScreen : public TextScreen {
protected:
    SDL_Window* window;
    SDL_Renderer* renderer;
    
public:
    void refresh() override {
        // Actualizar ventana SDL con el contenido del buffer
        renderTextToSDL();
        SDL_RenderPresent(renderer);
    }
};
```

## Rendimiento

### Escritura mediante Puerto de Caracteres

- **Ventaja:** Simplifica el código 6502 (solo necesita escribir a `$FFFF`)
- **Desventaja:** Más lenta (procesa caracteres de control, mueve cursor, etc.)
- **Uso recomendado:** Salida de texto secuencial, mensajes de consola

### Escritura Directa en Buffer

- **Ventaja:** Máxima velocidad (acceso directo a memoria)
- **Desventaja:** El programador debe calcular direcciones y gestionar el cursor manualmente
- **Uso recomendado:** Gráficos de texto, interfaces de usuario, animaciones

## Ejemplos Completos

Ver los siguientes archivos para ejemplos completos:

- **examples/text_screen_demo.cpp:** Demostración completa de todas las características
- **tests/test_text_screen.cpp:** Casos de prueba que muestran todos los modos de uso

## Solución de Problemas

### El texto no aparece

- Verificar que el dispositivo está registrado: `cpu.registerIODevice(textScreen)`
- Verificar que se está escribiendo en las direcciones correctas
- Verificar que se están usando caracteres ASCII imprimibles (0x20-0x7E)

### El scroll no funciona

- Verificar que el auto-scroll está habilitado: escribir `0x01` en `$FFFE`
- Asegurarse de que se está escribiendo mediante el puerto de caracteres (`$FFFF`)

### Los caracteres aparecen en posiciones incorrectas

- Verificar el cálculo de direcciones: `dirección = $FC00 + (fila × 40) + columna`
- Recordar que las filas y columnas son base 0 (0-23 y 0-39)

## API Completa de TextScreen

### Métodos Públicos

```cpp
// Constructor
TextScreen();

// IODevice interface
bool handlesRead(uint16_t address) const override;
bool handlesWrite(uint16_t address) const override;
uint8_t read(uint16_t address) override;
void write(uint16_t address, uint8_t value) override;

// VideoDevice interface
void refresh() override;
void clear() override;
std::string getBuffer() const override;
uint16_t getWidth() const override;
uint16_t getHeight() const override;

// Control methods
void setCursorPosition(uint8_t col, uint8_t row);
void getCursorPosition(uint8_t& col, uint8_t& row) const;
void writeCharAtCursor(char c);
void setAutoScroll(bool enabled);
bool getAutoScroll() const;
```

## Conclusión

El sistema de video TextScreen proporciona una forma sencilla y potente de añadir capacidades de salida visual al emulador CPU 6502. Su diseño modular permite futuras extensiones (gráficos, color, múltiples modos de video) mientras mantiene la simplicidad y compatibilidad con sistemas clásicos de 8 bits.
