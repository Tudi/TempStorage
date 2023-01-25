#include "StdAfx.h"

void bitWriterinit(BitWriter* in, unsigned char* dst, size_t maxBits, int zeroOutout = 1)
{
	memset(in, 0, sizeof(BitWriter));
	in->dst = dst;
	in->maxBits = maxBits;
	if (zeroOutout != 0)
	{
		memset(dst, 0, maxBits / 8);
	}
}

int bitWriterWrite(BitWriter* in, size_t bit)
{
	if (in->bitsWritten >= in->maxBits)
	{
		assert(false);
		return 1;
	}
	if (bit)
	{
		size_t bitPos = in->bitsWritten & 0x07;
		size_t shiftedBit = bit << bitPos;
		assert(shiftedBit <= 0xFF);
		size_t bytePos = in->bitsWritten / 8;
		in->dst[bytePos] |= shiftedBit;
	}
	in->bitsWritten++;
	return 0;
}

int bitWriterRead(BitWriter* in)
{
	if (in->bitsWritten >= in->maxBits)
	{
		return 2;
	}
	BYTE bitPos = in->bitsWritten & 0x07;
	BYTE shiftedBit = 1 << bitPos;
	assert(shiftedBit <= 0xFF);
	size_t bytePos = in->bitsWritten / 8;
	int bit = (in->dst[bytePos] & shiftedBit) != 0;
	in->bitsWritten++;
	return bit;
}