# Build rules for the PQC harnesses on the HORNET RV32IMF SoC
# (https://github.com/GSTL-ITU/HORNET-RV32IMF). A harness Makefile:
#
#     PROG = dilithium2
#     SRCS = hornet_dilithium.c ../sign.c ...
#     include ../../../../hornet/hornet.mk
#
# make            ELF and IMEM/DMEM images (HORNET's sw/common.mk)
# make sim        run on the SoC testbench (Verilator); UART log in build/<prog>.uart.log
# make host       the same harness built natively: reference digests in build/host.log
# make check      sim and host; the digests must match and the harness must PASS
# make size       section sizes
#
# Variables: HORNET_ROOT (HORNET checkout, default: next to this repository),
# OPT (-O3), RUNS (operations per scheme, each with fresh randomness), IMEM_SIZE,
# DMEM_SIZE, STACK_SIZE, TIMEOUT (simulation cycles).
# The compiler and Verilator run in HORNET's hornet-dev Docker image.

PQC_ROOT    := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/..)
HORNET_ROOT ?= $(abspath $(PQC_ROOT)/../HORNET-RV32IMF)
SRC_ROOT    := $(PQC_ROOT)

ifeq ($(wildcard $(HORNET_ROOT)/sw/common.mk),)
$(error HORNET not found at $(HORNET_ROOT); clone it there or set HORNET_ROOT)
endif

OPT        ?= -O3
RUNS       ?= 1
IMEM_SIZE  ?= 0x40000
DMEM_SIZE  ?= 0x40000
STACK_SIZE ?= 0x10000
TIMEOUT    ?= 100000000000
SIM_ARGS   ?= +timeout=$(TIMEOUT)
CFLAGS     += -I$(PQC_ROOT)/hornet -DRUNS=$(RUNS) $(PQC_CFLAGS)

include $(HORNET_ROOT)/sw/common.mk

HOST_CC     ?= gcc
HOST_CFLAGS ?= -O2 -Wall
HOST_BIN     = $(OUT)/host/$(PROG)

.PHONY: host check

$(HOST_BIN): $(SRCS) $(PQC_ROOT)/hornet/bench.h Makefile
	@mkdir -p $(@D)
	$(RUN) $(HOST_CC) $(HOST_CFLAGS) -I$(PQC_ROOT)/hornet -DRUNS=$(RUNS) $(PQC_CFLAGS) $(SRCS) -o $@

host: $(HOST_BIN)
	$(RUN) $(HOST_BIN) | tee $(OUT)/host.log

check: host sim
	@grep '^digest' $(OUT)/host.log > $(OUT)/digests.host
	@tr -d '\r' < $(OUT)/$(PROG).uart.log | grep '^digest' > $(OUT)/digests.hornet
	@test -s $(OUT)/digests.host
	@if diff $(OUT)/digests.host $(OUT)/digests.hornet > /dev/null && grep -q '^result PASS' $(OUT)/$(PROG).uart.log; then \
	    echo "CHECK PASS $(PROG): $$(wc -l < $(OUT)/digests.host) digests match the host build"; \
	else \
	    echo "CHECK FAIL $(PROG)"; diff $(OUT)/digests.host $(OUT)/digests.hornet; exit 1; \
	fi
