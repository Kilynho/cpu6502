#!/bin/bash
# Script de prueba para verificar Basic en el emulador

cd /home/kilynho/src/cpu6502/build

# Prueba 1: Comandos básicos con timeout
echo "=== Probando Basic con comandos básicos ==="
(
    sleep 2
    echo "32768"  # Responder a MEMORY SIZE con 32768
    sleep 1
    echo "40"  # Responder a TERMINAL WIDTH con 40
    sleep 2
    echo "10 PRINT \"HELLO\""
    sleep 1
    echo "20 PRINT \"WORLD\""
    sleep 1
    echo "30 PRINT 2+2"
    sleep 1
    echo "LIST"
    sleep 1
    echo "RUN"
    sleep 2
    echo "NEW"
    sleep 1
    echo 'PRINT "TEST"'
    sleep 1
    echo "PRINT 10*5"
    sleep 1
    echo ".quit"
) | timeout 30s ./emulator rom.bin 2>&1

echo ""
echo "=== Fin de la prueba ==="
