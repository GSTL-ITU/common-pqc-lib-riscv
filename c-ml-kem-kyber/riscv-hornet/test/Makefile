# ML-KEM on HORNET: make [KYBER_K=2|3|4] [sim|host|check]
KYBER_K ?= 3
PROG = kyber$(KYBER_K)
OUT  = build/$(PROG)
SRCS = hornet_kyber.c ../cbd.c ../fips202.c ../indcpa.c ../kem.c ../ntt.c ../poly.c ../polyvec.c \
       ../randombytes.c ../reduce.c ../symmetric-shake.c ../verify.c
PQC_CFLAGS = -DKYBER_K=$(KYBER_K)
IMEM_SIZE ?= 0x10000
DMEM_SIZE ?= 0x10000
STACK_SIZE ?= 0x8000
include ../../../hornet/hornet.mk
