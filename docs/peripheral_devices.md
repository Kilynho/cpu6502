# Integración de Hardware Adicional

Esta guía explica cómo crear y registrar nuevos dispositivos periféricos (VIA, Joystick, etc.) en el emulador.

## Pasos

1. Implementa la interfaz `PeripheralDevice`.
2. Crea la clase del dispositivo (por ejemplo, `VIADevice`).
3. Registra el dispositivo y mapea sus direcciones de E/S.
4. Añade pruebas unitarias.

## Ejemplo de registro

```cpp
// Ejemplo de registro de un dispositivo
std::shared_ptr<PeripheralDevice> via = std::make_shared<VIADevice>();
// cpu.registerPeripheral(via, direccion_base);
```
