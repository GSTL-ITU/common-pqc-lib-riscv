# ==============================================================================
# Toolchain Setup
# ==============================================================================
CV_SW_TOOLCHAIN  ?= /opt/riscv
RISCV            ?= $(CV_SW_TOOLCHAIN)
# Updated to match your xPack multilib compiler prefix
RISCV_EXE_PREFIX ?= $(RISCV)/bin/riscv-none-elf-
RISCV_GCC = $(RISCV_EXE_PREFIX)gcc
RISCV_AR  = $(RISCV_EXE_PREFIX)ar
RISCV_OBJCOPY = $(RISCV_EXE_PREFIX)objcopy

# ==============================================================================
# Architecture Configuration
# TARGET: cv32a6_imac_sv0 (32-bit, Integer, Mult, Atomics, Compressed, No FPU)
# ==============================================================================
ARCH_FLAGS = -march=rv32imac_zicsr -mabi=ilp32

# ==============================================================================
# BSP (Board Support Package) Variables
# ==============================================================================
BSP_SRC = crt0.S handlers.S syscalls.c vectors.S
BSP_OBJ = crt0.o handlers.o syscalls.o vectors.o
LIBCV_VERIF = libcv-verif.a 
BSP_CFLAGS = -Os -g -static $(ARCH_FLAGS) -Wall -pedantic

# ==============================================================================
# Kyber Application Variables
# ==============================================================================
TARGET = cva6_kyber

# All Kyber core submodules located one directory up
KYBER_SRC = ../cbd.c ../fips202.c ../indcpa.c ../kem.c ../ntt.c ../poly.c ../polyvec.c ../randombytes.c ../reduce.c ../symmetric-shake.c ../verify.c
KYBER_MAIN = cv32a6_imac_sv0_kyber.c

# Kyber CFLAGS: Optimized for crypto, uses local linker script, drops standard start files
KYBER_CFLAGS = $(ARCH_FLAGS) -O2 -fno-math-errno -ffp-contract=off -mstrict-align -malign-data=xlen \
               -ffunction-sections -fdata-sections -Wl,--gc-sections \
               -nostartfiles -T ./link.ld -I../

# ==============================================================================
# Build Rules
# ==============================================================================
all: $(TARGET).elf

# 1. Compile BSP Object Files
%.o : %.c
	$(RISCV_GCC) $(BSP_CFLAGS) -c $< -o $@
    
%.o : %.S
	$(RISCV_GCC) $(BSP_CFLAGS) -c $< -o $@

# 2. Archive BSP into a static library
$(LIBCV_VERIF): $(BSP_OBJ) 
	$(RISCV_AR) rcs $@ $(BSP_OBJ)

# 3. Compile Kyber and link against the BSP library
$(TARGET).elf: $(LIBCV_VERIF) $(KYBER_MAIN) $(KYBER_SRC)
	$(RISCV_GCC) $(KYBER_MAIN) $(KYBER_SRC) $(KYBER_CFLAGS) -L. -lcv-verif -o $@

# (Optional) Generate a flat binary if your specific testbench requires it over ELF
$(TARGET).bin: $(TARGET).elf
	$(RISCV_OBJCOPY) -O binary -j .init -j .text -j .rodata -j .sdata -j .data $< $@

clean:
	rm -f *.o *.a *.elf *.bin

vars:
	@echo "Build Variables:"
	@echo "   RISCV_GCC        = $(RISCV_GCC)"
	@echo "   ARCH_FLAGS       = $(ARCH_FLAGS)"