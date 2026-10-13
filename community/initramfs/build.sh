#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2026 Jiaxi Zhang <z1529105815@outlook.com>
set -eu
TASK_INIT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
TASK_CLANG=${CLANG:-clang}
"$TASK_CLANG" --target=aarch64-linux-gnu -fuse-ld=lld -O2 \
  -ffreestanding -fno-builtin -fno-stack-protector -fno-pie \
  -nostdlib -static -Wl,-e,_start -Wl,--build-id=none \
  -Wl,-z,max-page-size=4096 -Wall -Wextra -Werror \
  "$TASK_INIT_DIR/init.c" -o "$TASK_INIT_DIR/init"
python3 "$TASK_INIT_DIR/mkinitramfs.py" "$TASK_INIT_DIR/init" \
  "$TASK_INIT_DIR/../initramfs.cpio" --pad-to 46440608
