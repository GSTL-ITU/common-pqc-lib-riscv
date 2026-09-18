CC32=riscv32-unknown-elf
TARGET=aes_sca

CCFLAGS=-march=rv32imf -mabi=ilp32f -ffp-contract=off -mstrict-align -malign-data=xlen -O2 -fno-math-errno -T ./linksc.ld -nostartfiles -ffunction-sections -fdata-sections -Wl,--gc-sections -I../../ref

# Reference AES implementation
AES_SRC = ../../ref/aes.c

# Hornet drivers
DRV_SRC = ../drivers/uart.c ../drivers/irq.c

build:
	$(CC32)-gcc aes_sca_main.c $(AES_SRC) $(DRV_SRC) ../rom_gen/crt0.s $(CCFLAGS) -o $(TARGET).elf
	$(CC32)-objcopy -O binary -j .init -j .text -j .rodata -j .sdata $(TARGET).elf $(TARGET).bin
	../rom_gen/rom_generator $(TARGET).bin
	cp $(TARGET).data memory_init.mem

inspect:
	$(CC32)-size $(TARGET).elf
	$(CC32)-objdump -d $(TARGET).elf > $(TARGET).lst

clean:
	rm -f *.elf *.bin *.data *.lst memory_init.mem