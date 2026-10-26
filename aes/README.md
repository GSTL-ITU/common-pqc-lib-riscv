# AES for RISC-V Bare-Metal (Hornet)

This directory contains a bare-metal AES implementation for the Hornet RV32IMF soft core, together with two Hornet programs:
* **`test-aes`:** a known-answer test (KAT) for RTL simulation that checks ECB, CBC, CTR and CCM.
* **`aes-fpga`:** the target firmware for EM side-channel analysis (SCA) on the FPGA (Nexys 4 DDR, Nexys Video). The MATLAB acquisition and attack flow that drives it lives in [sca-common-tektronix/matlab/yusuf-aes](https://github.com/GSTL-ITU/sca-common-tektronix/tree/main/matlab/yusuf-aes).

AES is not a post-quantum algorithm. It is kept in this repository as the **reference target** for the side-channel setup: a well-understood cipher whose key must be recoverable before the same setup is used on the PQC implementations.

## 🟢 Current Status

* **`test-aes`:** all four modes pass on Hornet version 2, in RTL simulation and on the Nexys Video; the ciphertext digest matches a native build.
* **`aes-fpga`:** ported to Hornet version 2 (UART protocol checked in simulation). On Hornet version 1 (Nexys 4 DDR) the full 128-bit key was recovered with a first-order CEMA attack (16/16 bytes); the trigger moved to Pmod JA pin 1 in version 2, so the acquisition setup must be rewired before repeating it.

## Architecture & Hardware Context

* **Target Processor:** [HORNET-RV32IMF](https://github.com/GSTL-ITU/HORNET-RV32IMF)
* **Board:** Nexys 4 DDR or Nexys Video (Artix-7), core clock 50 MHz (Hornet version 2)
* **Execution Environment:** Bare-metal (No OS)
* **Compilation:** `riscv32-unknown-elf-gcc` (`-march=rv32imf_zicsr_zifencei -mabi=ilp32f`; `-O3` for the KAT, `-O2` for the SCA target), in Hornet's Docker image

## Acknowledgements & Source Material

The AES implementation (`ref/aes.c`, `ref/aes.h`, `ref/aes_test.c`) is Brad Conte's public-domain reference implementation from [B-Con/crypto-algorithms](https://github.com/B-Con/crypto-algorithms). It is a straightforward byte-oriented FIPS-197 implementation: a table S-box, separate `SubBytes`, `ShiftRows`, `MixColumns` and `AddRoundKey` steps, and **no side-channel countermeasures**. That is intentional, because it is the attack target.

Only two changes were made so that it runs bare-metal. Both are documented in the header of `aes.c`:
1. `<memory.h>` was replaced with `<string.h>`.
2. `malloc`/`free` in the CCM functions were replaced with a stack array of the same size, so no heap (`_sbrk`) is needed.

The NIST test vectors in `kat-vectors/` come from the [AES Algorithm Validation Suite (AESAVS)](https://csrc.nist.gov/projects/cryptographic-algorithm-validation-program/block-ciphers).

## Repository Structure

```text
aes/
├── ref/                      # Brad Conte AES (bare-metal patched)
│   ├── aes.c / aes.h         # AES-128/192/256 + CBC, CTR, CCM
│   └── aes_test.c            # Original host KAT harness (uses printf)
├── kat-vectors/              # NIST AESAVS KAT .rsp files (ECB, CBC, CFB1/8/128, OFB)
├── mct-vectors/              # Reserved for Monte Carlo test vectors (empty)
├── mct-intermediate/         # Reserved (empty)
├── mmt-vectors/              # Reserved for multi-block message test vectors (empty)
└── hornet/
    ├── test-aes/             # KAT + cycle counts on Hornet (make check)
    │   ├── aes_main.c        # Runs ECB/CBC/CTR/CCM tests, times key setup and one block
    │   ├── aes_test_bare.c   # aes_test.c without stdio
    │   └── Makefile          # ../../../hornet/hornet.mk
    └── aes-fpga/             # EM SCA target firmware
        ├── aes_sca_main.c    # UART command loop + triggered encryption
        ├── sca_sim.bin       # UART input of the protocol test in simulation (make sim)
        └── Makefile
```

## Building

Both programs build with the Hornet version 2 flow (`../../hornet/hornet.mk`,
which includes Hornet's `sw/common.mk`: startup code, drivers, linker script,
memory images). Requirements are in the [top-level README](../README.md#running-on-hornet).

```sh
cd hornet/test-aes
make check          # KAT in Verilator + native build

cd hornet/aes-fpga
make                # build/aes_sca.elf, build/imem.hex, build/dmem.hex
make sim            # protocol test: boot byte, encryption, key load, FIPS-197 vector, ping
```

For the board, either put the images into a bitstream (in Hornet:
`make fpga-build FPGA_PROG=<path>/aes/hornet/aes-fpga`) or load
`build/aes_sca.elf` with the bootloader (`scripts/hornet_load.py`).

## `test-aes` — Known-Answer Test

`aes_main.c` runs the four mode tests in `aes_test_bare.c`, then times the
AES-128 key schedule and one block encryption with `mcycle`, and prints the
results on the UART (`result PASS` when every mode passes). On Hornet version
2 (-O3): key schedule 3,474 cycles, one block 3,824 cycles, identical in
simulation and on the board.

## `aes-fpga` — EM Side-Channel Target Firmware

`aes_sca_main.c` turns the board into an encryption oracle: the host sends plaintexts over UART, and the board encrypts them with a secret key while raising a trigger pin for the oscilloscope.

### Protocol

The protocol is UART at 115200 baud and is lockstep: the host must read the full reply before it sends the next command.

| Host sends | Target replies | Meaning |
|---|---|---|
| `0x76` `'v'` | `'H'` | Ping |
| `0x6B` `'k'` + 16 key bytes | `'K'` | Load a new key and expand it. No trigger. |
| `0x70` `'p'` + 16 plaintext bytes | 16 ciphertext bytes | **Triggered** AES-128 encryption |
| anything else | `'E'` | Error / resync byte |

At boot, the firmware encrypts the FIPS-197 vector `6bc1bee2…172a` under the default key and sends `'R'` if the result is `3ad77bb4…ef97`, or `'X'` if it is not.

The default key is the FIPS-197 / SP 800-38A test key `2b7e151628aed2a6abf7158809cf4f3c`. It is active after every reset until a `'k'` command replaces it.

### Why the firmware looks the way it does

Every design choice keeps the measured window clean and repeatable:

* **Only the cipher runs inside the trigger window.** `triggered_encrypt()` raises the GPIO, runs 32 `nop`s, calls `aes_encrypt()`, runs 32 `nop`s and lowers the GPIO. The `nop` padding (a short, fixed delay of at least 32 cycles, since the loop adds its own instructions) keeps the GPIO edge itself out of the region the attack correlates over.
* **The trigger is a direct register write** (`GPIO_OUT = TRIG_PINS`), not a call to `gpio_set_trigger()`. There is no call or branch, so the delay from the edge to the first AES instruction is the same every time.
* **Compiler barriers** (`asm volatile("" ::: "memory")`) stop `-O2` from moving any part of the AES across the trigger writes.
* **Interrupts are off while a command is processed.** The UART ISR fills the receive buffer, and when a full frame has arrived, interrupts are disabled until the reply has been sent. No ISR can fire during the encryption.
* **The key schedule is expanded outside the window,** when the `'k'` command arrives or at boot. The window contains only `aes_encrypt()`, with its 10 rounds.

### Hardware mapping

| Signal | Address / pin |
|---|---|
| UART | `0x2001_0000` (Hornet v2 UART0, RX interrupt = fast interrupt 0) |
| GPIO trigger | `GPIO_OUT` bits 24 and 0 (`0x2002_0000`) → Pmod JA pin 1 and LED0 |
| Oscilloscope | JA pin 1 → AUX In, EM probe → CH4 |


### Security note

This firmware is **deliberately leaky**. It exists to validate a side-channel measurement setup. Do not use `ref/aes.c` where side-channel resistance matters.

---

## License

`ref/` is public domain (Brad Conte). The test programs follow the repository's MIT License (see the root `LICENSE`).
