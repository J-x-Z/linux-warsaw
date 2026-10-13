# v141 community image

`warsaw-linux-v141-community-sanitized.img` is an **untested packaging derivative** of the archived v141 image. It keeps the historical Linux 7.2.0-rc7 kernel, drivers, device tree and Android boot-image v4 header. There is no external ramdisk, boot signature or AVB footer.

Only the unique embedded initramfs byte range was replaced, with identical length and zero padding; every byte outside that range remains unchanged. The new archive contains a supplied freestanding AArch64 `/init`, virtual filesystem mount points and `/dev/console` / `/dev/null`. It contains no firmware, SSH credentials, SSH server, BusyBox, NFS userspace or kernel modules.

The diagnostic init reads kernel version, board identity, CPU state, clocksource and registered input devices. It does not mount a real filesystem or configure networking. An independent child requests an ordinary reboot after 60 seconds from init; this cannot recover an early kernel hang, guarantee a visible console or override inherited watchdog state. Reboot targets the existing active boot slot. No full hardware test of this derivative has been performed.

## Compatibility and limitations

The archived kernel depends on Warsaw's fixed firmware memory layout and ABL-initialized panel state. Its early assembly writes a boot marker at physical address `0xa36ff000`; this behavior is present in the original v141 binary and is deliberately retained. This is not a generic ARM64 image, a retail-phone compatibility claim or a cold display initialization implementation.

This release supplies source review material and an engineering-device diagnostic artifact, not a partition installer. It supplies no root filesystem. Do not write it over Android, flash it to production slots or boot it on an unverified layout. A temporary RAM boot still requires an unlocked compatible engineering device and a working physical recovery route. No locking, OEM flashing, partitioning or security-chain tools are supplied.

The old private image mounted `/dev/sda35` ext4 read/write and booted the project system. That private initramfs is not included and its historical systemd result is not attributed to this derivative. Wi-Fi, GPU rendering, audio, suspend and charging are not certified by this release. Missing firmware can produce probe errors in this intentionally limited diagnostic environment.

See the release SHA256 file and JSON manifest for image identity. Complete diagnostic-init source and its archive generator are in `community/initramfs/`; `community/BUILD.md` explains source rebuilding and the distinction from the fixed-range packaging operation.
