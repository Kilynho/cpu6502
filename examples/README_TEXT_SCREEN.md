# TextScreen - Ejemplo de Uso

Este archivo contiene un ejemplo completo del dispositivo de video `TextScreen`.

## Archivo

- **text_screen_demo.cpp**: Demostración completa de todas las funcionalidades de TextScreen

## Ejecutar el Ejemplo

### Usando Make

```bash
# Desde el directorio raíz del proyecto
make text_screen_demo
```

### Directamente

```bash
# Compilar primero
make all

# Ejecutar
./build/text_screen_demo
```

## Funcionalidades Demostradas

El ejemplo `text_screen_demo.cpp` demuestra las siguientes características:

### 1. Escritura Básica de Texto
- Escritura de "Hello, World!" usando el puerto de caracteres (`$FFFF`)
- Muestra cómo cada carácter se escribe secuencialmente desde código 6502

### 2. Posicionamiento del Cursor
- Escritura en diferentes posiciones de la pantalla
- Uso de los registros de cursor (`$FFFC` y `$FFFD`)
- Demostración de escritura no secuencial

### 3. Saltos de Línea y Formato
- Uso del carácter de nueva línea (`\n`)
- Escritura de texto en múltiples líneas
- Formato básico de texto

### 4. Auto-Scroll
- Llenado de la pantalla con más de 24 líneas
- Demostración del desplazamiento automático
- Las líneas antiguas desaparecen conforme se añaden nuevas

### 5. Escritura Directa en Memoria de Video
- Acceso directo al buffer de video (`$FC00-$FFFB`)
- Escritura en posiciones específicas sin usar el cursor
- Demostración de cálculo de direcciones

### 6. Creación de Marcos Decorativos
- Uso de caracteres ASCII para crear bordes
- Combinación de posicionamiento preciso del cursor
- Ejemplo de interfaz de usuario simple

## Salida del Programa

El programa muestra 6 demostraciones diferentes, cada una en su propia pantalla de 40x24 caracteres. Entre cada demo hay una pausa para que puedas ver los resultados.

## Mapa de Memoria Utilizado

| Dirección | Uso |
|-----------|-----|
| `$FC00-$FFFB` | Buffer de video (40x24 = 960 bytes) |
| `$FFFC` | Columna del cursor (0-39) |
| `$FFFD` | Fila del cursor (0-23) |
| `$FFFE` | Registro de control (auto-scroll, clear, etc.) |
| `$FFFF` | Puerto de salida de caracteres |

## Código 6502 de Ejemplo

El demo incluye ejemplos de código 6502 assembly traducido a código máquina, mostrando:

```asm
; Escribir 'H' en la pantalla
LDA #'H'      ; $A9 'H'
STA $FFFF     ; $8D $FF $FF

; Posicionar cursor
LDA #10       ; $A9 $0A
STA $FFFC     ; $8D $FC $FF (columna)
LDA #5        ; $A9 $05
STA $FFFD     ; $8D $FD $FF (fila)
```

## Próximos Pasos

Después de ejecutar este ejemplo:

1. Lee la documentación completa en `docs/video_device.md`
2. Revisa los tests en `tests/test_text_screen.cpp` para más ejemplos de uso
3. Experimenta creando tus propios programas con salida de texto
4. Considera cómo se podría extender con una GUI real (SDL/OpenGL)

## Integración con Otros Dispositivos

TextScreen puede usarse en conjunto con otros dispositivos:

```cpp
// Combinar FileDevice y TextScreen
auto fileDevice = std::make_shared<FileDevice>(&mem);
auto textScreen = std::make_shared<TextScreen>();

cpu.registerIODevice(fileDevice);
cpu.registerIODevice(textScreen);

// Ahora puedes cargar un programa que use la pantalla
fileDevice->loadBinary("text_app.bin", 0x8000);
cpu.Execute(1000, mem);
std::cout << textScreen->getBuffer();
```

## Notas

- La pantalla es de 40 columnas x 24 líneas (estándar de muchas computadoras de 8 bits)
- Solo se soportan caracteres ASCII imprimibles (0x20-0x7E)
- El auto-scroll está habilitado por defecto
- La pantalla se actualiza instantáneamente (en una GUI real, usarías `refresh()`)
