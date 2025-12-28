#!/usr/bin/env python3
from pathlib import Path
import sys

if len(sys.argv) < 2:
    print('Usage: inject_wozmon.py <woz_bin>')
    raise SystemExit(2)

woz = Path(sys.argv[1])
repo_root = Path(__file__).resolve().parent.parent
rom = Path(sys.argv[2]) if len(sys.argv) > 2 else repo_root / 'src' / 'wozmon' / 'rom.bin'
if not woz.exists() or not rom.exists():
    print('Missing files:', woz, rom)
    raise SystemExit(1)

b = bytearray(rom.read_bytes())
wb = woz.read_bytes()
offset = 0x7F00
for i, v in enumerate(wb):
    if offset + i < len(b):
        b[offset + i] = v
rom.write_bytes(bytes(b))
print(f'Injected {woz} into {rom} at offset {hex(offset)}')
