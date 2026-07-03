# SPHINCS+ for RISC-V bare-metal

This repo contains [SPHINCS+](https://github.com/sphincs/sphincsplus.git) (SLH-DSA) source code and a modified version for a RISC-V core.

## Building and Testing 

For a RISC-V build, you need the [RISC-V GNU Compiler Toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain.git).

For Hornet:
```sh
make riscv_spx
```
Or if you are using the multilib version of the gnu toolchain:
```sh
make riscv_spx_multilib
```

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

RISC-V bare metal implementation uses [tiny-AES](https://github.com/kokke/tiny-AES-c.git) library instead of OpenSSL.
