# Common PQC Libraries for RISC-V

This repository contains vendor-independent, bare-metal C ports of NIST Post-Quantum Cryptography (PQC) algorithms targeted for RISC-V soft cores. This codebase provides a streamlined environment for running, testing, and validating these cryptographic implementations on RISC-V architectures.

## 🟢 Current Status: Software Ready

All modules run on HORNET version 2, in RTL simulation and on the FPGA, with
outputs identical to a native build (see [Running on HORNET](#running-on-hornet)).

## Supported Algorithms

| Algorithm | Type | Status | Upstream Source |
|---|---|---|---|
| **AES-128** (Tiny-AES) | Symetric | 🟢 Software Ready | [kokke/tiny-AES-c](https://github.com/kokke/tiny-AES-C) |
| **ML-KEM** (Kyber) | Key Encapsulation | 🟢 Software Ready | [pq-crystals/kyber](https://github.com/pq-crystals/kyber) |
| **ML-DSA** (Dilithium) | Digital Signature | 🟢 Software Ready | [pq-crystals/dilithium](https://github.com/pq-crystals/dilithium) |
| **SLH-DSA** (Sphincs+) | Digital Signature | 🟢 Software Ready | [sphincs/sphincsplus](https://github.com/sphincs/sphincsplus) |
| **FN-DSA** (Falcon) | Digital Signature | 🟢 Software Ready | [falcon-sign.info](https://falcon-sign.info/) |

## Running on HORNET

Every scheme has a HORNET harness (`*/risc-v/hornet/` or `riscv-hornet/`) that
times KeyGen, Sign/Encaps and Verify/Decaps with the 64-bit `mcycle` counter
and prints a digest of every output (keys, ciphertexts, signatures). The same
harness also builds natively; the digests of both must be identical, so a run
on HORNET is checked against a known answer, not only against itself.

Requirements: a checkout of [HORNET-RV32IMF](https://github.com/GSTL-ITU/HORNET-RV32IMF)
(version 2) next to this repository (or `HORNET_ROOT=<path>`) and its
`hornet-dev` Docker image (`make docker-build` there). The build rules are in
[`hornet/hornet.mk`](hornet/hornet.mk) (`-O3`, `rv32imf_zicsr`/`ilp32f`, HORNET's
startup code, drivers and linker script).

```sh
cd c-ml-dsa-dilithium/risc-v/hornet/test
make check          # Verilator run + native run; prints CHECK PASS when the digests match
make sim            # only the simulation; UART log in build/<prog>/<prog>.uart.log
make host           # only the native build
make RUNS=100 ...   # 100 operations with fresh randomness each (median, spread)
```

| Harness | Directory |
|---|---|
| ML-KEM-768 | `c-ml-kem-kyber/riscv-hornet/test` (`KYBER_K=2/3/4`) |
| ML-DSA-44 | `c-ml-dsa-dilithium/risc-v/hornet/test` (`DILITHIUM_MODE=2/3/5`) |
| FN-DSA-512 | `c-fn-dsa-falcon/risc-v/hornet/test` (`LOGN=9/10`) |
| SLH-DSA-SHAKE-128s | `c-slh-dsa-sphincs/risc-v/hornet/sphincs-shake256-128s-simple` |
| AES-128 KAT, AES SCA target | `aes/hornet/test-aes`, `aes/hornet/aes-fpga` |

**On the FPGA.** Build a HORNET bitstream with the UART bootloader and memories
large enough for every harness, build the harness for the same sizes, and run
it without touching the board (HORNET's `scripts/fpga_run.sh` programs the
bitstream over JTAG as the reset and logs the UART output):

```sh
# in HORNET-RV32IMF
make fpga-build BOARD=nexysvideo FPGA_PROG=sw/examples/gpio BOOTLOADER=1 IMEM_SIZE=0x40000 DMEM_SIZE=0x40000
# in the harness directory
make IMEM_SIZE=0x40000 DMEM_SIZE=0x40000 OUT=build-fpga
<HORNET>/scripts/fpga_run.sh nexysvideo <HORNET>/build/fpga/nexysvideo/gpio/hornet_nexysvideo.bit \
    /dev/ttyUSB0 build-fpga/<prog>.elf build-fpga/board.log
python3 ../../../../hornet/summarize.py build-fpga/board.log --host build-fpga/host.log
```

### Results (HORNET v2, -O3, Nexys Video at 50 MHz)

Cycles from `mcycle` around each call. Seed 0 is the fixed seed of the
deterministic RNGs (the same as in the CVA6 runs); the board and the Verilator
simulation give identical counts. The 100-seed columns come from one board run
with `RUNS=100` (SLH-DSA: 3, its runtime does not depend on the seed); every output of every run matched the native build.

| Scheme | Operation | Cycles (seed 0) | Median | Min – max | Time @ 50 MHz (seed 0) | Seeds |
| :--- | :--- | ---: | ---: | ---: | ---: | ---: |
| ML-KEM-768 | KeyGen | 1,451,663 | 1,452,442 | 1,451,667 – 1,492,294 | 29.0 ms | 100 |
|  | Encaps | 1,645,824 | 1,646,604 | 1,645,829 – 1,686,456 | 32.9 ms | 100 |
|  | Decaps | 1,903,942 | 1,904,723 | 1,903,948 – 1,944,575 | 38.1 ms | 100 |
| ML-DSA-44 | KeyGen | 2,912,728 | 2,951,725 | 2,893,265 – 3,029,556 | 58.3 ms | 100 |
|  | Sign | 36,103,952 | 8,604,656 | 4,830,581 – 58,419,866 | 722.1 ms | 100 |
|  | Verify | 3,222,682 | 3,222,400 | 3,221,991 – 3,222,800 | 64.5 ms | 100 |
| FN-DSA-512 | KeyGen | 144,688,596 | 215,774,632 | 139,201,315 – 643,047,715 | 2,893.8 ms | 100 |
|  | Sign | 63,600,336 | 63,625,218 | 63,396,940 – 63,858,191 | 1,272.0 ms | 100 |
|  | Verify | 547,378 | 546,961 | 529,543 – 547,974 | 10.9 ms | 100 |
| SLH-DSA-SHAKE-128s | KeyGen | 6,760,272,813 | 6,760,272,817 | 6,760,272,817 – 6,760,272,817 | 135,205.5 ms | 3 |
|  | Sign | 51,405,057,698 | 51,404,006,225 | 51,402,253,793 – 51,405,057,705 | 1,028,101.2 ms | 3 |
|  | Verify | 48,422,334 | 49,473,743 | 48,422,341 – 51,226,057 | 968.4 ms | 3 |

### Compared with CVA6

The same harnesses and crypto files, built with the same GCC 15.2 at `-O3`,
run on CVA6 (`cv32a6_ima_sv32_fpga`, 25 MHz, DDR3 behind 8 KiB caches) on the
same Nexys Video board, see [cva6-pqc `pqc_bench`](https://github.com/GSTL-ITU/cva6-pqc/tree/dev/pqc_tests_fpga/cv32a6_ima_sv32_fpga/pqc_bench).
Run i uses the same randomness on both cores, and the CVA6 digests match the
native build as well. Cycles, lower is better; medians over the same 100
seeds (FN-DSA: the first 25).

| Scheme | Operation | CVA6 seed 0 | Hornet seed 0 | Hornet / CVA6 | CVA6 median | Hornet median (same seeds) | Hornet / CVA6 |
| :--- | :--- | ---: | ---: | ---: | ---: | ---: | ---: |
| ML-KEM-768 | KeyGen | 3,088,479 | 1,451,663 | 0.47 | 3,078,144 (100) | 1,452,442 | 0.47 |
|  | Encaps | 3,323,645 | 1,645,824 | 0.50 | 3,327,446 (100) | 1,646,604 | 0.49 |
|  | Decaps | 3,615,412 | 1,903,942 | 0.53 | 3,615,900 (100) | 1,904,723 | 0.53 |
| ML-DSA-44 | KeyGen | 6,555,112 | 2,912,728 | 0.44 | 6,651,464 (100) | 2,951,725 | 0.44 |
|  | Sign | 59,935,646 | 36,103,952 | 0.60 | 15,143,069 (100) | 8,604,656 | 0.57 |
|  | Verify | 6,816,171 | 3,222,682 | 0.47 | 6,813,650 (100) | 3,222,400 | 0.47 |
| FN-DSA-512 | KeyGen | 166,486,014 | 144,688,596 | 0.87 | 292,037,681 (25) | 212,795,676 | 0.73 |
|  | Sign | 69,828,009 | 63,600,336 | 0.91 | 69,790,061 (25) | 63,600,342 | 0.91 |
|  | Verify | 887,572 | 547,378 | 0.62 | 886,922 (25) | 547,100 | 0.62 |
| SLH-DSA-SHAKE-128s | KeyGen | 16,813,876,091 | 6,760,272,813 | 0.40 | not measured | 6,760,272,817 (3 seeds) | — |
|  | Sign | not measured (run > 60 min) | 51,405,057,698 | — | not measured | 51,404,006,225 (3 seeds) | — |
|  | Verify | not measured (run > 60 min) | 48,422,334 | — | not measured | 49,473,743 (3 seeds) | — |

Both cores retire almost the same number of instructions; the difference is
the cycles per instruction (about 1.1 on Hornet, 1.2-2.3 on CVA6), mostly from
the memory system (single-cycle on-chip RAM vs DDR3 behind small caches).

The RNGs are deterministic test generators (see below), and the lattice
schemes' runtimes depend on the randomness: ML-DSA signing at seed 0 restarts
more often than usual and takes several times its median.

## Implementation Notes

*   **Original Codebases:** The core cryptographic implementations retain their original source code and headers. 
*   **Modifications for Bare-Metal:** To facilitate easier RISC-V bare-metal testing and RTL simulation, the Random Number Generator (`rng.c`) has been modified to use hardcoded deterministic values. **Do not use this modified RNG in a production or secure environment.**

---

## License and Acknowledgements

The RISC-V specific wrappers, hardware drivers, ROM generators, and testing environments developed by our project team are distributed under the **MIT License** (see the root `LICENSE` file).

This repository also incorporates several third-party cryptographic reference implementations. We have left all original source code headers and copyright notices intact. These algorithms retain their original, highly permissive open-source licenses:

### Third-Party Licenses

*   **FN-DSA (Falcon):** Provided under the **MIT License**, whose text is included at the start of every source file.
*   **ML-DSA (Dilithium):** Licensed under **Public Domain (CC0)**, **Apache 2.0 License**, or **GPL 2.0 License**. For Keccak and the random number generator, this implementation uses public-domain code from sources and authors listed in the comments at the top of the respective files.
*   **ML-KEM (Kyber):** Licensed under **Public Domain (CC0)** or **Apache 2.0 License**. For Keccak and AES, this implementation uses public-domain code from sources and authors listed in the comments at the top of the respective files.
*   **SLH-DSA (SPHINCS+):** Licensed under **SPDX-License-Identifier: (LicenseRef-SPHINCS-PLUS-Public-Domain OR CC0-1.0 OR 0BSD OR MIT-0) AND MIT**.
