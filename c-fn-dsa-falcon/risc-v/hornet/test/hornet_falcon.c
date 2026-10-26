// FN-DSA (Falcon) on HORNET: KeyGen, Sign (dynamic, compressed) and Verify
// timed with mcycle; outputs digested for comparison with a native build
// (make check). Floating point is emulated with integers (FALCON_FPEMU).
#include <stddef.h>
#include <stdint.h>
#include "bench.h"
#include "../src/falcon.h"

#ifndef LOGN
#define LOGN 9 // Falcon-512
#endif

#define TMP_SIZE (FALCON_TMPSIZE_KEYGEN(LOGN) > FALCON_TMPSIZE_SIGNDYN(LOGN) ? \
                  FALCON_TMPSIZE_KEYGEN(LOGN) : FALCON_TMPSIZE_SIGNDYN(LOGN))

// 8-byte alignment for the emulated floating-point buffers
static uint8_t tmp[TMP_SIZE] __attribute__((aligned(8)));
static uint8_t privkey[FALCON_PRIVKEY_SIZE(LOGN)];
static uint8_t pubkey[FALCON_PUBKEY_SIZE(LOGN)];
static uint8_t sig[FALCON_SIG_COMPRESSED_MAXSIZE(LOGN)];
static uint8_t msg[32];

int main(void)
{
    shake256_context rng;
    size_t sig_len;
    int r;

    // no entropy source: get_seed() in src/rng.c gives a fixed seed
    shake256_init_prng_from_system(&rng);

    bench_start(LOGN == 9 ? "Falcon-512" : "Falcon-1024");
    for (int run = 0; run < RUNS; run++) {
        shake256_extract(&rng, msg, sizeof msg);

        BENCH("keygen", run, r = falcon_keygen_make(&rng, LOGN, privkey, sizeof privkey,
                                                    pubkey, sizeof pubkey, tmp, sizeof tmp));
        bench_check(r == 0, "keygen", run);
        sig_len = sizeof sig;
        BENCH("sign", run, r = falcon_sign_dyn(&rng, sig, &sig_len, FALCON_SIG_COMPRESSED,
                                               privkey, sizeof privkey, msg, sizeof msg, tmp, sizeof tmp));
        bench_check(r == 0, "sign", run);
        BENCH("verify", run, r = falcon_verify(sig, sig_len, FALCON_SIG_COMPRESSED,
                                               pubkey, sizeof pubkey, msg, sizeof msg, tmp, sizeof tmp));
        bench_check(r == 0, "signature verifies", run);
        bench_digest("pk", run, pubkey, sizeof pubkey);
        bench_digest("sk", run, privkey, sizeof privkey);
        bench_digest("sig", run, sig, sig_len);

        // a changed message must not verify
        msg[0] ^= 1;
        r = falcon_verify(sig, sig_len, FALCON_SIG_COMPRESSED, pubkey, sizeof pubkey, msg, sizeof msg, tmp, sizeof tmp);
        bench_check(r != 0, "forgery rejected", run);
        printf("sig_len %d run %d\n", (int)sig_len, run);
    }
    printf("pk %d sk %d bytes\n", (int)sizeof pubkey, (int)sizeof privkey);
    return bench_end();
}
