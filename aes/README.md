# AES for RISC-V Bare-Metal (Hornet)

This directory contains a bare-metal AES implementation for the Hornet RV32IMF soft core, together with two Hornet programs:
* **`test-aes`:** a known-answer test (KAT) for RTL simulation that checks ECB, CBC, CTR and CCM.
* **`aes-fpga`:** the target firmware for EM side-channel analysis (SCA) on the Nexys 4 DDR FPGA. The MATLAB acquisition and attack flow that drives it lives in [sca-common-tektronix/matlab/yusuf-aes](https://github.com/GSTL-ITU/sca-common-tektronix/tree/main/matlab/yusuf-aes).

AES is not a post-quantum algorithm. It is kept in this repository as the **reference target** for the side-channel setup: a well-understood cipher whose key must be recoverable before the same setup is used on the PQC implementations.

## 🟢 Current Status

* **`test-aes`:** all four modes pass in RTL simulation.
* **`aes-fpga`:** running on the Nexys 4 DDR. The full 128-bit key has been recovered with a first-order CEMA attack (16/16 bytes).

## Architecture & Hardware Context

* **Target Processor:** [HORNET-RV32IMF](https://github.com/GSTL-ITU/HORNET-RV32IMF)
* **Board:** Nexys 4 DDR (Artix-7), core clock 25 MHz
* **Execution Environment:** Bare-metal (No OS)
* **Compilation:** `riscv32-unknown-elf-gcc` (`-march=rv32imf -mabi=ilp32f -O2`)

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
    ├── drivers/              # Hornet UART, IRQ and GPIO drivers
    ├── rom_gen/              # crt0.s + rom_generator (.bin -> .mem)
    ├── test-aes/             # RTL simulation KAT
    │   ├── aes_main.c        # Runs ECB/CBC/CTR/CCM tests, reports via debug register
    │   ├── aes_test_bare.c   # aes_test.c without stdio
    │   ├── linksc.ld         # Hornet linker script
    │   ├── Makefile
    │   └── memory_init_tb.mem
    └── aes-fpga/             # EM SCA target firmware
        ├── aes_sca_main.c    # UART command loop + triggered encryption
        ├── linksc.ld
        ├── Makefile
        └── memory_init.mem   # Load this into the Hornet Vivado project
```

## Building

Both programs use the same flow: compile, extract the binary with `objcopy`, then convert it into a `.mem` file for the Hornet BRAM initialization with `rom_generator`.

Build `rom_generator` once:
```sh
cd hornet/rom_gen
gcc rom_generator.c -o rom_generator
```

**Simulation KAT:**
```sh
cd hornet/test-aes
make build          # -> sim_aes.elf, memory_init_tb.mem
```

**SCA firmware:**
```sh
cd hornet/aes-fpga
make build          # -> aes_sca.elf, memory_init.mem
make inspect        # section sizes + disassembly (aes_sca.lst)
```

## `test-aes` — Simulation Known-Answer Test

`aes_main.c` runs the four mode tests in `aes_test_bare.c` in order. After each test it writes one character to the Hornet debug interface at `0x10008010`, so if the program hangs or crashes, the last character written shows where:

| Character | Meaning |
|---|---|
| `S` | Started |
| `1` / `a` | ECB pass / fail |
| `2` / `b` | CBC pass / fail |
| `3` / `c` | CTR pass / fail |
| `4` / `d` | CCM pass / fail |
| `P` / `F` | All passed / at least one failed |

A passing run prints `S1234P`. Load `memory_init_tb.mem` into the Hornet simulation testbench to run it.

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
* **The trigger is a direct register write** (`*GPIO_REG = 1`), not a call to `gpio_set_trigger()`. There is no call or branch, so the delay from the edge to the first AES instruction is the same every time.
* **Compiler barriers** (`asm volatile("" ::: "memory")`) stop `-O2` from moving any part of the AES across the trigger writes.
* **Interrupts are off while a command is processed.** The UART ISR fills the receive buffer, and when a full frame has arrived, interrupts are disabled until the reply has been sent. No ISR can fire during the encryption.
* **The key schedule is expanded outside the window,** when the `'k'` command arrives or at boot. The window contains only `aes_encrypt()`, with its 10 rounds.

### Hardware mapping

| Signal | Address / pin |
|---|---|
| UART (Wishbone slave 3) | `0x10008010` (TX), `+0x1` (RX) |
| GPIO trigger (slave 5) | `0x10008020` → `gpio_trigger_o` |
| Oscilloscope | `gpio_trigger_o` → AUX In, EM probe → CH4 |

> Note: the header comment in `aes_sca_main.c` still says "EM probe → CH1". The setup uses CH4.

### Security note

This firmware is **deliberately leaky**. It exists to validate a side-channel measurement setup. Do not use `ref/aes.c` where side-channel resistance matters.

---

## License

`ref/` is public domain (Brad Conte). The Hornet drivers, ROM tools and test programs follow the repository's MIT License (see the root `LICENSE`).
