#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../api.h"
#include "../params.h"
#include "../rng.h"

#define SPX_MLEN 32
#define SPX_SIGNATURES 1

#define DEBUG_IF_ADDR 0x10008010
#define DEBUG_REG     ((volatile uint32_t *)DEBUG_IF_ADDR)

int main()
{
    int ret = 0;
    int i;
    *DEBUG_REG = 0;

    unsigned char pk[SPX_PK_BYTES];
    unsigned char sk[SPX_SK_BYTES];
    unsigned char m[SPX_MLEN];
    unsigned char sm[SPX_BYTES + SPX_MLEN];
    unsigned char mout[SPX_BYTES + SPX_MLEN];
    unsigned long long smlen;
    unsigned long long mlen;

    randombytes(m, SPX_MLEN);

    *DEBUG_REG = 1;
    
    // Generating keypair..

    if (crypto_sign_keypair(pk, sk)) { 
        // failed!
        *DEBUG_REG = 2;
    }
    else { 
        // successful
        *DEBUG_REG = 3;
    }

    // Testing signatures..
    
    for (i = 0; i < SPX_SIGNATURES; i++) {

        crypto_sign(sm, &smlen, m, SPX_MLEN, sk);

        if (smlen != SPX_BYTES + SPX_MLEN) {
            // smlen incorrect
            *DEBUG_REG = 5;
        }
        else {
            // smlen as expected 
            *DEBUG_REG = 6;
        }

        /* Test if signature is valid. */
        if (crypto_sign_open(mout, &mlen, sm, smlen, pk)) { 
            // verification failed!
            *DEBUG_REG = 7;
        }
        else { 
            // verification succeeded
            *DEBUG_REG = 8;
        }
        
        /* Test if the correct message was recovered. */
        if (mlen != SPX_MLEN) {
            // mlen incorrect
            *DEBUG_REG = 9;
        }
        else {
            // mlen as expected
            *DEBUG_REG = 10;
        }

        if (memcmp(m, mout, SPX_MLEN)) {
            // output message incorrect!
            *DEBUG_REG = 11;
        }
        else {
            // output message as expected
            *DEBUG_REG = 12;
        }

        /* Test if signature is valid when validating in-place. */
        if (crypto_sign_open(sm, &mlen, sm, smlen, pk)) {
            // in-place verification failed!
            *DEBUG_REG = 13;
        }
        else {
            // in-place verification succeeded
            *DEBUG_REG = 14;
        }

        /* Test if flipping bits invalidates the signature (it should). */

        /* Flip the first bit of the message. Should invalidate. */
        sm[smlen - 1] ^= 1;

        if (!crypto_sign_open(mout, &mlen, sm, smlen, pk)) {
            // flipping a bit of m DID NOT invalidate signature!
            *DEBUG_REG = 15;
        }
        else {
            // flipping a bit of m invalidates signature
            *DEBUG_REG = 16;
        }

        sm[smlen - 1] ^= 1;

        *DEBUG_REG = 17;
    }
}
