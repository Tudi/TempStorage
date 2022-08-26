#include "StdAfx.h"

void bitWriterinit(BitWriter* in, unsigned char* dst, size_t maxBits)
{
	memset(in, 0, sizeof(BitWriter));
	in->dst = dst;
	in->maxBits = maxBits;
	memset(dst, 0, maxBits / 8);
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
