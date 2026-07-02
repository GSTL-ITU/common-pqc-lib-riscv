CC32=riscv32-unknown-elf
TARGET=sim_falcon

# Adjusted from your provided Makefiles. Added -I./src to find headers.
CCFLAGS=-march=rv32imf -mabi=ilp32f -ffp-contract=off -mstrict-align -malign-data=xlen -O2 -fno-math-errno -T ./linksc.ld -nostartfiles -ffunction-sections -fdata-sections -Wl,--gc-sections -I./src

# All Falcon submodules
FALCON_SRC = ../src/codec.c ../src/common.c ../src/falcon.c ../src/fft.c ../src/fpr.c ../src/keygen.c ../src/rng.c ../src/shake.c ../src/sign.c ../src/vrfy.c

build:
	$(CC32)-gcc hornet_keygen.c $(FALCON_SRC) ../rom_gen/crt0.s $(CCFLAGS) -o $(TARGET).elf
	$(CC32)-objcopy -O binary -j .init -j .text -j .rodata -j .sdata $(TARGET).elf $(TARGET).bin
	../rom_gen/rom_generator $(TARGET).bin
	cp $(TARGET).data memory_init_tb.mem

clean:
	rm -f *.elf *.bin *.data memory_init_tb.mem