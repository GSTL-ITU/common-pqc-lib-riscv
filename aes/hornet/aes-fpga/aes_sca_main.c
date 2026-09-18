/*********************************************************************
* DISCLAIMER: This code is combination of various AES and Hornet implementations. The combining has done by using LLMs.
* Filename:   aes_sca_main.c
* Author:     Yusuf Tekin
* Date:       18-09-2026, 1:45 PM
* Location:   aes/hornet/aes-fpga/
* Target:     Hornet RV32IMF on Nexys 4DDR FPGA
* Details:    AES-128 EM side-channel acquisition firmware.
*             Encryption only.
*
*             Host -> target : command byte + payload over UART
*             Target -> host : response bytes over UART
*
*             The GPIO trigger (fpga_top: gpio_trigger_o, slave 5)
*             goes HIGH immediately before the aes_encrypt() call and
*             LOW immediately after it. Nothing else happens inside
*             that window: no UART traffic, no interrupts, no key
*             schedule expansion.
*
*             Wiring: gpio_trigger_o -> scope AUX In (rising edge)
*                     EM probe       -> scope CH1
*
* ---------------------------------------------------------------------
* PROTOCOL (lockstep: the host must read the full response before
*           sending the next command, because interrupts are disabled
*           during the crypto and the reply - bytes arriving in that
*           window are lost)
* ---------------------------------------------------------------------
*   0x76 'v'                       -> 'H'            (ping / handshake)
*   0x6B 'k' + 16 key bytes        -> 'K'            (load key, no trigger)
*   0x70 'p' + 16 plaintext bytes  -> 16 ciphertext  (TRIGGERED encrypt)
*   anything else                  -> 'E'            (resync byte)
*
*   At boot the target sends 'R' if the built-in known-answer test
*   passed, or 'X' if it failed. Wait for that byte before capturing.
*********************************************************************/

#include <stdint.h>
#include <string.h>

#include "../../ref/aes.h"
#include "../drivers/uart.h"
#include "../drivers/irq.h"
#include "../drivers/gpio.h"

/*************************** CONFIGURATION **************************/

#define UART_BASE_ADDR      0x10008010u
#define UART_RX_ADDR        (UART_BASE_ADDR + UART_RX_ADDR_OFFSET)

#define AES_KEYSIZE         128
#define AES_BLK             AES_BLOCK_SIZE          /* 16 */

/* Dead time inserted between the trigger edge and the AES call (and
 * between the AES call and the falling edge). Gives the scope a clean,
 * quiet baseline on both sides of the region of interest and keeps the
 * trigger edge itself out of the window you correlate over.
 * ~1 cycle per nop @ 25 MHz core clock => 32 nops ~ 1.3 us.
 * Raise these if you see the GPIO edge coupling into CH1.            */
#define TRIG_PRE_NOPS       32
#define TRIG_POST_NOPS      32

/* Commands */
#define CMD_PING            0x76u   /* 'v' */
#define CMD_KEY             0x6Bu   /* 'k' */
#define CMD_ENC             0x70u   /* 'p' */

/* Responses */
#define RESP_READY          0x52u   /* 'R' */
#define RESP_SELFTEST_FAIL  0x58u   /* 'X' */
#define RESP_PONG           0x48u   /* 'H' */
#define RESP_KEY_OK         0x4Bu   /* 'K' */
#define RESP_ERR            0x45u   /* 'E' */

/****************************** MACROS ******************************/

/* Direct register writes rather than gpio_set_trigger(): one store,
 * no call/return, no branch on the argument => constant, minimal and
 * repeatable latency between the edge and the first AES instruction. */
#define TRIG_HI()   do { *GPIO_REG = 1u; } while (0)
#define TRIG_LO()   do { *GPIO_REG = 0u; } while (0)

/* Compiler barrier: stops -O2 from hoisting any part of the AES work
 * across the trigger stores. */
#define BARRIER()   __asm__ volatile ("" ::: "memory")

static inline void nop_pad(int n)
{
    while (n--)
        __asm__ volatile ("nop");
}

/***************************** GLOBALS ******************************/

uart uart0;

static WORD key_schedule[60];

/* Default key: FIPS-197 / SP800-38A 128-bit test key.
 * Overwrite at runtime with the 'k' command.                        */
static BYTE aes_key[AES_BLK] = {
    0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
};

/* --- UART RX state machine, written by the ISR only --------------- */
static volatile uint8_t  rx_cmd;
static volatile uint8_t  rx_buf[AES_BLK];
static volatile uint32_t rx_count;
static volatile uint32_t rx_expect;
static volatile uint8_t  rx_have_cmd;
static volatile uint8_t  frame_ready;

/* --- Working buffers, filled outside the trigger window ----------- */
static BYTE in_blk[AES_BLK];
static BYTE out_blk[AES_BLK];

/*********************** FUNCTION DEFINITIONS ***********************/

static uint32_t payload_len_for(uint8_t cmd)
{
    switch (cmd) {
        case CMD_KEY:
        case CMD_ENC:  return AES_BLK;
        case CMD_PING: return 0u;
        default:       return 0u;   /* unknown -> 0 payload, main replies 'E' */
    }
}

static void rx_reset(void)
{
    rx_count    = 0u;
    rx_expect   = 0u;
    rx_have_cmd = 0u;
    frame_ready = 0u;
}

static void send_block(const BYTE *b, int len)
{
    int i;
    for (i = 0; i < len; i++)
        uart_transmit_byte(&uart0, b[i]);
}

/*-------------------------------------------------------------------
 * The measured region. Keep this function free of anything that is
 * not the cipher itself.
 *------------------------------------------------------------------*/
static void triggered_encrypt(void)
{
    BARRIER();
    TRIG_HI();
    BARRIER();
    nop_pad(TRIG_PRE_NOPS);
    BARRIER();

    aes_encrypt(in_blk, out_blk, key_schedule, AES_KEYSIZE);

    BARRIER();
    nop_pad(TRIG_POST_NOPS);
    BARRIER();
    TRIG_LO();
    BARRIER();
}

/*-------------------------------------------------------------------
 * Boot known-answer test: FIPS-197 vector for the default key.
 * Plaintext  6bc1bee22e409f96e93d7e117393172a
 * Ciphertext 3ad77bb40d7a3660a89ecaf32466ef97
 *------------------------------------------------------------------*/
static int self_test(void)
{
    static const BYTE pt[AES_BLK] = {
        0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96,
        0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a
    };
    static const BYTE ct[AES_BLK] = {
        0x3a, 0xd7, 0x7b, 0xb4, 0x0d, 0x7a, 0x36, 0x60,
        0xa8, 0x9e, 0xca, 0xf3, 0x24, 0x66, 0xef, 0x97
    };
    BYTE tmp[AES_BLK];

    aes_encrypt(pt, tmp, key_schedule, AES_KEYSIZE);

    return !memcmp(tmp, ct, AES_BLK);
}

/*============================== MAIN ==============================*/

int main(void)
{
    SET_MTVEC_VECTOR_MODE();

    uart_init(&uart0, (uint32_t *) UART_BASE_ADDR);

    TRIG_LO();                       /* trigger known-low out of reset */

    rx_reset();

    /* Key schedule is expanded here, never inside the trigger window. */
    aes_key_setup(aes_key, key_schedule, AES_KEYSIZE);

    uart_transmit_byte(&uart0, self_test() ? RESP_READY : RESP_SELFTEST_FAIL);

    while (1) {
        /* ---- Receive phase: interrupts on ---- */
        ENABLE_FAST_IRQ(0);
        ENABLE_GLOBAL_IRQ();

        while (!frame_ready) {
            __asm__ volatile ("nop");
        }

        /* ---- Process phase: interrupts off ----
         * No ISR can fire during the crypto, so trace-to-trace jitter
         * is limited to the cipher's own data-dependent behaviour.   */
        DISABLE_GLOBAL_IRQ();
        DISABLE_FAST_IRQ(0);

        switch (rx_cmd) {

        case CMD_PING:
            uart_transmit_byte(&uart0, RESP_PONG);
            break;

        case CMD_KEY:
            memcpy(aes_key, (const void *) rx_buf, AES_BLK);
            aes_key_setup(aes_key, key_schedule, AES_KEYSIZE);
            uart_transmit_byte(&uart0, RESP_KEY_OK);
            break;

        case CMD_ENC:
            memcpy(in_blk, (const void *) rx_buf, AES_BLK);
            triggered_encrypt();
            send_block(out_blk, AES_BLK);
            break;

        default:
            uart_transmit_byte(&uart0, RESP_ERR);
            break;
        }

        rx_reset();
    }

    return 0;
}

/*====================== INTERRUPT HANDLERS ========================*/

void mti_handler(void) {}
void mei_handler(void) {}
void msi_handler(void) {}
void exc_handler(void) {}
void fast_irq1_handler(void) {}

void fast_irq0_handler(void)
{
    volatile uint8_t *rx_ptr = (volatile uint8_t *) UART_RX_ADDR;
    uint8_t b = *rx_ptr;

    if (!rx_have_cmd) {
        rx_cmd      = b;
        rx_count    = 0u;
        rx_expect   = payload_len_for(b);
        rx_have_cmd = 1u;

        if (rx_expect == 0u) {
            rx_have_cmd = 0u;
            frame_ready = 1u;
            DISABLE_GLOBAL_IRQ();
        }
        return;
    }

    if (rx_count < AES_BLK)
        rx_buf[rx_count] = b;
    rx_count++;

    if (rx_count >= rx_expect) {
        rx_have_cmd = 0u;
        frame_ready = 1u;
        DISABLE_GLOBAL_IRQ();
    }
}