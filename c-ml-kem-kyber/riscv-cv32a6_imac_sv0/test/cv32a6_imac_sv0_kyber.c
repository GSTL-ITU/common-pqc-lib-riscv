#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "../kem.h"
#include "../randombytes.h"

// ==============================================================================
// FESVR / HTIF Testbench Communication
// ==============================================================================
extern volatile uint64_t tohost;
extern volatile uint64_t fromhost;

#define NTESTS 1

static int test_keys(void)
{
  uint8_t pk[CRYPTO_PUBLICKEYBYTES];
  uint8_t sk[CRYPTO_SECRETKEYBYTES];
  uint8_t ct[CRYPTO_CIPHERTEXTBYTES];
  uint8_t key_a[CRYPTO_BYTES];
  uint8_t key_b[CRYPTO_BYTES];

  crypto_kem_keypair(pk, sk);
  crypto_kem_enc(ct, key_b, pk);
  crypto_kem_dec(key_a, ct, sk);

  if(memcmp(key_a, key_b, CRYPTO_BYTES)) {
    return 1; 
  }
  return 0; 
}

static int test_invalid_sk_a(void)
{
  uint8_t pk[CRYPTO_PUBLICKEYBYTES];
  uint8_t sk[CRYPTO_SECRETKEYBYTES];
  uint8_t ct[CRYPTO_CIPHERTEXTBYTES];
  uint8_t key_a[CRYPTO_BYTES];
  uint8_t key_b[CRYPTO_BYTES];

  crypto_kem_keypair(pk, sk);
  crypto_kem_enc(ct, key_b, pk);

  randombytes(sk, CRYPTO_SECRETKEYBYTES);
  crypto_kem_dec(key_a, ct, sk);

  if(!memcmp(key_a, key_b, CRYPTO_BYTES)) {
    return 1;
  }
  return 0;
}

static int test_invalid_ciphertext(void)
{
  uint8_t pk[CRYPTO_PUBLICKEYBYTES];
  uint8_t sk[CRYPTO_SECRETKEYBYTES];
  uint8_t ct[CRYPTO_CIPHERTEXTBYTES];
  uint8_t key_a[CRYPTO_BYTES];
  uint8_t key_b[CRYPTO_BYTES];
  uint8_t b;
  size_t pos;

  do {
    randombytes(&b, sizeof(uint8_t));
  } while(!b);
  randombytes((uint8_t *)&pos, sizeof(size_t));

  crypto_kem_keypair(pk, sk);
  crypto_kem_enc(ct, key_b, pk);

  ct[pos % CRYPTO_CIPHERTEXTBYTES] ^= b;
  crypto_kem_dec(key_a, ct, sk);

  if(!memcmp(key_a, key_b, CRYPTO_BYTES)) {
    return 1;
  }
  return 0;
}

int main(void)
{
  // Prints temporarily disabled to prevent AXI memory faults.
  // We will let FESVR handle the Pass/Fail signaling.
  
  unsigned int i;
  int r = 0;

  for(i = 0; i < NTESTS; i++) {
    r |= test_keys();
    r |= test_invalid_sk_a();
    r |= test_invalid_ciphertext();
    
    if(r) {
      // FESVR Exit Code formula: (code << 1) | 1
      tohost = (1ULL << 1) | 1; 
      while(1) { __asm__ volatile ("wfi"); }
    }
  }
  
  // FESVR Success Code is 1
  tohost = 1; 
  while(1) { __asm__ volatile ("wfi"); }

  return 0;
}