#pragma once

typedef struct BitWriter
{
	unsigned char* dst;
	size_t bitsWritten;
	size_t maxBits;
}BitWriter;

void bitWriterinit(BitWriter* in, unsigned char* dst, size_t maxBits);
// assumes destination was initialized to 0 !
int bitWriterWrite(BitWriter* in, size_t bit);
