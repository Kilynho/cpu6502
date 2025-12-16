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
- `on_breakpoint(address)`: Se llama al alcanzar un breakpoint en la dirección dada.
- `on_io(address, value)`: Se llama al realizar operaciones de E/S.

Puedes definir cualquiera de estas funciones en tu script para interceptar el evento correspondiente.

---

## API de Scripting (Python)

El entorno de scripting actualmente expone la clase `ScriptingAPI` con los siguientes métodos:

- `on_start(callback)`: Registra un callback para el evento de inicio.
- `on_stop(callback)`: Registra un callback para el evento de parada.
- `on_breakpoint(callback)`: Registra un callback para eventos de breakpoint (recibe `address`).
- `on_io(callback)`: Registra un callback para eventos de E/S (recibe `address` y `value`).

Para pruebas y desarrollo, también están disponibles los métodos de disparo manual:
- `trigger_start()`: Dispara manualmente el evento de inicio.
- `trigger_stop()`: Dispara manualmente el evento de parada.
- `trigger_breakpoint(address)`: Dispara manualmente un evento de breakpoint.
- `trigger_io(address, value)`: Dispara manualmente un evento de E/S.

---

## Ejemplo de Script en Python

```python
import cpu6502

# Crear instancia de la API
api = cpu6502.ScriptingAPI()

# Registrar callbacks
def on_start_handler():
    print("Emulación iniciada")

def on_breakpoint_handler(address):
    print(f"Breakpoint alcanzado en 0x{address:04X}")

def on_io_handler(address, value):
    print(f"Operación de E/S en 0x{address:04X}, valor: 0x{value:02X}")

api.on_start(on_start_handler)
api.on_breakpoint(on_breakpoint_handler)
api.on_io(on_io_handler)

# Disparar eventos para pruebas
api.trigger_start()
api.trigger_breakpoint(0x9000)
api.trigger_io(0xFFFC, 0x42)
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

## Ejemplo: Script para Monitorear Eventos

```python
import cpu6502

api = cpu6502.ScriptingAPI()

def on_start_handler():
    print("Sistema iniciado, esperando eventos...")

def on_breakpoint_handler(address):
    if address == 0xC000:
        print("Breakpoint crítico en 0xC000 alcanzado")

api.on_start(on_start_handler)
api.on_breakpoint(on_breakpoint_handler)

# Simular ejecución
api.trigger_start()
api.trigger_breakpoint(0xC000)
```

---

## Referencias y Más Información

- Consulta el README y la documentación de cada dispositivo para más ejemplos de integración.
- Para dudas o sugerencias, abre un issue en el repositorio.

---