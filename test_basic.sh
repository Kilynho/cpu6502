#!/bin/bash
# Script de prueba para verificar Basic en el emulador

cd /home/kilynho/src/cpu6502/build

# Prueba 1: Comandos básicos con timeout
echo "=== Probando Basic con comandos básicos ==="
(
    sleep 2
    echo "10 PRINT \"HELLO\""
    sleep 1
    echo "LIST"
    sleep 1
    echo "RUN"
    sleep 1
    echo ".quit"
) | timeout 15s ./emulator rom.bin 2>&1 | head -100

echo ""
echo "=== Fin de la prueba ==="
