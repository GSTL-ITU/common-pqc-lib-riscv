# SPHINCS+ for RISC-V bare-metal

This repo contains [SPHINCS+](https://github.com/sphincs/sphincsplus.git) (SLH-DSA) source code and a modified version for a RISC-V core.

## Status

The SPHINCS+-SHAKE-128s-simple sources under `risc-v/hornet/` are kept for
reference; SLH-DSA is not part of the HORNET benchmark harnesses (see the
[top-level README](../README.md#running-on-hornet)).

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

The RISC-V sources use [tiny-AES](https://github.com/kokke/tiny-AES-c.git)
(`aes.c`, NIST AES-256 CTR DRBG in `rng.c`) instead of OpenSSL.
