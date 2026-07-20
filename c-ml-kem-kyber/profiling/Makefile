CC ?= /usr/bin/cc

GPROF_FLAGS = -pg -g

REF_CFLAGS += -Wall -Wextra -Wpedantic -Wmissing-prototypes -Wredundant-decls \
  -Wshadow -Wpointer-arith -O3 -z noexecstack -I ../ref $(GPROF_FLAGS)
REF_SOURCES = ../ref/kem.c ../ref/indcpa.c ../ref/polyvec.c ../ref/poly.c ../ref/ntt.c ../ref/cbd.c ../ref/reduce.c ../ref/verify.c
REF_SOURCESKECCAK = $(REF_SOURCES) ../ref/fips202.c ../ref/symmetric-shake.c
REF_HEADERS = ../ref/params.h ../ref/kem.h ../ref/indcpa.h ../ref/polyvec.h ../ref/poly.h ../ref/ntt.h ../ref/cbd.h ../ref/reduce.c ../ref/verify.h ../ref/symmetric.h
REF_HEADERSKECCAK = $(REF_HEADERS) ../ref/fips202.h

AVX2_CFLAGS += -Wall -Wextra -Wpedantic -Wmissing-prototypes -Wredundant-decls \
  -Wshadow -Wpointer-arith -mavx2 -mbmi2 -mpopcnt \
  -march=native -mtune=native -O3 -z noexecstack -I ../avx2 -I ../avx2/keccak4x $(GPROF_FLAGS)
AVX2_SOURCES = ../avx2/kem.c ../avx2/indcpa.c ../avx2/polyvec.c ../avx2/poly.c ../avx2/fq.S ../avx2/shuffle.S ../avx2/ntt.S ../avx2/invntt.S \
  ../avx2/basemul.S ../avx2/consts.c ../avx2/rejsample.c ../avx2/cbd.c ../avx2/verify.c
AVX2_SOURCESKECCAK   = $(AVX2_SOURCES) ../avx2/fips202.c ../avx2/fips202x4.c ../avx2/symmetric-shake.c \
  avx2_keccak4x_KeccakP-1600-times4-SIMD256.o
AVX2_HEADERS = ../avx2/params.h ../avx2/align.h ../avx2/kem.h ../avx2/indcpa.h ../avx2/polyvec.h ../avx2/poly.h ../avx2/reduce.h ../avx2/fq.inc ../avx2/shuffle.inc \
  ../avx2/ntt.h ../avx2/consts.h ../avx2/rejsample.h ../avx2/cbd.h ../avx2/verify.h ../avx2/symmetric.h ../avx2/randombytes.h
AVX2_HEADERSKECCAK   = $(AVX2_HEADERS) ../avx2/fips202.h ../avx2/fips202x4.h


.PHONY: profile clean

profile:
	$(MAKE) ref_test_kyber512
	./ref_test_kyber512
	gprof ref_test_kyber512 gmon.out > prof_ref_k512.txt
	rm -f gmon.out
	$(MAKE) ref_test_kyber768
	./ref_test_kyber768
	gprof ref_test_kyber768 gmon.out > prof_ref_k768.txt
	rm -f gmon.out
	$(MAKE) ref_test_kyber1024
	./ref_test_kyber1024
	gprof ref_test_kyber1024 gmon.out > prof_ref_k1024.txt
	rm -f gmon.out
	$(MAKE) avx2_keccak4x_KeccakP-1600-times4-SIMD256.o
	$(MAKE) avx2_test_kyber512
	./avx2_test_kyber512
	gprof avx2_test_kyber512 gmon.out > prof_avx2_k512.txt
	rm -f gmon.out
	$(MAKE) avx2_test_kyber768
	./avx2_test_kyber768
	gprof avx2_test_kyber768 gmon.out > prof_avx2_k768.txt
	rm -f gmon.out
	$(MAKE) avx2_test_kyber1024
	./avx2_test_kyber1024
	gprof avx2_test_kyber1024 gmon.out > prof_avx2_k1024.txt
	rm -f gmon.out


ref_test_kyber512: $(REF_SOURCESKECCAK) $(REF_HEADERSKECCAK) ../ref/test/test_kyber.c ../ref/randombytes.c
	$(CC) $(REF_CFLAGS) -DKYBER_K=2 $(REF_SOURCESKECCAK) ../ref/randombytes.c ../ref/test/test_kyber.c -o $@

ref_test_kyber768: $(REF_SOURCESKECCAK) $(REF_HEADERSKECCAK) ../ref/test/test_kyber.c ../ref/randombytes.c
	$(CC) $(REF_CFLAGS) -DKYBER_K=3 $(REF_SOURCESKECCAK) ../ref/randombytes.c ../ref/test/test_kyber.c -o $@

ref_test_kyber1024: $(REF_SOURCESKECCAK) $(REF_HEADERSKECCAK) ../ref/test/test_kyber.c ../ref/randombytes.c
	$(CC) $(REF_CFLAGS) -DKYBER_K=4 $(REF_SOURCESKECCAK) ../ref/randombytes.c ../ref/test/test_kyber.c -o $@


avx2_test_kyber512: $(AVX2_SOURCESKECCAK) $(AVX2_HEADERSKECCAK) ../avx2/test/test_kyber.c ../avx2/randombytes.c
	$(CC) $(AVX2_CFLAGS) -DKYBER_K=2 $(AVX2_SOURCESKECCAK) ../avx2/randombytes.c ../avx2/test/test_kyber.c -o $@

avx2_test_kyber768: $(AVX2_SOURCESKECCAK) $(AVX2_HEADERSKECCAK) ../avx2/test/test_kyber.c ../avx2/randombytes.c
	$(CC) $(AVX2_CFLAGS) -DKYBER_K=3 $(AVX2_SOURCESKECCAK) ../avx2/randombytes.c ../avx2/test/test_kyber.c -o $@

avx2_test_kyber1024: $(AVX2_SOURCESKECCAK) $(AVX2_HEADERSKECCAK) ../avx2/test/test_kyber.c ../avx2/randombytes.c
	$(CC) $(AVX2_CFLAGS) -DKYBER_K=4 $(AVX2_SOURCESKECCAK) ../avx2/randombytes.c ../avx2/test/test_kyber.c -o $@


avx2_keccak4x_KeccakP-1600-times4-SIMD256.o: \
  ../avx2/keccak4x/KeccakP-1600-times4-SIMD256.c \
  ../avx2/keccak4x/KeccakP-1600-times4-SnP.h \
  ../avx2/keccak4x/KeccakP-1600-unrolling.macros \
  ../avx2/keccak4x/KeccakP-SIMD256-config.h \
  ../avx2/keccak4x/KeccakP-align.h \
  ../avx2/keccak4x/KeccakP-brg_endian.h
	$(CC) $(AVX2_CFLAGS) -c $< -o $@


clean:
	rm -f ref_test_kyber512
	rm -f ref_test_kyber768
	rm -f ref_test_kyber1024
	rm -f avx2_test_kyber512
	rm -f avx2_test_kyber768
	rm -f avx2_test_kyber1024
	rm -f *.o
