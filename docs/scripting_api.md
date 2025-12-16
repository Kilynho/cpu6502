---
# API de Scripting y Eventos

Esta documentación describe la API de scripting disponible en el sistema, los eventos (hooks) que pueden ser interceptados y cómo ejecutar scripts externos, especialmente en Python.

## Introducción

El sistema permite extender y automatizar el comportamiento del emulador mediante scripts externos. Los scripts pueden reaccionar a eventos internos (hooks) y manipular el estado de la CPU, memoria y dispositivos.

Actualmente, el scripting está orientado a Python, pero la arquitectura permite agregar otros lenguajes en el futuro.

---

## Ejecución de Scripts

Los scripts pueden ejecutarse de dos formas principales:

1. **Al inicio del emulador**: usando un argumento de línea de comandos, por ejemplo:
   ```bash
   ./cpu_demo --script mi_script.py
   ```
2. **Desde la consola interactiva** (si está habilitada):
   ```
   > run_script("mi_script.py")
   ```

El script se ejecuta en un entorno aislado, con acceso a una API específica para interactuar con el emulador.

---

## Hooks y Eventos Disponibles

Los scripts pueden suscribirse a los siguientes eventos (hooks):

- `on_start()`: Se llama al iniciar la emulación.
- `on_stop()`: Se llama al detener la emulación.
- `on_reset()`: Se llama tras un reset de la CPU.
- `on_instruction(address, opcode)`: Se llama antes de ejecutar cada instrucción 6502.
- `on_memory_read(address, value)`: Se llama al leer memoria.
- `on_memory_write(address, value)`: Se llama al escribir memoria.
- `on_irq()`: Se llama al disparar una IRQ.
- `on_nmi()`: Se llama al disparar una NMI.

Puedes definir cualquiera de estas funciones en tu script para interceptar el evento correspondiente.

---

## API de Scripting (Python)

El entorno de scripting expone los siguientes objetos y funciones:

- `cpu`: Objeto CPU, permite leer/modificar registros (`A`, `X`, `Y`, `PC`, `SP`, `P`).
- `mem`: Objeto Memoria, permite leer/escribir bytes (`mem[0x8000] = 0x42`).
- `log(msg)`: Escribe un mensaje en el log del emulador.
- `breakpoint(address)`: Inserta un breakpoint en la dirección dada.
- `step(cycles)`: Ejecuta un número de ciclos de CPU.
- `run_until(address)`: Ejecuta hasta alcanzar la dirección dada.

---

## Ejemplo de Script en Python

```python
def on_start():
    log("Script iniciado. PC=0x%04X" % cpu.PC)
    breakpoint(0x9000)

def on_instruction(address, opcode):
    if address == 0x9000:
        log("Llegó a 0x9000, valor de A: %02X" % cpu.A)
        cpu.A = 0x42  # Modifica el registro A

def on_memory_write(address, value):
    if address == 0xFFFC:
        log("Escritura en vector de reset: %02X" % value)
```

---

## Uso de Hooks

Para usar un hook, simplemente define la función correspondiente en tu script. El sistema detecta automáticamente qué hooks están implementados y los llama en el momento adecuado.

Puedes combinar varios hooks en un mismo script para monitorear y modificar el comportamiento del emulador en tiempo real.

---

## Buenas Prácticas

- Mantén los scripts cortos y enfocados en una tarea.
- Usa `log()` para depuración y trazabilidad.
- Evita bucles infinitos dentro de los hooks.
- No modifiques el estado de la CPU/memoria salvo que sea necesario.

---

## Ejemplo: Script para Parar en una Instrucción Específica

```python
def on_instruction(address, opcode):
    if address == 0xC000:
        log("Breakpoint en 0xC000, deteniendo ejecución.")
        step(0)  # Detiene la CPU
```

---

## Referencias y Más Información

- Consulta el README y la documentación de cada dispositivo para más ejemplos de integración.
- Para dudas o sugerencias, abre un issue en el repositorio.

---