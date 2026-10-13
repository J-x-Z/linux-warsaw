// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2026 Jiaxi Zhang <z1529105815@outlook.com>
// Publication-only userspace for the archived Warsaw v141 kernel.
// Uses the Linux AArch64 syscall ABI; no libc, firmware, credentials or SSH.
typedef unsigned long size_t;
struct timespec { long tv_sec, tv_nsec; };
static long outfd = 1;
static long sc6(long nr, long a, long b, long c, long d, long e, long f)
{
    register long x0 __asm__("x0") = a;
    register long x1 __asm__("x1") = b;
    register long x2 __asm__("x2") = c;
    register long x3 __asm__("x3") = d;
    register long x4 __asm__("x4") = e;
    register long x5 __asm__("x5") = f;
    register long x8 __asm__("x8") = nr;
    __asm__ volatile("svc #0" : "+r"(x0) : "r"(x1), "r"(x2), "r"(x3),
                     "r"(x4), "r"(x5), "r"(x8) : "memory", "cc");
    return x0;
}
static size_t len(const char *s) { size_t n = 0; while (s[n]) ++n; return n; }
static void put(const char *s)
{
    size_t n = len(s);
    while (n) {
        long r = sc6(64, outfd, (long)s, (long)n, 0, 0, 0);
        if (r == -4) continue;
        if (r <= 0) return;
        s += r; n -= (size_t)r;
    }
}
static void number(long v)
{
    char b[32]; size_t n = 0; unsigned long u;
    if (v < 0) { put("-"); u = (unsigned long)(-(v + 1)) + 1; }
    else u = (unsigned long)v;
    do { b[n++] = (char)('0' + u % 10); u /= 10; } while (u);
    for (size_t i = 0; i < n / 2; ++i) {
        char t = b[i]; b[i] = b[n - i - 1]; b[n - i - 1] = t;
    }
    b[n] = 0; put(b);
}
static void sleep_seconds(long sec)
{
    struct timespec req = { sec, 0 }, rem = { 0, 0 };
    while (sc6(101, (long)&req, (long)&rem, 0, 0, 0, 0) == -4) req = rem;
}
static void file(const char *path)
{
    char b[513]; long fd = sc6(56, -100, (long)path, 0, 0, 0, 0);
    put(path); put(":\n");
    if (fd < 0) { put("unavailable errno="); number(-fd); put("\n"); return; }
    size_t total = 0;
    while (total < 8192) {
        long n = sc6(63, fd, (long)b, 512, 0, 0, 0);
        if (n == -4) continue;
        if (n <= 0) break;
        for (long i = 0; i < n; ++i) if (!b[i]) b[i] = '\n';
        b[n] = 0; put(b); total += (size_t)n;
    }
    sc6(57, fd, 0, 0, 0, 0, 0); put("\n");
}
static void vmount(const char *fs, const char *path)
{
    long r = sc6(40, (long)fs, (long)path, (long)fs, 0, 0, 0);
    if (r < 0) { put("virtual mount "); put(path); put(" errno="); number(-r); put("\n"); }
}
static void run(void) __attribute__((noreturn));
static void run(void)
{
    // Independent child deadline before any mount or diagnostic read.
    long child = sc6(220, 17, 0, 0, 0, 0, 0); // clone(SIGCHLD), separate address space
    if (!child) {
        sleep_seconds(60);
        sc6(142, 0xfee1deadUL, 0x28121969UL, 0x01234567UL, 0, 0, 0);
        for (;;) sleep_seconds(10);
    }
    vmount("devtmpfs", "/dev");
    long fd = sc6(56, -100, (long)"/dev/console", 1 | 256, 0, 0, 0);
    if (fd >= 0) outfd = fd;
    put("\nWarsaw v141 community diagnostic init\n");
    put("Historical kernel and DT; publication-only initramfs, not hardware retested.\n");
    put("No real-root mount, firmware payload, credentials or network service.\n");
    put("A 60-second init-relative reboot is armed; early kernel hangs are outside it.\n");
    if (child < 0) { put("Reboot timer clone failed errno="); number(-child); put("\n"); }
    vmount("proc", "/proc"); vmount("sysfs", "/sys");
    file("/proc/sys/kernel/osrelease");
    file("/proc/device-tree/model"); file("/proc/device-tree/compatible");
    file("/sys/devices/system/cpu/online"); file("/sys/devices/system/cpu/present");
    file("/sys/devices/system/clocksource/clocksource0/current_clocksource");
    file("/proc/bus/input/devices"); file("/proc/uptime");
    put("End of bounded diagnostic snapshot. Reboot targets the existing active slot.\n");
    if (child < 0) {
        sleep_seconds(60);
        sc6(142, 0xfee1deadUL, 0x28121969UL, 0x01234567UL, 0, 0, 0);
    }
    for (;;) sleep_seconds(10);
}
void _start(void) __attribute__((noreturn));
void _start(void) { run(); }
