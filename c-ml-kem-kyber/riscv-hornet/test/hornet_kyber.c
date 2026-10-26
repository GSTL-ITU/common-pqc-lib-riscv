// ML-KEM (Kyber) on HORNET: KeyGen, Encaps and Decaps timed with mcycle,
// plus the reference tests with a random secret key and a changed ciphertext;
// outputs digested for comparison with a native build (make check)
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "bench.h"
#include "../kem.h"
#include "../randombytes.h"

static uint8_t pk[CRYPTO_PUBLICKEYBYTES];
static uint8_t sk[CRYPTO_SECRETKEYBYTES];
static uint8_t ct[CRYPTO_CIPHERTEXTBYTES];
static uint8_t key_a[CRYPTO_BYTES];
static uint8_t key_b[CRYPTO_BYTES];

int main(void)
{
  uint32_t pos;
  uint8_t b;

  bench_start(CRYPTO_ALGNAME);
  for(int run = 0; run < RUNS; run++) {
    // Alice generates a key pair, Bob encapsulates, Alice decapsulates
    BENCH("keygen", run, crypto_kem_keypair(pk, sk));
    BENCH("encaps", run, crypto_kem_enc(ct, key_b, pk));
    BENCH("decaps", run, crypto_kem_dec(key_a, ct, sk));
    bench_check(memcmp(key_a, key_b, CRYPTO_BYTES) == 0, "shared secrets match", run);
    bench_digest("pk", run, pk, sizeof pk);
    bench_digest("sk", run, sk, sizeof sk);
    bench_digest("ct", run, ct, sizeof ct);
    bench_digest("ss", run, key_a, sizeof key_a);

    // a random secret key must give a different shared secret
    crypto_kem_keypair(pk, sk);
    crypto_kem_enc(ct, key_b, pk);
    randombytes(sk, CRYPTO_SECRETKEYBYTES);
    crypto_kem_dec(key_a, ct, sk);
    bench_check(memcmp(key_a, key_b, CRYPTO_BYTES) != 0, "invalid secret key", run);

    // so must a changed ciphertext byte
    do {
      randombytes(&b, sizeof b);
    } while(!b);
    randombytes((uint8_t *)&pos, sizeof pos);
    crypto_kem_keypair(pk, sk);
    crypto_kem_enc(ct, key_b, pk);
    ct[pos % CRYPTO_CIPHERTEXTBYTES] ^= b;
    crypto_kem_dec(key_a, ct, sk);
    bench_check(memcmp(key_a, key_b, CRYPTO_BYTES) != 0, "invalid ciphertext", run);
    bench_digest("ss_bad", run, key_a, sizeof key_a);
  }
  printf("pk %d sk %d ct %d bytes\n", CRYPTO_PUBLICKEYBYTES, CRYPTO_SECRETKEYBYTES, CRYPTO_CIPHERTEXTBYTES);
  return bench_end();
}
