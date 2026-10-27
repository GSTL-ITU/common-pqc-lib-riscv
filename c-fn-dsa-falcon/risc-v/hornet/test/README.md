# Falcon on Hornet (`test`)

Bare-metal harness for Falcon-512 on the Hornet RV32IMF core:
`falcon_keygen_make`, `falcon_sign_dyn` (compressed signature of a 32-byte
message) and `falcon_verify`, each timed with the 64-bit `mcycle` counter.
The harness also checks that a changed message is rejected and prints digests
of the public key, private key and signature.

Hornet has single-precision floating point only (F, no D), so Falcon is built
with `FALCON_FPEMU 1`: all 64-bit floating-point operations are emulated with
32-bit integer instructions. The Hornet binary contains no F instruction.

## Running

Requirements and the FPGA flow are in the [top-level README](../../../../README.md#running-on-hornet).

```sh
make check          # Falcon-512 in Verilator + native build; CHECK PASS when all digests match
make LOGN=10 check  # Falcon-1024
```

## Source Code Modification

### `src/rng.c` (deterministic seed)

Falcon seeds its SHAKE256-based PRNG from a system entropy source
(`/dev/urandom`, `CryptGenRandom`), which does not exist bare-metal.
`Zf(get_seed)` was replaced with a deterministic function that fills the
buffer with `i & 0xFF`, so key generation always succeeds and every run is
repeatable (and comparable with the native build and with other cores that
use the same file). Not secure; never use it outside testing.

```c
/* HORNET BARE-METAL PATCH */
int Zf(get_seed)(void *seed, size_t len) {
    uint8_t *buf = (uint8_t *)seed;
    if (len == 0) return 1;
    for (size_t i = 0; i < len; i++) {
        buf[i] = (uint8_t)(i & 0xFF);
    }
    return 1; // 1 means success
}
```

## Results (Falcon-512, -O3, Hornet on the Nexys Video)

Cycles from `mcycle` around each call; the Verilator simulation gives the same
numbers.

| Operation | Cycles (seed 0) | Median of 100 seeds | Min – max | Time @ 50 MHz |
| :--- | ---: | ---: | ---: | ---: |
| **KeyGen** | 144,688,596 | 215,774,632 | 139,201,315 – 643,047,715 | 2,893.8 ms |
| **Sign** | 63,600,336 | 63,625,218 | 63,396,940 – 63,858,191 | 1,272.0 ms |
| **Verify** | 547,378 | 546,961 | 529,543 – 547,974 | 10.9 ms |

Key generation restarts when the sampled polynomials are unsuitable, so its
time varies by a factor of about 4 between seeds; signing and verification
vary little.
