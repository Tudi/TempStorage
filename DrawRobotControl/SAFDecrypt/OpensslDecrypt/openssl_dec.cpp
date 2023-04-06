#include "openssl_dec.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/aes.h>

// Function to print the contents of a buffer in hex format
void print_hex(unsigned char* buf, int len) {
    for (int i = 0; i < len; i++) {
        printf("%02x", buf[i]);
    }
    printf("\n");
}

int dec_AES(unsigned char* input, int input_len, unsigned char* key1, int key1_len, unsigned char* key2, int key_len, unsigned char** out_buf, int& out_len)
{
    *out_buf = NULL;
    out_len = 0;

    // Initialize the AES decryption context
    AES_KEY aes_key;
    if (AES_set_decrypt_key(key1, key1_len * 8, &aes_key) != 0) {
        printf("Error initializing AES decryption context\n");
        return 1;
    }
    unsigned char *output = (unsigned char*)malloc(input_len);
    if (output == NULL)
    {
        return 1;
    }
    memset(output, 0, input_len);

    // Decrypt the input buffer
    AES_cbc_encrypt(input, output, input_len, &aes_key, key2, AES_DECRYPT);

    *out_buf = output;
    out_len = input_len;

    return 0;
}

int decrypt_with(OpenSSLDecrypts type,
    unsigned char* buf1, int buf1_len, unsigned char* key1, int key1_len, unsigned char* key2, int key2_len,
    unsigned char* buf2, int buf2_len, unsigned char* key3, int key3_len, unsigned char* key4, int key4_len)
{
    unsigned char* out1_buf, * out2_buf;
    int out1_len = 0, out2_len = 0;
    if (type == AES_128)
    {
        dec_AES(buf1, buf1_len, key1, key1_len, key2, key2_len, &out1_buf, out1_len);
        dec_AES(buf2, buf2_len, key3, key3_len, key4, key4_len, &out2_buf, out2_len);

        // Print the decrypted output buffer in hex format
//        print_hex(output, input_len);

        int ret = memcmp(out1_buf, out2_buf, out1_len) == 0;
        free(out1_buf);
        free(out2_buf);
        return ret;
    }
    else
    {
        return 1;
    }

    return 0;
}