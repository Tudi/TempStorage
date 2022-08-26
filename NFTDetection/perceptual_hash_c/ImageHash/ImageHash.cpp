#include "StdAfx.h"

void aHashInit(A_HASH_RGB* in, size_t hashBits)
{
	memset(in, 0, sizeof(A_HASH_RGB));
	in->hashBitsPerRow = (unsigned int)sqrt((double)hashBits);
	if (in->hashBitsPerRow * in->hashBitsPerRow != hashBits)
	{
		printf("Please provide a number that can be squared : %u*%u != %zu\n", in->hashBitsPerRow, in->hashBitsPerRow, hashBits);
		assert(false);
	}
}

int genAHash(FIBITMAP* in_Img, A_HASH_RGB* out_hash)
{
	BitWriter bw[3];
	size_t hashBits = out_hash->hashBitsPerRow * out_hash->hashBitsPerRow;
	bitWriterinit(&bw[0], out_hash->rHashBits, hashBits);
	bitWriterinit(&bw[1], out_hash->gHashBits, hashBits);
	bitWriterinit(&bw[2], out_hash->bHashBits, hashBits);

	size_t pitch = FreeImage_GetPitch(in_Img);
	BYTE* BITS = FreeImage_GetBits(in_Img);
	size_t Width = FreeImage_GetWidth(in_Img);
	size_t Height = FreeImage_GetHeight(in_Img);

	// get the luminosity of each bit
	size_t hashBitsPerRow = out_hash->hashBitsPerRow;
	size_t rowCountPixelMat = (Height + hashBitsPerRow - 1) / hashBitsPerRow;
	size_t colCountPixelMat = (Width + hashBitsPerRow - 1) / hashBitsPerRow;
	size_t bytesNeededBitLuminosity = hashBitsPerRow * hashBitsPerRow * ColorChannelCount * sizeof(size_t);
	size_t* bitLuminosity = (size_t*)malloc(bytesNeededBitLuminosity);
	if (bitLuminosity == NULL)
	{
		assert(bitLuminosity != NULL);
		return 1;
	}
	memset(bitLuminosity, 0, bytesNeededBitLuminosity);
	size_t bitLuminosityStride = hashBitsPerRow * ColorChannelCount;
	for (size_t row = 0; row < Height; row++)
	{
		size_t bitmatRow = row / rowCountPixelMat;
		for (size_t col = 0; col < Width; col++)
		{
			size_t bitmatCol = col / colCountPixelMat;
			bitLuminosity[bitmatRow * bitLuminosityStride + bitmatCol * ColorChannelCount + 0] += BITS[row * pitch + col * Bytespp + 0];
			bitLuminosity[bitmatRow * bitLuminosityStride + bitmatCol * ColorChannelCount + 1] += BITS[row * pitch + col * Bytespp + 1];
			bitLuminosity[bitmatRow * bitLuminosityStride + bitmatCol * ColorChannelCount + 2] += BITS[row * pitch + col * Bytespp + 2];
		}
	}

	// get avg luminosity of each channel
	size_t SumLum[3] = { 0 };
	for (size_t row = 0; row < hashBitsPerRow; row++)
	{
		for (size_t col = 0; col < hashBitsPerRow; col++)
		{
			SumLum[0] += bitLuminosity[row * bitLuminosityStride + col * ColorChannelCount + 0];
			SumLum[1] += bitLuminosity[row * bitLuminosityStride + col * ColorChannelCount + 1];
			SumLum[2] += bitLuminosity[row * bitLuminosityStride + col * ColorChannelCount + 2];
		}
	}
	size_t pixelsInBitMatSquare = rowCountPixelMat * colCountPixelMat;
	// relative scale to the size of the small square of pixels
	SumLum[0] *= pixelsInBitMatSquare;
	SumLum[1] *= pixelsInBitMatSquare;
	SumLum[2] *= pixelsInBitMatSquare;

	// get the actual bit value based on relative bitmat value
	for (size_t row = 0; row < hashBitsPerRow; row++)
	{
		for (size_t col = 0; col < hashBitsPerRow; col++)
		{
			size_t rBitVal = (bitLuminosity[row * bitLuminosityStride + col * ColorChannelCount + 0] * (Height * Width) >= SumLum[0]);
			bitWriterWrite(&bw[0], rBitVal);
			size_t gBitVal = (bitLuminosity[row * bitLuminosityStride + col * ColorChannelCount + 1] * (Height * Width) >= SumLum[1]);
			bitWriterWrite(&bw[1], gBitVal);
			size_t bBitVal = (bitLuminosity[row * bitLuminosityStride + col * ColorChannelCount + 2] * (Height * Width) >= SumLum[2]);
			bitWriterWrite(&bw[2], bBitVal);
		}
	}

	free(bitLuminosity);

	return 0;
}

void printAHash(A_HASH_RGB* in_hash)
{
	for (size_t i = 0; i < (in_hash->hashBitsPerRow + 7) / 8; i += 8)
	{
		printf("%zu,", *(size_t*)&in_hash->rHashBits[i]);
		printf("%zu,", *(size_t*)&in_hash->gHashBits[i]);
		printf("%zu,", *(size_t*)&in_hash->bHashBits[i]);
	}
}

// it would be "prettier" if we extracted code and perform the operations 3 times, but sooner or later speed will start to matter
int genPHash(FIBITMAP* in_Img, A_HASH_RGB* out_hash)
{
	BitWriter bw[3];
	const size_t hashBitsPerRow = out_hash->hashBitsPerRow;
	const size_t hashBits = hashBitsPerRow * hashBitsPerRow;

	assert(hashBitsPerRow <= PHASH_DCT_SIZE);

	bitWriterinit(&bw[0], out_hash->rHashBits, hashBits);
	bitWriterinit(&bw[1], out_hash->gHashBits, hashBits);
	bitWriterinit(&bw[2], out_hash->bHashBits, hashBits);

	size_t pitch = FreeImage_GetPitch(in_Img);
	BYTE* BITS = FreeImage_GetBits(in_Img);
	size_t Width = FreeImage_GetWidth(in_Img);
	size_t Height = FreeImage_GetHeight(in_Img);

	// scale down the image into 3 different 32x32 matrixes
	size_t rowCountDCTMat = (Height + PHASH_DCT_SIZE - 1) / PHASH_DCT_SIZE;
	size_t colCountDCTMat = (Width + PHASH_DCT_SIZE - 1) / PHASH_DCT_SIZE;
	size_t bytesNeededDCT = PHASH_DCT_SIZE * PHASH_DCT_SIZE * ColorChannelCount * sizeof(double);
	double* DCTMAT = (double*)malloc(bytesNeededDCT);
	double* DCTMAT_Res = (double*)malloc(bytesNeededDCT);
	if (DCTMAT == NULL || DCTMAT_Res == NULL)
	{
		assert(DCTMAT != NULL);
		assert(DCTMAT_Res != NULL);
		return 1;
	}
	memset(DCTMAT, 0, bytesNeededDCT);
	memset(DCTMAT_Res, 0, bytesNeededDCT);
	size_t DCTMatRowStride = PHASH_DCT_SIZE;
	size_t DCTMatStride = PHASH_DCT_SIZE * PHASH_DCT_SIZE;
	double* DCTMAT_R = &DCTMAT[0 * DCTMatStride];
	double* DCTMAT_G = &DCTMAT[1 * DCTMatStride];
	double* DCTMAT_B = &DCTMAT[2 * DCTMatStride];
	for (size_t row = 0; row < Height; row++)
	{
		size_t bitmatRow = row / rowCountDCTMat;
		for (size_t col = 0; col < Width; col++)
		{
			size_t bitmatCol = col / colCountDCTMat;
			DCTMAT_R[bitmatRow * DCTMatRowStride + bitmatCol] += BITS[row * pitch + col * Bytespp + 0];
			DCTMAT_G[bitmatRow * DCTMatRowStride + bitmatCol] += BITS[row * pitch + col * Bytespp + 1];
			DCTMAT_B[bitmatRow * DCTMatRowStride + bitmatCol] += BITS[row * pitch + col * Bytespp + 2];
		}
	}

	// perform the DCT
	dctII(&DCTMAT_Res[0 * DCTMatStride], DCTMAT_R, PHASH_DCT_SIZE, PHASH_DCT_SIZE);
	dctII(&DCTMAT_Res[1 * DCTMatStride], DCTMAT_G, PHASH_DCT_SIZE, PHASH_DCT_SIZE);
	dctII(&DCTMAT_Res[2 * DCTMatStride], DCTMAT_B, PHASH_DCT_SIZE, PHASH_DCT_SIZE);

	DCTMAT_R = &DCTMAT_Res[0 * DCTMatStride];
	DCTMAT_G = &DCTMAT_Res[1 * DCTMatStride];
	DCTMAT_B = &DCTMAT_Res[2 * DCTMatStride];

	// get avg DCT of each channel
	double AvgDCT[3] = { 0 };
	for (size_t row = 0; row < hashBitsPerRow; row++)
	{
		for (size_t col = 0; col < hashBitsPerRow; col++)
		{
			AvgDCT[0] += DCTMAT_R[row * PHASH_DCT_SIZE + col];
			AvgDCT[1] += DCTMAT_G[row * PHASH_DCT_SIZE + col];
			AvgDCT[2] += DCTMAT_B[row * PHASH_DCT_SIZE + col];
		}
	}
	double valueCountDCTHash = (double)(hashBits * hashBits);
	AvgDCT[0] = AvgDCT[0] / valueCountDCTHash;
	AvgDCT[1] = AvgDCT[1] / valueCountDCTHash;
	AvgDCT[2] = AvgDCT[2] / valueCountDCTHash;

	// get the actual bit value based on relative bitmat value
	for (size_t row = 0; row < hashBitsPerRow; row++)
	{
		for (size_t col = 0; col < hashBitsPerRow; col++)
		{
			size_t rBitVal = (DCTMAT_R[row * PHASH_DCT_SIZE + col] >= AvgDCT[0]);
			bitWriterWrite(&bw[0], rBitVal);
			size_t gBitVal = (DCTMAT_G[row * PHASH_DCT_SIZE + col] >= AvgDCT[1]);
			bitWriterWrite(&bw[1], gBitVal);
			size_t bBitVal = (DCTMAT_B[row * PHASH_DCT_SIZE + col] >= AvgDCT[2]);
			bitWriterWrite(&bw[2], bBitVal);
		}
	}

	free(DCTMAT_Res);
	free(DCTMAT);

	return 0;
}

static size_t bitCount(size_t num)
{
	size_t setBits = 0;
	while (num > 0) {
		setBits += (num & 1);
		num >>= 1;
	}
	return setBits;
}

void compareHash(A_HASH_RGB* h1, A_HASH_RGB* h2, A_HASH_RGB_CompareResult* out)
{
	memset(out, 0, sizeof(A_HASH_RGB_CompareResult));

	const size_t hashBitcount = h1->hashBitsPerRow * h1->hashBitsPerRow;

	// get the haming distance for each channel
	for (size_t i = 0; i < MAX_HASH_BYTES_PER_CHANNEL; i++)
	{
		size_t valxor = ((int)h1->rHashBits[i]) ^ ((int)h2->rHashBits[i]);
		out->rBitsMatch += bitCount(valxor);
		valxor = ((int)h1->gHashBits[i]) ^ ((int)h2->gHashBits[i]);
		out->gBitsMatch += bitCount(valxor);
		valxor = ((int)h1->bHashBits[i]) ^ ((int)h2->bHashBits[i]);
		out->bBitsMatch += bitCount(valxor);
	}

	// store the inverse of the Hamming distance
	out->rBitsMatch = hashBitcount - out->rBitsMatch;
	out->gBitsMatch = hashBitcount - out->gBitsMatch;
	out->bBitsMatch = hashBitcount - out->bBitsMatch;

	out->rPctMatch = out->rBitsMatch * 100.0 / double(hashBitcount);
	out->gPctMatch = out->gBitsMatch * 100.0 / double(hashBitcount);
	out->bPctMatch = out->bBitsMatch * 100.0 / double(hashBitcount);

	out->pctMatchAvg = (out->rPctMatch + out->gPctMatch + out->bPctMatch) / 3;
}
