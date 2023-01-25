#ifndef _BITFIELD_H_
#define _BITFIELD_H_

#include <stdlib.h>
#include <string.h>

/// <summary>
/// Bitfields will be 2 IO operation lookup tables.
/// They are best used when a list of small values  are checked against another list of values
/// Alternative solution to bitfields are binary trees that trade more IO operations to less memory
/// Ypou probably should not use a BitField if the max value in the array is larger than 16 bits
/// </summary>
typedef struct BitField
{
	unsigned int size;	// number of bytes present in 'field'
	unsigned char* field; // values sored as bit index
}BitField;

// Header only library. Does not need cross compilation. Reduces risk of allocator/allignment related issues
#define initBitField(bf) { bf.size = 0; bf.field = NULL;}
#define freeBitField(bf) { bf.size = 0; free(bf.field); bf.field = NULL; }

#define BitFieldResize(bf, MaxValue) { size_t oldSize = bf.size;  \
			bf.size = (MaxValue / 8) + 1; \
			bf.field = realloc(bf.field, bf.size); \
			if(bf.size > oldSize) memset(&bf.field[oldSize], 0, bf.size-oldSize); }
#define BitFieldSet(bf, value) { if(bf.size <= value / 8) BitFieldResize(bf, value); bf.field[value / 8] |= (1 << (value & 0x07)); }
#define BitFieldSetUnsafe(bf, value) { bf.field[value / 8] = (1 << (value & 0x07)); }
#define BitFieldHasValue(bf, value, outResult) { if(value / 8 >= bf.size) outResult = 0; else outResult = (bf.field[value / 8] & (1 << (value & 0x07))); }

#endif