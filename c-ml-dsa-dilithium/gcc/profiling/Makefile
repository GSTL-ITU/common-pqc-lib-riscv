CC ?= /usr/bin/cc

GPROF_FLAGS = -pg -g

REF_CFLAGS += -Wall -Wextra -Wpedantic -Wmissing-prototypes -Wredundant-decls \
  -Wshadow -Wvla -Wpointer-arith -O3 -I../ref $(GPROF_FLAGS)
REF_SOURCES = ../ref/sign.c ../ref/packing.c ../ref/polyvec.c ../ref/poly.c ../ref/ntt.c ../ref/reduce.c ../ref/rounding.c
REF_HEADERS = ../ref/config.h ../ref/params.h ../ref/api.h ../ref/sign.h ../ref/packing.h ../ref/polyvec.h ../ref/poly.h ../ref/ntt.h \
  ../ref/reduce.h ../ref/rounding.h ../ref/symmetric.h ../ref/randombytes.h
REF_KECCAK_SOURCES = $(REF_SOURCES) ../ref/fips202.c ../ref/symmetric-shake.c
REF_KECCAK_HEADERS = $(REF_HEADERS) ../ref/fips202.h

AVX2_CFLAGS += -Wall -Wextra -Wpedantic -Wmissing-prototypes -Wredundant-decls \
  -Wshadow -Wpointer-arith -mavx2 -mpopcnt \
  -march=native -mtune=native -O3 -I../avx2 $(GPROF_FLAGS)
AVX2_SOURCES = ../avx2/sign.c ../avx2/packing.c ../avx2/polyvec.c ../avx2/poly.c ../avx2/ntt.S ../avx2/invntt.S ../avx2/pointwise.S \
  ../avx2/shuffle.S ../avx2/consts.c ../avx2/rejsample.c ../avx2/rounding.c
AVX2_HEADERS = ../avx2/align.h ../avx2/config.h ../avx2/params.h ../avx2/api.h ../avx2/sign.h ../avx2/packing.h ../avx2/polyvec.h ../avx2/poly.h ../avx2/ntt.h \
  ../avx2/consts.h ../avx2/shuffle.inc ../avx2/rejsample.h ../avx2/rounding.h ../avx2/symmetric.h ../avx2/randombytes.h
AVX2_KECCAK_SOURCES = $(AVX2_SOURCES) ../avx2/fips202.c ../avx2/fips202x4.c ../avx2/f1600x4.S ../avx2/symmetric-shake.c
AVX2_KECCAK_HEADERS = $(AVX2_HEADERS) ../avx2/fips202.h ../avx2/fips202x4.h


.PHONY: profile clean

profile:
	$(MAKE) ref_test_dilithium2
	./ref_test_dilithium2
	gprof ref_test_dilithium2 gmon.out > prof_ref_d2.txt
	rm -f gmon.out
	$(MAKE) ref_test_dilithium3
	./ref_test_dilithium3
	gprof ref_test_dilithium3 gmon.out > prof_ref_d3.txt
	rm -f gmon.out
	$(MAKE) ref_test_dilithium5
	./ref_test_dilithium5
	gprof ref_test_dilithium5 gmon.out > prof_ref_d5.txt
	rm -f gmon.out
	$(MAKE) avx2_test_dilithium2
	./avx2_test_dilithium2
	gprof avx2_test_dilithium2 gmon.out > prof_avx2_d2.txt
	rm -f gmon.out
	$(MAKE) avx2_test_dilithium3
	./avx2_test_dilithium3
	gprof avx2_test_dilithium3 gmon.out > prof_avx2_d3.txt
	rm -f gmon.out
	$(MAKE) avx2_test_dilithium5
	./avx2_test_dilithium5
	gprof avx2_test_dilithium5 gmon.out > prof_avx2_d5.txt
	rm -f gmon.out


ref_test_dilithium2: ../ref/test/test_dilithium.c ../ref/randombytes.c $(REF_KECCAK_SOURCES) \
  $(REF_KECCAK_HEADERS)
	$(CC) $(REF_CFLAGS) -DDILITHIUM_MODE=2 \
	  -o $@ $< ../ref/randombytes.c $(REF_KECCAK_SOURCES)

ref_test_dilithium3: ../ref/test/test_dilithium.c ../ref/randombytes.c $(REF_KECCAK_SOURCES) \
  $(REF_KECCAK_HEADERS)
	$(CC) $(REF_CFLAGS) -DDILITHIUM_MODE=3 \
	  -o $@ $< ../ref/randombytes.c $(REF_KECCAK_SOURCES)

ref_test_dilithium5: ../ref/test/test_dilithium.c ../ref/randombytes.c $(REF_KECCAK_SOURCES) \
  $(REF_KECCAK_HEADERS)
	$(CC) $(REF_CFLAGS) -DDILITHIUM_MODE=5 \
	  -o $@ $< ../ref/randombytes.c $(REF_KECCAK_SOURCES)


avx2_test_dilithium2: ../avx2/test/test_dilithium.c ../avx2/randombytes.c $(AVX2_KECCAK_SOURCES) \
  $(AVX2_KECCAK_HEADERS)
	$(CC) $(AVX2_CFLAGS) -DDILITHIUM_MODE=2 \
	  -o $@ $< ../avx2/randombytes.c $(AVX2_KECCAK_SOURCES)

avx2_test_dilithium3: ../avx2/test/test_dilithium.c ../avx2/randombytes.c $(AVX2_KECCAK_SOURCES) \
  $(AVX2_KECCAK_HEADERS)
	$(CC) $(AVX2_CFLAGS) -DDILITHIUM_MODE=3 \
	  -o $@ $< ../avx2/randombytes.c $(AVX2_KECCAK_SOURCES)

avx2_test_dilithium5: ../avx2/test/test_dilithium.c ../avx2/randombytes.c $(AVX2_KECCAK_SOURCES) \
  $(AVX2_KECCAK_HEADERS)
	$(CC) $(AVX2_CFLAGS) -DDILITHIUM_MODE=5 \
	  -o $@ $< ../avx2/randombytes.c $(AVX2_KECCAK_SOURCES)


clean:
	rm -f ref_test_dilithium2
	rm -f ref_test_dilithium3
	rm -f ref_test_dilithium5
	rm -f avx2_test_dilithium2
	rm -f avx2_test_dilithium3
	rm -f avx2_test_dilithium5

