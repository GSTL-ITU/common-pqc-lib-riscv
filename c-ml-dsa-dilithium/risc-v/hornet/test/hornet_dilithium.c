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

void fail(void) 
{
    *DEBUG_REG = 'F';
    while(1) {
        __asm__ volatile ("nop");
    }
}

int main(void)
{
  // Signal start
  *DEBUG_REG = 'S';

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


  for(i = 0; i < NTESTS; ++i) {
    randombytes(m, MLEN);

    crypto_sign_keypair(pk, sk);
    crypto_sign(sm, &smlen, m, MLEN, ctx, CTXLEN, sk);
    ret = crypto_sign_open(m2, &mlen, sm, smlen, ctx, CTXLEN, pk);
    
    if(ret) {
      // Verification failed
      fail();
    }
    if(smlen != MLEN + CRYPTO_BYTES) {
      // Signed message lengths wrong
      fail();
    }
    if(mlen != MLEN) {
      // Message lengths wrong
      fail();
    }
    for(j = 0; j < MLEN; ++j) {
      if(m2[j] != m[j]) {
        // Messages don't match
        fail();
      }
    }

    randombytes((uint8_t *)&j, sizeof(j));
    do {
      randombytes(&b, 1);
    } while(!b);
    sm[j % (MLEN + CRYPTO_BYTES)] += b;
    ret = crypto_sign_open(m2, &mlen, sm, smlen, ctx, CTXLEN, pk);
    if(!ret) {
      // Trivial forgeries possible
      fail();
    }
  }

  // Pass
  *DEBUG_REG = 'P';

  while(1) {
    __asm__ volatile ("nop");
  }

  return 0;
}
