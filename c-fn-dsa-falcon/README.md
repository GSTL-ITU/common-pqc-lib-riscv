# Falcon PQC for RISC-V Bare-Metal (Hornet)

This repository contains the ongoing work to port and test the Falcon Post-Quantum Cryptography (PQC) algorithm on a custom RISC-V architecture. The project specifically targets a resource-constrained, bare-metal embedded environment utilizing software-emulated 64-bit floating-point operations.

## Architecture & Hardware Context

The software in this repository is designed to run on **Hornet**, a custom RV32IMF soft core.
* **Target Processor:** [HORNET-RV32IMF](https://github.com/GSTL-ITU/HORNET-RV32IMF)
* **Execution Environment:** Bare-metal (No OS)
* **Compilation:** `riscv32-unknown-elf-gcc` (`-march=rv32imf -mabi=ilp32f`)

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
        ├── drivers/          # Hornet-specific bare-metal drivers (UART, GPIO, IRQ)
        ├── rom_gen/          # Boot code (crt0.s) and ROM generation utilities
        ├── src/              # Modified Falcon source code configured for RV32 FPU emulation
        └── sw_keygen_test/   # Dedicated test harness for Falcon Key Generation
```
## Current Status & Simulation

Initial porting efforts are focused on verifying the mathematical correctness of the FPU emulation on the Hornet core via simulation. 

Currently, the repository includes the `sw_keygen_test`, which successfully validates the `falcon_keygen_make` function. A detailed execution report and cycle analysis from the **Cadence Xcelium** simulation can be found in the test's dedicated documentation: 
👉 [sw_keygen_test/README.md](risc-v/hornet/sw_keygen_test/README.md)        