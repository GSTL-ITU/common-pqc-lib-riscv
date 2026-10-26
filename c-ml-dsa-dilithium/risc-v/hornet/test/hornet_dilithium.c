// ML-DSA (Dilithium) on HORNET: KeyGen, Sign and Verify timed with mcycle,
// outputs digested for comparison with a native build (make check)
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "bench.h"
#include "../randombytes.h"
#include "../sign.h"

#define MLEN 59
#define CTXLEN 14

static uint8_t ctx[CTXLEN];
static uint8_t m[MLEN + CRYPTO_BYTES];
static uint8_t m2[MLEN + CRYPTO_BYTES];
static uint8_t sm[MLEN + CRYPTO_BYTES];
static uint8_t pk[CRYPTO_PUBLICKEYBYTES];
static uint8_t sk[CRYPTO_SECRETKEYBYTES];

int main(void)
{
  size_t smlen, mlen;
  uint32_t pos;
  uint8_t b;
  int ret;

  bench_start(CRYPTO_ALGNAME);
  for(int run = 0; run < RUNS; ++run) {
    randombytes(m, MLEN);

    BENCH("keygen", run, crypto_sign_keypair(pk, sk));
    BENCH("sign", run, crypto_sign(sm, &smlen, m, MLEN, ctx, CTXLEN, sk));
    BENCH("verify", run, ret = crypto_sign_open(m2, &mlen, sm, smlen, ctx, CTXLEN, pk));

    bench_check(ret == 0, "signature verifies", run);
    bench_check(smlen == MLEN + CRYPTO_BYTES, "signed message length", run);
    bench_check(mlen == MLEN && memcmp(m, m2, MLEN) == 0, "recovered message", run);
    bench_digest("pk", run, pk, sizeof pk);
    bench_digest("sk", run, sk, sizeof sk);
    bench_digest("sm", run, sm, smlen);

    // a changed byte must not verify
    randombytes((uint8_t *)&pos, sizeof pos);
    do {
      randombytes(&b, 1);
    } while(!b);
    sm[pos % (MLEN + CRYPTO_BYTES)] += b;
    ret = crypto_sign_open(m2, &mlen, sm, smlen, ctx, CTXLEN, pk);
    bench_check(ret != 0, "forgery rejected", run);
  }
  printf("pk %d sk %d sig %d bytes\n", CRYPTO_PUBLICKEYBYTES, CRYPTO_SECRETKEYBYTES, CRYPTO_BYTES);
  return bench_end();
}
