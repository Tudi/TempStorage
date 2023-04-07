#include "openssl_dec.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/aes.h>
#include <openssl/des.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>

// Function to print the contents of a buffer in hex format
void print_hex(unsigned char* buf, int len) {
    for (int i = 0; i < len; i++) {
        printf("%02x", buf[i]);
    }
    printf("\n");
}

int dec_AES(unsigned char* input, int input_len, unsigned char* key1, int key1_len, unsigned char* key2, int key2_len, unsigned char** out_buf, int& out_len, OpenSSLDecrypts type)
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
    if(type == DEC_AES_CBC)
        AES_cbc_encrypt(input, output, input_len, &aes_key, key2, AES_DECRYPT);
//    else if (type == AES_CBC)
 //       AES_ecb_encrypt(input, output, &aes_key, AES_DECRYPT);

    *out_buf = output;
    out_len = input_len;

    return 0;
}

int dec_RSA(unsigned char* encrypted_data, int encrypted_len, unsigned char* key_data, int key_len, unsigned char** out_buf, int& out_len)
{
    // Initialize OpenSSL
    static int isOpenSSLInitialized = 0;
    if (isOpenSSLInitialized == 0)
    {
        isOpenSSLInitialized = 1;
        OpenSSL_add_all_algorithms();
    }

    RSA* rsa_private_key = RSA_new();
    rsa_private_key = d2i_RSAPrivateKey(NULL, (const unsigned char**)&key_data, key_len);
    if (rsa_private_key == NULL)
    {
        *out_buf = NULL;
        out_len = 0;
        return 1;
    }

    // Decrypt the data
    unsigned char* decrypted_data = (unsigned char*)malloc(encrypted_len * 10);
    int decrypted_len = RSA_private_decrypt(encrypted_len, encrypted_data, decrypted_data, rsa_private_key, RSA_PKCS1_PADDING);

    // Clean up
    RSA_free(rsa_private_key);
    EVP_cleanup();

    *out_buf = decrypted_data;
    out_len = decrypted_len;

    return 0;
}

int decrypt_with(OpenSSLDecrypts type,
    unsigned char* buf1, int buf1_len, unsigned char* key1, int key1_len, unsigned char* key2, int key2_len,
    unsigned char* buf2, int buf2_len, unsigned char* key3, int key3_len, unsigned char* key4, int key4_len)
{
    unsigned char* out1_buf, * out2_buf;
    int out1_len = 0, out2_len = 0;
    if (type == DEC_AES_CBC)
    {
        dec_AES(buf1, buf1_len, key1, key1_len, key2, key2_len, &out1_buf, out1_len, type);
        dec_AES(buf2, buf2_len, key3, key3_len, key4, key4_len, &out2_buf, out2_len, type);

        // Print the decrypted output buffer in hex format
//        print_hex(output, input_len);

        int ret = memcmp(out1_buf, out2_buf, out1_len) == 0;
        free(out1_buf);
        free(out2_buf);
        return ret;
    }
    else if (type == DEC_RSA)
    {
        dec_RSA(buf1, buf1_len, key1, key1_len, &out1_buf, out1_len);
        dec_RSA(buf2, buf2_len, key3, key3_len, &out2_buf, out2_len);
        if (out1_len == 0 || out2_len == 0)
        {
            return 0;
        }
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