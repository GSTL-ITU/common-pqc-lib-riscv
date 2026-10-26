// Timing and result reporting shared by the HORNET harnesses.
//
// On HORNET every operation is timed with the 64-bit mcycle/minstret
// counters; the same harness also builds with a native compiler (no
// counters), and the digest lines of both must match (make check).
//
// Output lines (UART on HORNET, stdout on the host):
//   clk_hz <Hz>
//   cycles <op> <run> <cycles> <instret>
//   digest <name> <run> <64-bit FNV-1a of the bytes>
//   result PASS | result FAIL <count>
#ifndef BENCH_H
#define BENCH_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifndef RUNS
#define RUNS 1
#endif

#if defined(BENCH_PLATFORM)
// another core supplies bench_cycles(), bench_instret(), bench_clk_hz() and printf,
// e.g. -DBENCH_PLATFORM='"bench_cva6.h"' (cva6-pqc/pqc_tests_fpga/.../pqc_bench)
#include BENCH_PLATFORM
#elif defined(__riscv)
#include "hornet.h"
static inline uint64_t bench_cycles(void) { return hornet_cycles(); }
static inline uint64_t bench_instret(void) { return hornet_instret(); }
static inline uint32_t bench_clk_hz(void) { return hornet_clk_freq(); }
#else
static inline uint64_t bench_cycles(void) { return 0; }
static inline uint64_t bench_instret(void) { return 0; }
static inline uint32_t bench_clk_hz(void) { return 0; }
#endif

static int bench_fails;

// newlib-nano's printf has no 64-bit conversions
static inline const char *bench_u64(uint64_t v, char *buf)
{
    char *p = buf + 21;
    *p = '\0';
    do {
        *--p = (char)('0' + v % 10);
        v /= 10;
    } while (v);
    return p;
}

static inline void bench_report(const char *op, int run, uint64_t cycles, uint64_t instret)
{
    char a[22], b[22];
    printf("cycles %s %d %s %s\n", op, run, bench_u64(cycles, a), bench_u64(instret, b));
}

// Times one statement: BENCH("keygen", run, crypto_sign_keypair(pk, sk));
#define BENCH(op, run, stmt) do {                         \
        uint64_t c0_ = bench_cycles(), i0_ = bench_instret(); \
        stmt;                                              \
        uint64_t c1_ = bench_cycles(), i1_ = bench_instret(); \
        bench_report(op, run, c1_ - c0_, i1_ - i0_);       \
    } while (0)

static inline void bench_digest(const char *name, int run, const void *data, size_t len)
{
    const uint8_t *p = (const uint8_t *)data;
    uint64_t h = 0xcbf29ce484222325ull;
    for (size_t i = 0; i < len; i++) {
        h ^= p[i];
        h *= 0x100000001b3ull;
    }
    printf("digest %s %d %08lx%08lx\n", name, run, (unsigned long)(h >> 32), (unsigned long)(h & 0xffffffffu));
}

static inline void bench_check(int ok, const char *what, int run)
{
    if (!ok) {
        printf("check failed: %s (run %d)\n", what, run);
        bench_fails++;
    }
}

static inline void bench_start(const char *name)
{
    printf("%s runs=%d\n", name, RUNS);
    printf("clk_hz %lu\n", (unsigned long)bench_clk_hz());
}

// main returns this: 0 ends the simulation with PASS
static inline int bench_end(void)
{
    if (bench_fails)
        printf("result FAIL %d\n", bench_fails);
    else
        printf("result PASS\n");
    return bench_fails;
}

#endif
