// SLH-DSA (SPHINCS+) on HORNET: KeyGen, Sign and Verify timed with mcycle,
// outputs digested for comparison with a native build (make check)
#include <stdint.h>
#include <string.h>
#include "bench.h"
#include "../api.h"
#include "../params.h"
#include "../rng.h"

#define SPX_MLEN 32

static unsigned char pk[SPX_PK_BYTES];
static unsigned char sk[SPX_SK_BYTES];
static unsigned char m[SPX_MLEN];
static unsigned char sm[SPX_BYTES + SPX_MLEN];
static unsigned char mout[SPX_BYTES + SPX_MLEN];

int main(void)
{
    unsigned long long smlen, mlen;
    int r;

    bench_start("SPHINCS+-SHAKE-128s-simple");
    for (int run = 0; run < RUNS; run++) {
        randombytes(m, SPX_MLEN);

        BENCH("keygen", run, r = crypto_sign_keypair(pk, sk));
        bench_check(r == 0, "keygen", run);
        BENCH("sign", run, crypto_sign(sm, &smlen, m, SPX_MLEN, sk));
        bench_check(smlen == SPX_BYTES + SPX_MLEN, "signed message length", run);
        BENCH("verify", run, r = crypto_sign_open(mout, &mlen, sm, smlen, pk));
        bench_check(r == 0, "signature verifies", run);
        bench_check(mlen == SPX_MLEN && memcmp(m, mout, SPX_MLEN) == 0, "recovered message", run);
        bench_digest("pk", run, pk, sizeof pk);
        bench_digest("sk", run, sk, sizeof sk);
        bench_digest("sm", run, sm, smlen);

        // flipping a bit of the message must invalidate the signature
        sm[smlen - 1] ^= 1;
        r = crypto_sign_open(mout, &mlen, sm, smlen, pk);
        bench_check(r != 0, "forgery rejected", run);
    }
    printf("pk %d sk %d sig %d bytes\n", SPX_PK_BYTES, SPX_SK_BYTES, SPX_BYTES);
    return bench_end();
}
