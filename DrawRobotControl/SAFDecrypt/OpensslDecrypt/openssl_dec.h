#pragma once

typedef enum OpenSSLDecrypts
{
	DEC_AES_CBC, // requires IV block
	DEC_AES_CTR, // can encrypt independent blocks
	DEC_AES_GCM, // cheap version of CTR
	DEC_AES_CCM8, // with 8 bit tag and counter function support
	DEC_RSA,
	DEC_DES,
	DEC_TriDES,
	DEC_ECC,
}OpenSSLDecrypts;

int decrypt_with(OpenSSLDecrypts type, 
	unsigned char* buf1, int buf1_len, unsigned char* key1, int key1_len, unsigned char* key2, int key2_len,
	unsigned char* buf2, int buf2_len, unsigned char* key3, int key3_len, unsigned char* key4, int key4_len);