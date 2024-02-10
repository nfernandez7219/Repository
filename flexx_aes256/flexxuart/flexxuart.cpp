#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

// Enable ECB, CTR and CBC mode. Note this can be done before including aes.h or at compile-time.
// E.g. with GCC by using the -D flag: gcc -c aes.c -DCBC=0 -DCTR=1 -DECB=1
#define CBC 1
#define CTR 1
#define ECB 1

#include "aes.h"


static void phex(uint8_t* str);
static int test_encrypt_cbc(void);
static int test_decrypt_cbc(void);
static int test_encrypt_ctr(void);
static int test_decrypt_ctr(void);
static int test_encrypt_ecb(void);
static int test_decrypt_ecb(void);
static void test_encrypt_ecb_verbose(void);

/* IV is constant */
static uint8_t iv[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

static int flexx_encrypt(uint8_t *keyptr, uint8_t *message, int msglen,
                         uint8_t *cipher_ptr, int *cipherlen)
{
        int len = msglen;
        struct AES_ctx ctx;
        int err = 0;

        /* check if msg is a multiple of 16 bytes */
        if (len & (0xF)) {
                len = (len + (0xf-1)) & ~(0xf-1);
        }

        if (len > * cipherlen) {
                *cipherlen = len;
                return -1;
        }

        if (!cipher_ptr || !message)
                return -2;

        memset(cipher_ptr, 0, len);
        memcpy(cipher_ptr, message, len);

        AES_init_ctx_iv(&ctx, keyptr, iv);
        AES_CBC_encrypt_buffer(&ctx, cipher_ptr, len);

        return err;
}


static int flexx_decrypt(uint8_t* keyptr, uint8_t *cipher_ptr, int cipherlen,
        uint8_t* message, int* messagelen)
{
        int len = cipherlen;
        struct AES_ctx ctx;
        int err = 0;

        /* check if cipherlen is a multiple of 16 bytes */
        if (len & (0xF)) {
                len = (len + (0xf - 1)) & ~(0xf - 1);
        }

        if (len > * messagelen) {
                *messagelen = len;
                return -1;
        }

        if (!cipher_ptr || !message)
                return -2;

        memset(message, 0, len);
        memcpy(message, cipher_ptr, len);

        AES_init_ctx_iv(&ctx, keyptr, iv);
        AES_CBC_decrypt_buffer(&ctx, message, len);

        return err;
}


int main(void)
{
        uint8_t key[] = { 0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
                          0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4 };
        uint8_t original_message[] = "Bitmicro";
        uint8_t cipher_text[128];
        int cipher_text_len = 0;
        int err;

        uint8_t decrypted_message[128];
        int decryped_message_len;



        //exit = test_encrypt_cbc() + test_decrypt_cbc();

        /* first try to see if output buffer is enough */
        err = flexx_encrypt(key, original_message, strlen((char *)original_message), cipher_text, &cipher_text_len);
        assert(err == -1);

        /* now do the actual encryption */
        assert(cipher_text_len <= sizeof(cipher_text));
        err = flexx_encrypt(key, original_message, strlen((char*)original_message), cipher_text, &cipher_text_len);


        /* first try to see if output buffer is enough */
        err = flexx_decrypt(key, cipher_text, cipher_text_len, decrypted_message, &decryped_message_len);
        assert(err == -1);

        /* now do the actual decryption */
        assert(decryped_message_len <= sizeof(decrypted_message));
        err = flexx_decrypt(key, cipher_text, cipher_text_len, decrypted_message, &decryped_message_len);

        if (strcmp((char *)original_message, (char*)decrypted_message) == 0) {
                printf("aes 256 experiment successful!\n");
        } else {
                printf("aes 256 experiment not successful!\n");
        }


        return 0;
}


// prints string as hex
static void phex(uint8_t* str)
{

#if defined(AES256)
        uint8_t len = 32;
#elif defined(AES192)
        uint8_t len = 24;
#elif defined(AES128)
        uint8_t len = 16;
#endif

        unsigned char i;
        for (i = 0; i < len; ++i)
                printf("%.2x", str[i]);
        printf("\n");
}

static int test_decrypt_cbc(void)
{
        uint8_t key[] = { 0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
                          0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4 };
        uint8_t in[] = { 0xf5, 0x8c, 0x4c, 0x04, 0xd6, 0xe5, 0xf1, 0xba, 0x77, 0x9e, 0xab, 0xfb, 0x5f, 0x7b, 0xfb, 0xd6,
                          0x9c, 0xfc, 0x4e, 0x96, 0x7e, 0xdb, 0x80, 0x8d, 0x67, 0x9f, 0x77, 0x7b, 0xc6, 0x70, 0x2c, 0x7d,
                          0x39, 0xf2, 0x33, 0x69, 0xa9, 0xd9, 0xba, 0xcf, 0xa5, 0x30, 0xe2, 0x63, 0x04, 0x23, 0x14, 0x61,
                          0xb2, 0xeb, 0x05, 0xe2, 0xc3, 0x9b, 0xe9, 0xfc, 0xda, 0x6c, 0x19, 0x07, 0x8c, 0x6a, 0x9d, 0x1b };
        
        uint8_t out[] = { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
                          0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
                          0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11, 0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
                          0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17, 0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10 };
        //  uint8_t buffer[64];
        struct AES_ctx ctx;

        AES_init_ctx_iv(&ctx, key, iv);
        AES_CBC_decrypt_buffer(&ctx, in, 64);

        printf("CBC decrypt: ");

        if (0 == memcmp((char*)out, (char*)in, 64)) {
                printf("SUCCESS!\n");
                return(0);
        }
        else {
                printf("FAILURE!\n");
                return(1);
        }
}

static int test_encrypt_cbc(void)
{
        /* the aes 256 bit key */
        uint8_t key[] = { 0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81,
                          0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4 };

        /* the cipher text expected output */
        uint8_t out[] = { 0xf5, 0x8c, 0x4c, 0x04, 0xd6, 0xe5, 0xf1, 0xba, 0x77, 0x9e, 0xab, 0xfb, 0x5f, 0x7b, 0xfb, 0xd6,
                          0x9c, 0xfc, 0x4e, 0x96, 0x7e, 0xdb, 0x80, 0x8d, 0x67, 0x9f, 0x77, 0x7b, 0xc6, 0x70, 0x2c, 0x7d,
                          0x39, 0xf2, 0x33, 0x69, 0xa9, 0xd9, 0xba, 0xcf, 0xa5, 0x30, 0xe2, 0x63, 0x04, 0x23, 0x14, 0x61,
                          0xb2, 0xeb, 0x05, 0xe2, 0xc3, 0x9b, 0xe9, 0xfc, 0xda, 0x6c, 0x19, 0x07, 0x8c, 0x6a, 0x9d, 0x1b };

        uint8_t iv[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

        /* the plain text intput string */
        uint8_t in[] = { 0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
                          0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51,
                          0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11, 0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef,
                          0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17, 0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10 };
        struct AES_ctx ctx;

        AES_init_ctx_iv(&ctx, key, iv);
        AES_CBC_encrypt_buffer(&ctx, in, 64);

        printf("CBC encrypt: ");

        if (0 == memcmp((char*)out, (char*)in, 64)) {
                printf("SUCCESS!\n");
                return(0);
        }
        else {
                printf("FAILURE!\n");
                return(1);
        }
}

