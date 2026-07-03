# Dilithium for RISC-V bare-metal

This repo contains [CRYSTALS-Dilithium](https://github.com/pq-crystals/dilithium.git) (ML-DSA) source code and a modified version for a RISC-V core. 

## Building & Testing

For a RISC-V build, you need the [RISC-V GNU Compiler Toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain.git).

For Hornet:
```sh
make riscv_test_dilithium2
```
Or if you are using the multilib version of the gnu toolchain:
```sh
make riscv_test_dilithium2_multilib
```

## Repository Structure

```text
.
├── gcc             # Original source code (for reference testing and KAT generation)
│   ├── avx2        # Implementation for Intel AVX2
│   └── ref         # Reference implementation
└── risc-v
    └── hornet      # Modified Dilithium source code and related files for Hornet
```

## Notes

The random number generation has been changed with a seeded pseudo rng.
