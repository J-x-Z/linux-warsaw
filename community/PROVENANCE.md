# Source provenance and publication boundary

This is an isolated v141 source snapshot. A complete archived base119 tree was recovered, and the original ordered 18-patch v130–v141 sequence was applied without fuzzy context. Six original baseline source pins matched. The resulting board DTB was compiled once and matched the historical v141 DTB byte for byte.

The whole kernel was not rebuilt for publication. Source correspondence is established by the archived source and original build recipe, with additional DTB and entry-code checks; a complete source-to-machine-code reproduction is not claimed. The old early boot marker was confirmed in both recovered source and historical Image.

`PROVENANCE.json` lists the patch identities and affected paths. The tree already contains them. Patch files in `patches/` are historical review references, not an instruction to apply them again.

The original private image had an embedded maintenance environment with private credentials and extracted firmware. It is not published. The community image replaces that entire archive with the fully supplied diagnostic-init source, while retaining the original kernel bytes outside the archive. No latest BSP, recent Git history, runtime logs, private firmware, calibration/NV backup, authentication material or bootloader rescue toolkit is included.

Original Linux COPYING/LICENSES and per-file copyright notices remain intact. The snapshot import author is Jiaxi Zhang; that import does not reattribute upstream or third-party work. No new DCO sign-off for the entire kernel tree is asserted. Publication preparation used Codex assistance.

The historical image reached systemd on the project filesystem and showed the saved framebuffer console. The public packaging derivative has not been boot tested and does not carry those historical whole-system results.
