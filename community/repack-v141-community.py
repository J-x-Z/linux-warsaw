#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-only
# Copyright (C) 2026 Jiaxi Zhang
"""Replace v141's private builtin CPIO with a source-complete RAM diagnostic.

This is a packaging derivative, not a newly built or hardware-tested kernel.
Only the original fixed-length CPIO range may change. The old Image must have
been authenticated against the private archive's original release receipt.
No firmware, keys, BusyBox, shared libraries or kernel modules are accepted in
the diagnostic CPIO. It must contain one static AArch64 /init and directories.
"""
import argparse
import hashlib
import json
import pathlib
import stat
import struct

ORIGINAL_BOOT_SHA256 = 'f6ca444108196b0a19e2dd528d5d8fa7b7311da8debc2f28f604f6ed162b747a'
ORIGINAL_CPIO_SHA256 = '52b41733ce209cc5d09a78cf7005bfeef97bc3f965439e74c31f1583df679d35'
KERNEL_OFFSET = 4096
KERNEL_BYTES = 89397760
CPIO_OFFSET = 37217804
CPIO_BYTES = 46440608
BOOT_BYTES = 89403392


def cpio_entries(data):
    pos = 0
    result = []
    found_trailer = False
    while pos < len(data):
        if data[pos:pos+6] == b'\0'*6 and not any(data[pos:]):
            break
        if data[pos:pos+6] != b'070701':
            raise ValueError('diagnostic CPIO is not uncompressed newc')
        fields = [int(data[pos+6+8*i:pos+14+8*i], 16) for i in range(13)]
        size, namesize = fields[6], fields[11]
        if not 1 <= namesize <= 4096:
            raise ValueError('invalid CPIO name size')
        namebytes = data[pos+110:pos+110+namesize]
        if len(namebytes) != namesize or namebytes[-1:] != b'\0':
            raise ValueError('invalid CPIO name')
        name = namebytes[:-1].decode('ascii')
        start = (pos+110+namesize+3) & ~3
        end = start+size
        if end > len(data):
            raise ValueError('truncated CPIO member')
        body = data[start:end]
        pos = (end+3) & ~3
        if name == 'TRAILER!!!':
            if size or any(data[pos:]):
                raise ValueError('multiple or nonzero trailing CPIO streams')
            found_trailer = True
            break
        if name.startswith('./'):
            name = name[2:]
        if name in ('', '.'):
            name = '.'
        if name in {e['name'] for e in result}:
            raise ValueError('duplicate diagnostic CPIO member')
        mode = fields[1]
        if stat.S_ISDIR(mode):
            if name not in {'.', 'dev', 'proc', 'sys', 'run', 'tmp'} or body:
                raise ValueError('unexpected diagnostic directory')
        elif stat.S_ISREG(mode):
            if name != 'init' or not mode & 0o111:
                raise ValueError('unexpected diagnostic file')
            if body[:6] != b'\x7fELF\x02\x01' or struct.unpack_from('<H', body, 18)[0] != 183:
                raise ValueError('/init is not little-endian ELF64 AArch64')
            phoff = struct.unpack_from('<Q', body, 32)[0]
            phentsize, phnum = struct.unpack_from('<HH', body, 54)
            if phentsize != 56 or phoff+phentsize*phnum > len(body):
                raise ValueError('invalid init ELF program headers')
            if any(struct.unpack_from('<I', body, phoff+i*phentsize)[0] in (2, 3) for i in range(phnum)):
                raise ValueError('dynamic/interpreted diagnostic init is forbidden')
        elif stat.S_ISCHR(mode):
            expected = {'dev/console': (5, 1), 'dev/null': (1, 3)}
            if name not in expected or (fields[9], fields[10]) != expected[name] or size or fields[2] or fields[3]:
                raise ValueError('unexpected diagnostic character device')
        else:
            raise ValueError('diagnostic CPIO accepts only init, directories and exact console/null character nodes')
        result.append({'name':name,'mode':oct(mode),'bytes':size,'rdev':{'major':fields[9], 'minor':fields[10]} if stat.S_ISCHR(mode) else None,'sha256':hashlib.sha256(body).hexdigest()})
    if not found_trailer or len([e for e in result if e['name']=='init']) != 1:
        raise ValueError('missing unique init or CPIO trailer')
    return result


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--original-img', type=pathlib.Path, required=True)
    ap.add_argument('--original-cpio', type=pathlib.Path, required=True)
    ap.add_argument('--diagnostic-cpio', type=pathlib.Path, required=True)
    ap.add_argument('--output', type=pathlib.Path, required=True)
    args = ap.parse_args()
    original = args.original_img.read_bytes()
    oldcpio = args.original_cpio.read_bytes()
    newcpio = args.diagnostic_cpio.read_bytes()
    if hashlib.sha256(original).hexdigest() != ORIGINAL_BOOT_SHA256 or hashlib.sha256(oldcpio).hexdigest() != ORIGINAL_CPIO_SHA256:
        raise ValueError('original image/CPIO do not match the exact historical v141 release')
    if args.output.exists():
        raise ValueError('output already exists')
    if len(original) != BOOT_BYTES or original[:8] != b'ANDROID!':
        raise ValueError('unexpected original boot image')
    if struct.unpack_from('<I', original, 40)[0] != 4 or struct.unpack_from('<I', original, 20)[0] != 1584:
        raise ValueError('unexpected original Android v4 header')
    if struct.unpack_from('<I', original, 8)[0] != KERNEL_BYTES or struct.unpack_from('<I', original, 12)[0] != 0:
        raise ValueError('unexpected kernel/ramdisk lengths')
    if struct.unpack_from('<I', original, 1580)[0] != 0 or original[-64:-60] == b'AVBf':
        raise ValueError('original has a boot signature or AVB footer; do not silently invalidate it')
    if original[KERNEL_OFFSET+56:KERNEL_OFFSET+60] != b'ARM\x64':
        raise ValueError('unexpected Linux ARM64 Image magic')
    start, end = KERNEL_OFFSET+CPIO_OFFSET, KERNEL_OFFSET+CPIO_OFFSET+CPIO_BYTES
    if len(oldcpio) != CPIO_BYTES or original[start:end] != oldcpio:
        raise ValueError('original embedded CPIO does not match authenticated archived payload')
    if original.find(oldcpio) != start or original.find(oldcpio, start+1) != -1:
        raise ValueError('original CPIO range is not unique')
    entries = cpio_entries(newcpio)
    if len(newcpio) > CPIO_BYTES:
        raise ValueError('diagnostic archive exceeds fixed original CPIO range')
    changed = original[:start]+newcpio+b'\0'*(CPIO_BYTES-len(newcpio))+original[end:]
    if len(changed) != len(original) or changed[:start] != original[:start] or changed[end:] != original[end:]:
        raise AssertionError('bytes outside the CPIO range changed')
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_bytes(changed)
    release_sha = hashlib.sha256(changed).hexdigest()
    result = {
        'status':'PACKAGED_NOT_HARDWARE_TESTED',
        'release':'v141-community-sanitized',
        'original_boot_sha256_from_verified_archive':ORIGINAL_BOOT_SHA256,
        'original_cpio_sha256_from_verified_archive':ORIGINAL_CPIO_SHA256,
        'output':args.output.name,'sha256':release_sha,'bytes':len(changed),
        'kernel_release_unchanged':'7.2.0-rc7-J-x-Z-warsaw-stageA-v141',
        'kernel_bytes':KERNEL_BYTES,'external_ramdisk_bytes':0,
        'android_header_version':4,'boot_signature_bytes':0,'avb_footer_present':False,
        'replaced_range':{'boot_offset':start,'Image_offset':CPIO_OFFSET,'bytes':CPIO_BYTES,'new_cpio_bytes':len(newcpio)},
        'outside_cpio_byte_identical':True,'android_header_byte_identical':True,
        'diagnostic_cpio_members':entries,
        'private_original_not_a_public_asset':True,
        'limitations':['Historical drivers and DT unchanged.','Diagnostic init not hardware-tested.','No original firmware, authentication data, foreign binary payload or root filesystem.','No automatic block mount, WLAN, audio firmware, userspace SSH or network service.']
    }
    args.output.with_suffix('.json').write_text(json.dumps(result, indent=2)+'\n')
    args.output.with_suffix('.sha256').write_text(release_sha+'  '+args.output.name+'\n')
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
