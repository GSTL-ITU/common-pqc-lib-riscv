# FN-DSA on HORNET: make [LOGN=9|10] [sim|host|check]
LOGN ?= 9
PROG = falcon$(LOGN)
OUT  = build/$(PROG)
SRCS = hornet_falcon.c ../src/codec.c ../src/common.c ../src/falcon.c ../src/fft.c ../src/fpr.c \
       ../src/keygen.c ../src/rng.c ../src/shake.c ../src/sign.c ../src/vrfy.c
PQC_CFLAGS = -DLOGN=$(LOGN)
IMEM_SIZE ?= 0x20000
DMEM_SIZE ?= 0x20000
STACK_SIZE ?= 0x8000
include ../../../../hornet/hornet.mk
