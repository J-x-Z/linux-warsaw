#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2026 Jiaxi Zhang <z1529105815@outlook.com>
"""Create the complete public diagnostic newc archive with deterministic metadata."""
from pathlib import Path
import argparse, stat
p=argparse.ArgumentParser();p.add_argument('init',type=Path);p.add_argument('output',type=Path);p.add_argument('--pad-to',type=int,default=46440608);a=p.parse_args()
elf=a.init.read_bytes()
if elf[:4]!=b'\x7fELF' or elf[4:6]!=b'\x02\x01' or int.from_bytes(elf[18:20],'little')!=183:
    raise SystemExit('init must be an ELF64 little-endian AArch64 executable')
b=bytearray();ino=0

def add(name,mode,data=b'',major=0,minor=0):
    global ino
    ino+=1;nm=name.encode()+b'\0'
    fields=[ino,mode,0,0,2 if stat.S_ISDIR(mode) else 1,0,len(data),0,0,major,minor,len(nm),0]
    b.extend(b'070701'+''.join(f'{x:08x}' for x in fields).encode());b.extend(nm)
    b.extend(b'\0'*(-len(b)%4));b.extend(data);b.extend(b'\0'*(-len(b)%4))
add('.',stat.S_IFDIR|0o755)
for n in ('dev','proc','sys'):add(n,stat.S_IFDIR|0o755)
add('dev/console',stat.S_IFCHR|0o600,major=5,minor=1)
add('dev/null',stat.S_IFCHR|0o666,major=1,minor=3)
add('init',stat.S_IFREG|0o755,elf)
add('TRAILER!!!',0)
if len(b)>a.pad_to:raise SystemExit('archive exceeds preserved historical initramfs range')
b.extend(b'\0'*(a.pad_to-len(b)));a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_bytes(b)
print(f'Created {a.output.name}: {len(b)} bytes, only init and virtual mount points')
