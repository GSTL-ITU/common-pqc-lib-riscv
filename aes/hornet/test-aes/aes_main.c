#include <stdint.h>
#include "../../ref/aes.h"

// Hornet Debug Interface
#define DEBUG_IF_ADDR 0x10008010
#define DEBUG_REG     ((volatile char *)DEBUG_IF_ADDR)

int main(void)
{
	int ecb_pass, cbc_pass, ctr_pass, ccm_pass, all_pass;

	*DEBUG_REG = 'S'; // 'S' for Start

	// Run each mode separately and report as we go, so a hang/crash in
	// one mode still tells you which one via the last byte written.
	ecb_pass = aes_ecb_test();
	*DEBUG_REG = ecb_pass ? '1' : 'a'; // '1'=ECB pass, 'a'=ECB fail

	cbc_pass = aes_cbc_test();
	*DEBUG_REG = cbc_pass ? '2' : 'b'; // '2'=CBC pass, 'b'=CBC fail

	ctr_pass = aes_ctr_test();
	*DEBUG_REG = ctr_pass ? '3' : 'c'; // '3'=CTR pass, 'c'=CTR fail

	ccm_pass = aes_ccm_test();
	*DEBUG_REG = ccm_pass ? '4' : 'd'; // '4'=CCM pass, 'd'=CCM fail

	all_pass = ecb_pass && cbc_pass && ctr_pass && ccm_pass;

	*DEBUG_REG = all_pass ? 'P' : 'F'; // 'P' for Pass, 'F' for Fail

	while (1) {
		__asm__ volatile ("nop");
	}

	return 0;
}