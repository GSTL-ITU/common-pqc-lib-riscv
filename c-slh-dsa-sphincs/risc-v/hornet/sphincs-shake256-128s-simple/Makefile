# SLH-DSA (SPHINCS+-SHAKE-128s-simple) on HORNET: make [sim|host|check]
# The harness uses the SHAKE256 PRNG of randombytes.c; rng.c and aes.c (NIST
# DRBG) are only for PQCgenKAT_sign.c on a PC.
PROG = sphincs-shake-128s
OUT  = build
SRCS = test/hornet_sphincs.c address.c randombytes.c wots.c utils.c fors.c sign.c hash_shake256.c \
       thash_shake256_simple.c fips202.c
IMEM_SIZE ?= 0x10000
DMEM_SIZE ?= 0x10000
STACK_SIZE ?= 0x8000
include ../../../../hornet/hornet.mk
