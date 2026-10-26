# Dilithium for RISC-V bare-metal

This repo contains [CRYSTALS-Dilithium](https://github.com/pq-crystals/dilithium.git) (ML-DSA) source code and a modified version for a RISC-V core. 

## Building & Testing

The Hornet harness runs on Hornet version 2 (HORNET's startup code, drivers
and linker script, `-O3`, output on the UART). Requirements and the FPGA flow
are in the [top-level README](../README.md#running-on-hornet).

```sh
cd risc-v/hornet/test
make check                     # Dilithium2: Verilator + native build, digests must match
make DILITHIUM_MODE=3 check    # parameter sets 2, 3 and 5
```

The harness (`hornet_dilithium.c`) signs a 59-byte message with a 14-byte
context, verifies it, checks that a changed signature byte is rejected, and
prints the digests of the public key, secret key and signed message.

### Execution Report (Dilithium2, -O3, Hornet v2 on the Nexys Video)

Cycles from the 64-bit `mcycle` around each call; the Verilator simulation
gives the same numbers.

| Operation | Cycles (seed 0) | Median of 100 seeds | Min – max | Time @ 50 MHz |
| :--- | ---: | ---: | ---: | ---: |
| **KeyGen** | 2,912,728 | 2,951,725 | 2,893,265 – 3,029,556 | 58.3 ms |
| **Sign** | 36,103,952 | 8,604,656 | 4,830,581 – 58,419,866 | 722.1 ms |
| **Verify** | 3,222,682 | 3,222,400 | 3,221,991 – 3,222,800 | 64.5 ms |

**Public Key Size:** 1312 bytes \
**Secret Key Size:** 2560 bytes \
**Signature Size:** 2420 bytes

Signing repeats until a candidate signature passes the rejection checks, so
its time depends on the randomness: seed 0 needs many more attempts than
usual. The previous report (Hornet version 1, cycles from simulation
timestamps divided by an assumed 100 MHz clock while the testbench ran at
40 MHz) is replaced by the numbers above.

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
    └── hornet      # Modified Dilithium source code; test/ holds the Hornet harness
```

### Notes

The random number generation has been changed with a seeded pseudo rng.

The cycle counts cover only the timed call (`mcycle` is read right before and after it).
