# cpu6502

## Cambios recientes (diciembre 2025)

- Ahora `cpu_demo` permite cargar binarios externos usando:
  ```
  ./cpu_demo file ../examples/demo_program.bin
  ```
- Si no se especifica un binario, ejecuta un programa de prueba clásico en memoria.
- Se añadió logging detallado de accesos a memoria en `cpu_log.txt`.
- Argumentos de línea de comandos:
  - `file <ruta>`: carga un binario externo en 0x8000.
  - `infinite`: ejecuta ciclos infinitos.
- Mejoras en la documentación inline y comentarios del código.

### Integración de E/S Apple II (nueva característica)

- La CPU ahora soporta dispositivos de E/S modulares mediante la interfaz `IODevice`.
- Se incluye `AppleIO` para simular el teclado ($FD0C) y pantalla ($FDED) de Apple II.
- Registrar dispositivos IO:
  ```cpp
  auto appleIO = std::make_shared<AppleIO>();
  cpu.registerIODevice(appleIO);
  ```
- Los dispositivos IO interceptan accesos a memoria antes de la lectura/escritura estándar.
- Ideal para extender el emulador con periféricos, timers, gráficos, etc.

Consulta los archivos en `docs/` para detalles de arquitectura e instrucciones soportadas.
