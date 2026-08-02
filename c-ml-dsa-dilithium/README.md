# Dilithium for RISC-V bare-metal

This repo contains [CRYSTALS-Dilithium](https://github.com/pq-crystals/dilithium.git) (ML-DSA) source code and a modified version for a RISC-V core. 

## Building & Testing

For a RISC-V build, you need the [RISC-V GNU Compiler Toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain.git).

For Hornet:
```sh
make riscv_dilithium2
```
or if you are using the multilib version of the gnu toolchain:
```sh
make riscv_dilithium2_multilib
```
where the number after dilithium ranges over the parameter set 2, 3 and 5.

If you want more detailed testing, you can use
```sh
make riscv_test_dilithium2
```
or similarly for the multilib version
```sh
make riscv_test_dilithium2_multilib
```
Running this test will print out execution flow as well as key and signature sizes to the debug address. However, modifying the Hornet debug interface ```debug_interface_wb.v``` is required.

### Execution Report

Below values are for Hornet core running with 100 MHz clock frequency and Dilithium2 parameter set.
| Operation | Time | Clock Cycles |
| :--- | :--- | :--- |
| **Keypair Generation** | 86,098,575 ns | 8,609,857 |
| **Crypto Sign** | 1,051,490,525 ns | 105,149,052 |
| **Crypto Sign Open** | 94,446,325 ns | 9,444,632 |

**Public Key Size:** 1312 bytes \
**Secret Key Size:** 2560 bytes \
**Signature Size:** 2420 bytes 

## Profiling

There is a Makefile script included for profiling Dilithium using [Gprof](https://ftp.gnu.org/old-gnu/Manuals/gprof-2.9.1/html_mono/gprof.html).
Running below command will make detailed profiling reports for reference and AVX2 implementations (all parameters included):
```sh
make profile
```

## Repository Structure

```text
.
├── gcc             # Original source code (for reference testing and KAT generation)
│   ├── avx2        # Implementation for Intel AVX2
│   └── ref         # Reference implementation
│   └── profiling   # Profiling tests
└── risc-v
    └── hornet      # Modified Dilithium source code and related files for Hornet
```

### Notes

The random number generation has been changed with a seeded pseudo rng.

"Time" and "Clock Cycles" values in the execution report are not exact, as the debug prints and if checks alter the cycle counts slightly.
