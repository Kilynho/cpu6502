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

### Integración de E/S Apple II

- La CPU ahora soporta dispositivos de E/S modulares mediante la interfaz `IODevice`.
- Se incluye `AppleIO` para simular el teclado ($FD0C) y pantalla ($FDED) de Apple II.
- Registrar dispositivos IO:
  ```cpp
  auto appleIO = std::make_shared<AppleIO>();
  cpu.registerIODevice(appleIO);
  ```
- Los dispositivos IO interceptan accesos a memoria antes de la lectura/escritura estándar.
- Ideal para extender el emulador con periféricos, timers, gráficos, etc.

### Soporte para Almacenamiento de Archivos (FileDevice)

- Nuevo dispositivo `FileDevice` que permite cargar y guardar binarios desde/hacia archivos del host.
- Dos modos de operación:
  - **API directa en C++**: Métodos `loadBinary()` y `saveBinary()`
  - **Registros mapeados en memoria**: Control desde código 6502 en direcciones `$FE00-$FE4F`
- Perfecto para desarrollo, pruebas y persistencia de datos.
- Ejemplo de uso:
  ```cpp
  auto fileDevice = std::make_shared<FileDevice>(&mem);
  cpu.registerIODevice(fileDevice);
  
  // Cargar programa desde archivo
  fileDevice->loadBinary("programa.bin", 0x8000);
  
  // Guardar datos de memoria
  fileDevice->saveBinary("datos.bin", 0x0200, 256);
  ```
- Ver `docs/file_device.md` para documentación completa y `examples/file_device_demo.cpp` para ejemplos.

Consulta los archivos en `docs/` para detalles de arquitectura e instrucciones soportadas.
