# Linux on Xiaomi Warsaw — historical v141 snapshot
#红米K90 ultra，当前内核源码是提交主线上游前的最后一版用于测试，使用了很多私有补丁不够规范，最新源码已跟随上游到最新版本，正在被上游审核，等待审核通过后大约在2027年年初进行更新。

This repository publishes an archived **v141** development snapshot for community source review and feedback. Warsaw is an SM8750 Xiaomi engineering handset. This is an experimental Linux bring-up tree, not the Android kernel repository and not the current development version.

The source is restored from the historical Linux 7.2.0-rc7 base and the original ordered Warsaw patch sequence. Original Linux/Qualcomm and contributor copyright notices and licensing are retained. The publication commit records the recovered snapshot; it does not claim authorship of the upstream kernel.

## What this snapshot establishes

The saved v141 run reached systemd, mounted the project's existing local ext4 root filesystem, and provided a framebuffer text console and USB maintenance networking. These are historical observations, not a new retest and not validation of every peripheral. They do not establish functional Wi-Fi, audio, GPU rendering, suspend or charging.

This release intentionally excludes later development work. See `community/PROVENANCE.md` for the exact inputs, source/image correspondence and any publication-only changes. The `.img` is provided in GitHub Releases rather than stored in Git history.

## Image and device prerequisites

The historical image is not a general installer. The original v141 initramfs mounts a project-specific root partition read/write. A matching handset variant, firmware/bootloader memory contract and prepared filesystem layout are prerequisites. A successful historical run on the project device is not a compatibility claim for retail phones or another layout. No bootloader, partition-creation, firmware-flashing or locking tool is included.

The archived early entry also writes a fixed physical boot marker and depends on the Warsaw memory layout. Read `community/IMAGE.md` before attempting to run an image. If public packaging changes the original initramfs, the release identifies those changes and does not claim that the repackaged image was hardware tested.

## Feedback

Please open a GitHub issue with the exact release, handset/board variant, build configuration, reproduction steps and the first relevant error. Source review is welcome even without access to this engineering device. Remove private credentials, device identifiers and calibration data from logs before posting them.

The intended maintainer identity is Jiaxi Zhang (`J-x-Z`). Snapshot preparation used Codex assistance. Original contributor attribution remains in the source.

本仓库只公开归档的 v141 阶段，用于社区审查和反馈，不含当前最新版适配。镜像与实机能力的限制见上述说明及 `community/` 文档。
