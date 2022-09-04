#pragma once

struct FIBITMAP;

#define MAX_HASH_BITS_PER_CHANNEL 64
#define MAX_HASH_BYTES_PER_CHANNEL ((MAX_HASH_BITS_PER_CHANNEL+63)/8)
#define PHASH_DCT_SIZE	32

// number of bits for the hash might change in the future
typedef struct A_HASH_RGB
{
	unsigned int hashBitsPerRow; // divide image to this amount of parts that will represent 1 bit in the hash
	unsigned char rHashBits[MAX_HASH_BYTES_PER_CHANNEL];
	unsigned char gHashBits[MAX_HASH_BYTES_PER_CHANNEL];
	unsigned char bHashBits[MAX_HASH_BYTES_PER_CHANNEL];
}A_HASH_RGB;

typedef struct A_HASH_RGB_CompareResult
{
	size_t rBitsMatch;
	size_t gBitsMatch;
	size_t bBitsMatch;
	double rPctMatch;
	double gPctMatch;
	double bPctMatch;
	double pctMatchAvg;
}A_HASH_RGB_CompareResult;

void aHashInit(A_HASH_RGB* in, size_t hashBits);
int genAHash(FIBITMAP* in_Img, A_HASH_RGB* out_hash);
int genAHashGrayScale(FIBITMAP* in_Img, A_HASH_RGB* out_hash);
int genPHash(FIBITMAP* in_Img, A_HASH_RGB* out_hash);
int genPHashGrayScale(FIBITMAP* in_Img, A_HASH_RGB* out_hash);
void printAHash(A_HASH_RGB* in_hash, int isGrayScale);
void compareHash(A_HASH_RGB* h1, A_HASH_RGB* h2, A_HASH_RGB_CompareResult *out);
