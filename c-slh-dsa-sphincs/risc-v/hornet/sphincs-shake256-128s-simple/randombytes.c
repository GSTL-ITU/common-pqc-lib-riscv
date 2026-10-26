// Deterministic SHAKE256 PRNG for bare-metal testing, the same generator and
// seed as ML-KEM (c-ml-kem-kyber/riscv-hornet/randombytes.c). Not secure: in
// a product this must come from a hardware entropy source.
// (rng.c, the NIST AES-256 CTR DRBG, remains for PQCgenKAT_sign.c on a PC.)
#include <stdint.h>
#include <string.h>
#include "fips202.h"
#include "rng.h"

static uint8_t prng_state[32] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
};

int randombytes(unsigned char *x, unsigned long long xlen)
{
    // SHAKE256(state) gives xlen output bytes followed by the next state
    uint8_t buffer[256 + 32];
    while (xlen > 0) {
        size_t n = xlen > 256 ? 256 : (size_t)xlen;
        shake256(buffer, n + 32, prng_state, 32);
        memcpy(x, buffer, n);
        memcpy(prng_state, buffer + n, 32);
        x += n;
        xlen -= n;
    }
    return RNG_SUCCESS;
}
