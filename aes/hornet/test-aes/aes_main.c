// AES known-answer tests (ECB, CBC, CTR, CCM) on HORNET, plus the cycle count
// of the AES-128 key schedule and of one block encryption
#include <stdint.h>
#include "bench.h"
#include "../../ref/aes.h"

static const BYTE key[16] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
                              0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
static const BYTE pt[16]  = { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
                              0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a };
static WORD schedule[60];
static BYTE ct[16];

int main(void)
{
    bench_start("AES");
    bench_check(aes_ecb_test(), "ECB", 0);
    bench_check(aes_cbc_test(), "CBC", 0);
    bench_check(aes_ctr_test(), "CTR", 0);
    bench_check(aes_ccm_test(), "CCM", 0);
    for (int run = 0; run < RUNS; run++) {
        BENCH("keysetup128", run, aes_key_setup(key, schedule, 128));
        BENCH("encrypt128", run, aes_encrypt(pt, ct, schedule, 128));
        bench_digest("ct", run, ct, sizeof ct);
    }
    return bench_end();
}
