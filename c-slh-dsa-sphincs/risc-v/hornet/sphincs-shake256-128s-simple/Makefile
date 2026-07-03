CC32=riscv32-unknown-elf
CC64=riscv64-unknown-elf
RVCCFLAGS=-march=rv32im -mabi=ilp32 -ffp-contract=off -mstrict-align -malign-data=xlen -O2 -fno-math-errno -T linksc.ld -nostartfiles -ffunction-sections -fdata-sections -Wl,--gc-sections -o $(TARGET).elf
TARGET=sphincs

HASH = shake256
THASH = simple

SOURCES =          address.c rng.c wots.c utils.c fors.c sign.c hash_$(HASH).c thash_$(HASH)_$(THASH).c aes.c test/spx.c
HEADERS = params.h address.h rng.h wots.h utils.h fors.h api.h  hash.h thash.h aes.h

ifeq ($(HASH),shake256)
	SOURCES += fips202.c
	HEADERS += fips202.h
endif
ifeq ($(HASH),haraka)
	SOURCES += haraka.c
	HEADERS += haraka.h
endif
ifeq ($(HASH),sha256)
	SOURCES += sha256.c
	HEADERS += sha256.h
endif

riscv_spx:
	$(CC32)-gcc $(HEADERS) $(SOURCES) rom_gen/crt0.s $(RVCCFLAGS)
	$(CC32)-objcopy -O binary -j .init -j .text -j .rodata -j .sdata $(TARGET).elf $(TARGET).bin
	rom_gen/rom_generator $(TARGET).bin
	cp $(TARGET).data memory_init.mem

riscv_spx_multilib:
	$(CC64)-gcc $(HEADERS) $(SOURCES) rom_gen/crt0.s $(RVCCFLAGS)
	$(CC64)-objcopy -O binary -j .init -j .text -j .rodata -j .sdata $(TARGET).elf $(TARGET).bin
	rom_gen/rom_generator $(TARGET).bin
	cp $(TARGET).data memory_init.mem

clean:
	rm -f *.elf *.bin *.data *.gch memory_init.mem
