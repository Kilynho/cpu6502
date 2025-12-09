## Best Practices

1. **Always update flags correctly** - This is critical for program correctness
2. **Test edge cases** - 0x00, 0xFF, boundary crossings
3. **Document cycle counts** - Include comments about timing
4. **Use helper functions** - `UpdateZeroAndNegativeFlags`, etc.
5. **Log memory access** - For debugging and tracing

## Ejemplo de uso con binario externo

```
./cpu_demo file ../examples/demo_program.bin
```

## Instrucciones soportadas

- LDA (Immediate, Zero Page, Zero Page,X, Absolute, Absolute,X, Absolute,Y)
- LDX (Immediate)
- STA (Zero Page)
- JSR, RTS

Consulta el código fuente para detalles de implementación y logging.
