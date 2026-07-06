#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "../randombytes.h"
#include "../sign.h"

#define MLEN 59
#define CTXLEN 14
#define NTESTS 1

#define DEBUG_IF_ADDR 0x10008010
#define DEBUG_REG     ((volatile uint32_t *)DEBUG_IF_ADDR)

int main(void)
{
  size_t i, j;
  int ret;
  size_t mlen, smlen;
  uint8_t b;
  uint8_t ctx[CTXLEN] = {0};
  uint8_t m[MLEN + CRYPTO_BYTES];
  uint8_t m2[MLEN + CRYPTO_BYTES];
  uint8_t sm[MLEN + CRYPTO_BYTES];
  uint8_t pk[CRYPTO_PUBLICKEYBYTES];
  uint8_t sk[CRYPTO_SECRETKEYBYTES];

  *DEBUG_REG = 10;

  for(i = 0; i < NTESTS; ++i) {
    randombytes(m, MLEN);
    *DEBUG_REG = 11;

    crypto_sign_keypair(pk, sk);
    *DEBUG_REG = 12;
    crypto_sign(sm, &smlen, m, MLEN, ctx, CTXLEN, sk);
    *DEBUG_REG = 13;
    ret = crypto_sign_open(m2, &mlen, sm, smlen, ctx, CTXLEN, pk);
    
    if(ret) {
      // Verification failed
      *DEBUG_REG = 14;
    }
    else *DEBUG_REG = 15;
    if(smlen != MLEN + CRYPTO_BYTES) {
      // Signed message lengths wrong
      *DEBUG_REG = 16;
    }
    else *DEBUG_REG = 17;
    if(mlen != MLEN) {
      // Message lengths wrong
      *DEBUG_REG = 18;
    }
    else *DEBUG_REG = 19;
    for(j = 0; j < MLEN; ++j) {
      if(m2[j] != m[j]) {
        // Messages don't match
        *DEBUG_REG = 20;
      }
    }
    *DEBUG_REG = 21;

    randombytes((uint8_t *)&j, sizeof(j));
    do {
      randombytes(&b, 1);
    } while(!b);
    sm[j % (MLEN + CRYPTO_BYTES)] += b;
    ret = crypto_sign_open(m2, &mlen, sm, smlen, ctx, CTXLEN, pk);
    if(!ret) {
      // Trivial forgeries possible
      *DEBUG_REG = 22;
    }
    else *DEBUG_REG = 23;
  }

  *DEBUG_REG = CRYPTO_PUBLICKEYBYTES;
  *DEBUG_REG = CRYPTO_SECRETKEYBYTES;
  *DEBUG_REG = CRYPTO_BYTES;

  *DEBUG_REG = 100;

  return 0;
}
