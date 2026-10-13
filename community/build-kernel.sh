#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2026 Jiaxi Zhang <z1529105815@outlook.com>
set -eu
TASK_ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
TASK_OUT=${OUT:-"$TASK_ROOT/out-community-v141"}
TASK_JOBS=${JOBS:-2}
export ARCH=arm64 LLVM=1 SOURCE_DATE_EPOCH=0
export KBUILD_BUILD_TIMESTAMP='1970-01-01 00:00:00 UTC'
export KBUILD_BUILD_USER=J-x-Z KBUILD_BUILD_HOST=warsaw-builder KBUILD_BUILD_VERSION=1
"$TASK_ROOT/community/initramfs/build.sh"
mkdir -p "$TASK_OUT/usr"
cp "$TASK_ROOT/community/configs/v141-original.config" "$TASK_OUT/.config"
cp "$TASK_ROOT/community/initramfs.cpio" "$TASK_OUT/usr/community-v141-diagnostic.cpio"
cd "$TASK_ROOT"
scripts/config --file "$TASK_OUT/.config" --set-str INITRAMFS_SOURCE usr/community-v141-diagnostic.cpio
make O="$TASK_OUT" olddefconfig
make O="$TASK_OUT" -j"$TASK_JOBS" Image qcom/sm8750-xiaomi-warsaw.dtb
