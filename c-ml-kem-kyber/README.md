# Kyber (ML-KEM) PQC for RISC-V Bare-Metal (Hornet)

This repository contains the ongoing work to port and test the ML-KEM (Kyber) Post-Quantum Cryptography (PQC) algorithm on a custom RISC-V architecture. The project specifically targets a resource-constrained, bare-metal embedded environment.

## Architecture & Hardware Context

The software in this repository is designed to run on **Hornet**, a custom RV32IMF soft core.
* **Target Processor:** [HORNET-RV32IMF](https://github.com/GSTL-ITU/HORNET-RV32IMF)
* **Execution Environment:** Bare-metal (No OS)
* **Compilation:** `riscv32-unknown-elf-gcc` (`-march=rv32imf_zicsr_zifencei -mabi=ilp32f -O3`, in Hornet's Docker image)

## Acknowledgements & Source Material

The cryptographic source code in this repository is adapted from the official Kyber reference implementation. 
* **Official Kyber Project:** [https://pq-crystals.org/kyber/](https://pq-crystals.org/kyber/)

The codebase has been heavily modified to support deterministic testing in a bare-metal simulator. OS-dependent entropy sources (`/dev/urandom`) and standard C library I/O dependencies (`stdio.h`, `printf`) have been replaced with Hornet's UART output (newlib `printf`) and a deterministic SHAKE256 PRNG.

## Repository Structure

```text
.
├── avx2/             # Native x86 AVX2 implementation (Ignored for FPGA)
├── ref/              # Standard C reference implementation
├── profiling/        # Profiling tests
└── riscv-hornet/     # Bare-metal implementation for the Hornet core
    ├── test/
    │   ├── hornet_kyber.c      # Harness: KeyGen, Encaps, Decaps timed with mcycle + invalid-sk/ct tests
    │   └── Makefile            # make [KYBER_K=2|3|4] [sim|host|check] (../../../hornet/hornet.mk)
    ├── randombytes.c           # Deterministic SHAKE256 PRNG (seed 00..1F)
    └── [Core Kyber Source]     # indcpa, ntt, poly, kem, fips202 (unmodified where possible)
```

## Running on Hornet

The harness runs on Hornet (HORNET's startup code, drivers and
linker script, `-O3`); printf goes to the UART. Requirements and the FPGA flow
are in the [top-level README](../README.md#running-on-hornet).

```sh
cd riscv-hornet/test
make check      # Verilator + native build; CHECK PASS when all digests match
```

`make check` compares the digests of the public key, secret key, ciphertext
and shared secret (and of the shared secret after a changed ciphertext) with a
native build of the same harness.

### Execution Report (Kyber768, -O3, Hornet on the Nexys Video)

Cycles from the 64-bit `mcycle` around each call; the Verilator simulation
gives the same numbers.

| Operation | Cycles (seed 0) | Median of 100 seeds | Min – max | Time @ 50 MHz |
| :--- | ---: | ---: | ---: | ---: |
| **KeyGen** | 1,451,663 | 1,452,442 | 1,451,667 – 1,492,294 | 29.0 ms |
| **Encaps** | 1,645,824 | 1,646,604 | 1,645,829 – 1,686,456 | 32.9 ms |
| **Decaps** | 1,903,942 | 1,904,723 | 1,903,948 – 1,944,575 | 38.1 ms |

## Profiling

There is a Makefile script included for profiling Dilithium using [Gprof](https://ftp.gnu.org/old-gnu/Manuals/gprof-2.9.1/html_mono/gprof.html).
Running below command will make detailed profiling reports for reference and AVX2 implementations (all parameters included):
```sh
make profile
```
