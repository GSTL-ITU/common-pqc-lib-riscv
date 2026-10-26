# AES known-answer tests on HORNET: make [sim|host|check]
PROG = aes-test
SRCS = aes_main.c aes_test_bare.c ../../ref/aes.c
PQC_CFLAGS = -I../../ref
IMEM_SIZE ?= 0x10000
DMEM_SIZE ?= 0x10000
STACK_SIZE ?= 0x2000
include ../../../hornet/hornet.mk
