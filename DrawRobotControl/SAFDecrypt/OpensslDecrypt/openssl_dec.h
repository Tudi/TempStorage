#pragma once

typedef enum OpenSSLDecrypts
{
	AES_128,
}OpenSSLDecrypts;

int decrypt_with(OpenSSLDecrypts type, 
	unsigned char* buf1, int buf1_len, unsigned char* key1, int key1_len, unsigned char* key2, int key2_len,
	unsigned char* buf2, int buf2_len, unsigned char* key3, int key3_len, unsigned char* key4, int key4_len);