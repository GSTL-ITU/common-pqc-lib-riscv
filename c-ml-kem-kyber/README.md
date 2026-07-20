# Kyber (ML-KEM) PQC for RISC-V Bare-Metal (Hornet)

This repository contains the ongoing work to port and test the ML-KEM (Kyber) Post-Quantum Cryptography (PQC) algorithm on a custom RISC-V architecture. The project specifically targets a resource-constrained, bare-metal embedded environment.

## Architecture & Hardware Context

The software in this repository is designed to run on **Hornet**, a custom RV32IMF soft core.
* **Target Processor:** [HORNET-RV32IMF](https://github.com/GSTL-ITU/HORNET-RV32IMF)
* **Execution Environment:** Bare-metal (No OS)
* **Compilation:** `riscv32-unknown-elf-gcc` (`-march=rv32imf -mabi=ilp32f`)

## Acknowledgements & Source Material

The cryptographic source code in this repository is adapted from the official Kyber reference implementation. 
* **Official Kyber Project:** [https://pq-crystals.org/kyber/](https://pq-crystals.org/kyber/)

The codebase has been heavily modified to support deterministic testing in a bare-metal simulator. OS-dependent entropy sources (`/dev/urandom`) and standard C library I/O dependencies (`stdio.h`, `printf`) have been stripped and replaced with custom bare-metal MMIO debugging and a deterministic SHAKE256 PRNG.

## Repository Structure

```text
.
├── avx2/             # Native x86 AVX2 implementation (Ignored for FPGA)
├── ref/              # Standard C reference implementation
├── profiling/        # Profiling tests
└── riscv-hornet/     # Modified bare-metal implementation for Hornet core
    ├── test/         
    │   ├── hornet_kyber.c      # Main test harness (Keygen, Encapsulation, Decapsulation)
    │   ├── linksc.ld           # Custom Hornet memory linker script
    │   ├── Makefile            # Toolchain build script for RV32IMF
    │   ├── memory_init_tb.mem  # Generated hex memory file for simulation
    │   └── verilator.sh        # Simulation script targeting barebones_top_tb
    ├── randombytes.c           # Custom bare-metal deterministic SHAKE256 PRNG
    └── [Core Kyber Source]     # indcpa, ntt, poly, kem, fips202 (unmodified where possible)
```

## Current Status & Simulation

Initial porting efforts are focused on verifying the mathematical correctness and memory footprint of the Kyber algorithm on the Hornet core via RTL simulation.

The repository includes the `hornet_kyber.c` test harness, which validates `crypto_kem_keypair`, `crypto_kem_enc`, and `crypto_kem_dec` alongside deliberate corruption testing. The simulation is executed via **Verilator**.

### Cycle Analysis & Calculation

The Verilator testbench (`barebones_top_tb.v`) drives the clock with a half-period toggle delay of `#12.5` ns. 
This yields a full clock period of **25 ns**, which corresponds to a hardware frequency of **40 MHz**.

To calculate the exact clock cycles consumed by the Kyber operations, we use the following formula:

$$\text{Clock Cycles} = \frac{\text{Total Simulation Time (ns)}}{25 \text{ ns}}$$

**Execution Report:**
* **Start Time:** 588 ns
* **End Time:** 435,572,038 ns
* **Total Execution Time:** 435,571,450 ns
* **Total Clock Cycles:** 17,422,858 cycles

## Profiling

There is a Makefile script included for profiling Dilithium using [Gprof](https://ftp.gnu.org/old-gnu/Manuals/gprof-2.9.1/html_mono/gprof.html).
Running below command will make detailed profiling reports for reference and AVX2 implementations (all parameters included):
```sh
make profile
```
