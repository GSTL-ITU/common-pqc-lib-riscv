# ML-DSA on HORNET: make [DILITHIUM_MODE=2|3|5] [sim|host|check]
DILITHIUM_MODE ?= 2
PROG = dilithium$(DILITHIUM_MODE)
OUT  = build/$(PROG)
SRCS = hornet_dilithium.c ../sign.c ../packing.c ../polyvec.c ../poly.c ../ntt.c ../reduce.c \
       ../rounding.c ../randombytes.c ../fips202.c ../symmetric-shake.c
PQC_CFLAGS = -DDILITHIUM_MODE=$(DILITHIUM_MODE)
IMEM_SIZE ?= 0x10000
DMEM_SIZE ?= 0x20000
STACK_SIZE ?= 0x10000
include ../../../../hornet/hornet.mk
