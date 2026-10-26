# SPHINCS+ for RISC-V bare-metal

This repo contains [SPHINCS+](https://github.com/sphincs/sphincsplus.git) (SLH-DSA) source code and a modified version for a RISC-V core.

## Building and Testing

The Hornet harness (`test/hornet_sphincs.c`) runs on Hornet version 2
(HORNET's startup code, drivers and linker script, `-O3`, output on the UART).
Requirements and the FPGA flow are in the [top-level README](../README.md#running-on-hornet).

```sh
cd risc-v/hornet/sphincs-shake256-128s-simple
make check      # Verilator + native build; CHECK PASS when all digests match
```

It generates a key pair, signs a 32-byte message, verifies it, checks that a
flipped message bit is rejected, and prints the digests of the public key,
secret key and signed message.

### Execution Report (SPHINCS+-SHAKE-128s-simple, -O3, Hornet v2 on the Nexys Video)

Cycles from the 64-bit `mcycle` around each call; the Verilator simulation
gives the same numbers.

| Operation | Cycles (seed 0) | Median of 3 seeds | Min – max | Time @ 50 MHz |
| :--- | ---: | ---: | ---: | ---: |
| **KeyGen** | 6,760,272,813 | overnight run |  | 135,205.5 ms |
| **Sign** | overnight run | overnight run |  |  |
| **Verify** | overnight run | overnight run |  |  |

**Public Key Size:** 32 bytes \
**Secret Key Size:** 64 bytes \
**Signature Size:** 7856 bytes

## Repository Structure

```text
.
├── gcc                                     # Original source code (for reference testing and KAT generation)
│   └── sphincs-shake256-128s-simple        # Reference implementation for "SHAKE256 128s Simple" variant
└── risc-v
    └── hornet
        └── sphincs-shake256-128s-simple    # Modified SPHINCS+ source code and related files for Hornet
```

## Notes

The Hornet harness takes its randomness from `randombytes.c`, a deterministic
SHAKE256 PRNG (the same generator and seed as the ML-KEM port). `rng.c` and
`aes.c` (the NIST AES-256 CTR DRBG on [tiny-AES](https://github.com/kokke/tiny-AES-c.git)
instead of OpenSSL) remain for `PQCgenKAT_sign.c` on a PC. The runtime of
SPHINCS+ does not depend on the randomness.
