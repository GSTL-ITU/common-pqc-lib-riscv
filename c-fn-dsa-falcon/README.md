# Falcon PQC for RISC-V Bare-Metal (Hornet)

This repository contains the ongoing work to port and test the Falcon Post-Quantum Cryptography (PQC) algorithm on a custom RISC-V architecture. The project specifically targets a resource-constrained, bare-metal embedded environment utilizing software-emulated 64-bit floating-point operations.

## Architecture & Hardware Context

The software in this repository is designed to run on **Hornet**, a custom RV32IMF soft core.
* **Target Processor:** [HORNET-RV32IMF](https://github.com/GSTL-ITU/HORNET-RV32IMF)
* **Execution Environment:** Bare-metal (No OS)
* **Compilation:** `riscv32-unknown-elf-gcc` (`-march=rv32imf_zicsr_zifencei -mabi=ilp32f -O3`, in Hornet's Docker image)

## Acknowledgements & Source Material

The cryptographic source code in this repository is adapted from the official Falcon implementation. 
* **Official Falcon Project:** [https://falcon-sign.info/](https://falcon-sign.info/)

The codebase has been modified to support deterministic testing in a bare-metal simulator, bypassing OS-dependent entropy sources and enforcing 32-bit integer emulation for all FPU calculations.

## Repository Structure

```text
.
├── gcc-x86/                  # Native PC implementation (used for reference testing and KAT generation)
└── risc-v/
    └── hornet/
        ├── src/              # Modified Falcon source code configured for RV32 FPU emulation
        └── test/             # Hornet harness: KeyGen, Sign, Verify (make check)
```
## Current Status

The harness in [`risc-v/hornet/test`](risc-v/hornet/test/README.md) runs key
generation, signing (dynamic, compressed signatures) and verification on
Hornet, in Verilator and on the FPGA, and checks every output
against a native build. Results and how to run it: [test/README.md](risc-v/hornet/test/README.md).
