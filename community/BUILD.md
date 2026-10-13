# Building the historical source

The kernel tree is already the reconstructed v141 source: do not reapply the historical patch list to it. The original resolved configuration is `configs/v141-original.config`.

The original `CONFIG_INITRAMFS_SOURCE` names a private archived initramfs. That payload is intentionally not published. Build the publication-only diagnostic init and its deterministic archive instead:

```sh
CLANG=clang ./community/initramfs/build.sh
```

This requires LLVM Clang/LLD with the AArch64 Linux target and Python 3; it needs no libc, BusyBox, TinySSH, firmware or cross sysroot. The archive is zero-padded to the historical 46,440,608-byte initramfs range. Its only program is the supplied freestanding `/init`.

Before a clean kernel build, copy the original config into a separate output directory, change only `CONFIG_INITRAMFS_SOURCE` to the absolute path of `community/initramfs.cpio`, and resolve the configuration. See `build-kernel.sh` for the historical built-in DT producer and full invocation.

A newly compiled kernel will contain the new initramfs and current build metadata. It is not promised to match the old kernel binary byte for byte. The published `.img` was instead made by replacing the exact original embedded initramfs range while keeping every kernel byte outside that range unchanged. The original private image is not distributed. Exact source recovery, a byte-identical historical DTB, and preservation of the published image's kernel bytes do not substitute for a complete clean-build binary reproduction.

The supplied diagnostic init does not mount the project root, load firmware or modules, start SSH, configure networking or change boot slots. It reads virtual kernel interfaces and arms an init-relative 60-second ordinary reboot. That timer cannot recover a failure before init starts, and reboot returns to the device's existing active slot.
