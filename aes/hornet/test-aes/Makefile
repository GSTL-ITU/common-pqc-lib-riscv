CC32=riscv32-unknown-elf
TARGET=sim_aes

# Same core flags as the Falcon test-1 Makefile. -I../../ref finds aes.h.
CCFLAGS=-march=rv32imf -mabi=ilp32f -ffp-contract=off -mstrict-align -malign-data=xlen -O2 -fno-math-errno -T ./linksc.ld -nostartfiles -ffunction-sections -fdata-sections -Wl,--gc-sections -I../../ref 

# AES sources: the patched reference implementation + the stripped KAT harness
AES_SRC = ../../ref/aes.c aes_test_bare.c

build:
	$(CC32)-gcc aes_main.c $(AES_SRC) ../rom_gen/crt0.s $(CCFLAGS) -o $(TARGET).elf
	$(CC32)-objcopy -O binary -j .init -j .text -j .rodata -j .sdata $(TARGET).elf $(TARGET).bin
	../rom_gen/rom_generator $(TARGET).bin
	cp $(TARGET).data memory_init_tb.mem

clean:
	rm -f *.elf *.bin *.data memory_init_tb.mem