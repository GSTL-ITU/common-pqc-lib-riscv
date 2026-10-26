# AES-128 EM side-channel target on HORNET (UART protocol in ../../README.md)
#   make            images; make fpga-build in HORNET puts them in a bitstream
#   make sim        protocol test in simulation (sca_sim.bin: boot byte, ping, key, encryptions)
PROG = aes_sca
SRCS = aes_sca_main.c ../../ref/aes.c
PQC_CFLAGS = -I../../ref
OPT = -O2
IMEM_SIZE ?= 0x10000
DMEM_SIZE ?= 0x10000
STACK_SIZE ?= 0x1000
SIM_ARGS = +uart_in=$(abspath sca_sim.bin) +uart_in_wait=1 +uart_in_block=17 +uart_hex +timeout=400000
include ../../../hornet/hornet.mk
