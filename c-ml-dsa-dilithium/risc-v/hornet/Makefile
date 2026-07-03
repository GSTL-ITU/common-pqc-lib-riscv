CC ?= /usr/bin/cc
CFLAGS += -Wall -Wextra -Wpedantic -Wmissing-prototypes -Wredundant-decls \
  -Wshadow -Wvla -Wpointer-arith -O3 -fomit-frame-pointer
NISTFLAGS += -Wno-unused-result -O3 -fomit-frame-pointer
SOURCES = sign.c packing.c polyvec.c poly.c ntt.c reduce.c rounding.c
HEADERS = config.h params.h api.h sign.h packing.h polyvec.h poly.h ntt.h \
  reduce.h rounding.h symmetric.h randombytes.h
KECCAK_SOURCES = $(SOURCES) fips202.c symmetric-shake.c
KECCAK_HEADERS = $(HEADERS) fips202.h

CC32=riscv32-unknown-elf
CC64=riscv64-unknown-elf
RVCCFLAGS=-march=rv32im -mabi=ilp32 -ffp-contract=off -mstrict-align -malign-data=xlen -O2 -fno-math-errno -T linksc.ld -nostartfiles -ffunction-sections -fdata-sections -Wl,--gc-sections -o $(TARGET).elf
TARGET=dilithium

riscv_test_dilithium2:
	$(CC32)-gcc test/test_dilithium.c randombytes.c $(KECCAK_SOURCES) $(KECCAK_HEADERS) rom_gen/crt0.s $(RVCCFLAGS) -DDILITHIUM_MODE=2
	$(CC32)-objcopy -O binary -j .init -j .text -j .rodata -j .sdata $(TARGET).elf $(TARGET).bin
	rom_gen/rom_generator $(TARGET).bin
	cp $(TARGET).data memory_init.mem

riscv_test_dilithium2_multilib: 
	$(CC64)-gcc test/test_dilithium.c randombytes.c $(KECCAK_SOURCES) $(KECCAK_HEADERS) rom_gen/crt0.s $(RVCCFLAGS) -DDILITHIUM_MODE=2
	$(CC64)-objcopy -O binary -j .init -j .text -j .rodata -j .sdata $(TARGET).elf $(TARGET).bin
	rom_gen/rom_generator $(TARGET).bin
	cp $(TARGET).data memory_init.mem

clean:
	rm -f *.elf *.bin *.data memory_init.mem
