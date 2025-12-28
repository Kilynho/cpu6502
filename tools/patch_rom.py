#!/usr/bin/env python3
"""Simple ROM patcher for cpu6502: inject a JMP $FF24 at CPU $FF08.

File mapping: file offset 0x7F00 == CPU $FF00. We write 3 bytes at offset 0x7F08.
"""
from pathlib import Path
rom = Path('src/wozmon/rom.bin')
if not rom.exists():
    print(f"ROM not found: {rom}")
    raise SystemExit(1)

# JMP ABS opcode 4C <lo> <hi>
patch_offset = 0x7F00 + 0x08
patch = bytes([0x4C, 0x24, 0xFF])  # JMP $FF24

data = rom.read_bytes()
if len(data) < patch_offset + len(patch):
    print(f"ROM too small ({len(data)} bytes) for patch at offset {patch_offset:#x}")
    raise SystemExit(2)

old = data[patch_offset:patch_offset+len(patch)]
if old == patch:
    print("Patch already applied.")
    raise SystemExit(0)

new = bytearray(data)
new[patch_offset:patch_offset+len(patch)] = patch
rom.write_bytes(bytes(new))
print(f"Wrote JMP $FF24 at file offset {patch_offset:#x} (overwrote {old.hex()})")
