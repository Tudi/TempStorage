#include "serialize_generic.h"
#include "crc.h"
#include <stddef.h>
#include <inttypes.h>
#include <stdlib.h>
#include <memory.h>
#ifdef ENABLE_GSS_ASSERTS
#include <assert.h>
#define serialize_assert(x) assert(x)
#else
#define serialize_assert(x)
#endif

#ifdef ENABLE_GG_SAFETY_CHECKS
	#define BOUNDS_CHECK_DATA_TYPE unsigned int
	#define BOUNDS_CHECK_HEADER 0x0BADBEEF
	#define BOUNDS_CHECK_FOOTER 0xFEEDFACE
#endif

void* create_GSS(int maxFields, unsigned int* in_out_allocated)
{
	serialize_assert(in_out_allocated != NULL);

	size_t allocSize = sizeof(GenericSerializedStructure);
	allocSize += maxFields * sizeof(GenericSerializedStructureFieldDescriptor);

	GenericSerializedStructure* ret = malloc(allocSize + MIN_ALLOC_SIZE_GSS);
	if (ret == NULL)
	{
		serialize_assert(0);
		return NULL;
	}
	memset(ret, 0, allocSize);

	ret->size = allocSize;
	ret->fieldCount = maxFields;

	*in_out_allocated = allocSize + MIN_ALLOC_SIZE_GSS;

	return ret;
}

static int ensureEnoughBytesAvailable(GenericSerializedStructure** store, unsigned int* in_out_allocated, int needed)
{
	serialize_assert(store != NULL);
	serialize_assert(*store != NULL);
	serialize_assert(in_out_allocated != NULL);
	serialize_assert((*in_out_allocated) >= (*store)->size);

	int bytesAvailable = (*in_out_allocated) - (*store)->size;
	if (bytesAvailable < needed)
	{
		void* newBuffer = realloc((*store), (*in_out_allocated) + needed + MIN_ALLOC_SIZE_GSS);
		if (newBuffer)
		{
			free(*store);
			*store = NULL;
			serialize_assert(0);
			return 1;
		}
		(*store) = (GenericSerializedStructure*)newBuffer;
		*in_out_allocated += needed + MIN_ALLOC_SIZE_GSS;
	}
	return 0;
}

static int isFieldIsUnique(GenericSerializedStructure* store, unsigned short TFID)
{
	serialize_assert(store != NULL);
	for (size_t i = 0; i < store->fieldCount; i++)
	{
		if (store->fields[i].type == TFID)
		{
			return 0;
		}
	}
	return 1;
}

int setData_GSS(GenericSerializedStructure** store, unsigned int* in_out_allocated,
	unsigned short TFIndex, unsigned short TFID, void* data, unsigned int size)
{
	serialize_assert(store != NULL);
	serialize_assert(*store != NULL);
	serialize_assert(in_out_allocated != NULL);
	serialize_assert((*in_out_allocated) >= (*store)->size);
	serialize_assert(data != NULL);

	uint8_t* dataWriteLoc;
	if (startDataWrite_GSS(store, in_out_allocated, TFIndex, TFID, &dataWriteLoc, MIN_ALLOC_SIZE_FIELD_GSS) != 0)
	{
		return 1;
	}

	memcpy(dataWriteLoc, data, size);

	endDataWrite_GSS(*store, size);

	return 0;
}

int startDataWrite_GSS(GenericSerializedStructure** store, unsigned int* in_out_allocated,
	unsigned short TFIndex, unsigned short TFID, unsigned char** data, unsigned int sizeReserve)
{
	serialize_assert(store != NULL);
	serialize_assert(*store != NULL);
	serialize_assert(in_out_allocated != NULL);
	serialize_assert((*in_out_allocated) >= (*store)->size);
	serialize_assert(data != NULL);

	if (ensureEnoughBytesAvailable(store, in_out_allocated, sizeReserve) != 0)
	{
		return 1;
	}

	serialize_assert(TFIndex < (*store)->fieldCount);
	serialize_assert((*store)->fields[TFIndex].offset == 0);
	serialize_assert((*store)->fields[TFIndex].type == 0);
	serialize_assert(isFieldIsUnique(*store, TFID) == 1);

	(*store)->fields[TFIndex].offset = (*store)->size;
	(*store)->fields[TFIndex].type = TFID;

	*data = (uint8_t*)(*store) + (*store)->size;

	return 0;
}

void endDataWrite_GSS(GenericSerializedStructure* store, unsigned int size)
{
	serialize_assert(store != NULL);
	store->size += size;
}

static inline int getRealFieldIndex(const GenericSerializedStructure* store,
	unsigned short TFIndex, unsigned short TFID)
{
	serialize_assert(store != NULL);
	// maybe field got relocated, try to find it
	if (TFIndex >= (store)->fieldCount || (store)->fields[TFIndex].type != TFID)
	{
		for (size_t i = 0; i < (store)->fieldCount; i++)
		{
			if ((store)->fields[TFIndex].type == TFID)
			{
				return i;
			}
		}
		return -1;
	}
	return TFIndex;
}

int getData_GSS(const GenericSerializedStructure* store,
	unsigned short pTFIndex, unsigned short TFID, const unsigned char** data)
{
	serialize_assert(store != NULL);
	serialize_assert(data != NULL);

	int TFIndex = getRealFieldIndex(store, pTFIndex, TFID);
	if (TFIndex < 0)
	{
		*data = 0;
		return -1;
	}
	// field is missing data. 'null' value
	if ((store)->fields[TFIndex].offset == 0)
	{
		*data = 0;
		return 0;
	}

	serialize_assert((store)->fields[TFIndex].offset < store->size);
	serialize_assert((store)->fields[TFIndex].offset + getSize_GSS(store, TFIndex, TFID) <= store->size);

	*data = (uint8_t*)(store) + (store)->fields[TFIndex].offset;
	return 0;
}

int getSize_GSS(const GenericSerializedStructure* store,
	unsigned short pTFIndex, unsigned short TFID)
{
	serialize_assert(store != NULL);

	int TFIndex = getRealFieldIndex(store, pTFIndex, TFID);
	if (TFIndex < 0)
	{
		return 0;
	}
	int curOffset = store->fields[TFIndex].offset;
	while (TFIndex++ && TFIndex < store->fieldCount && store->fields[TFIndex].offset == 0);
	if (TFIndex < store->fieldCount && store->fields[TFIndex].offset != 0)
	{
		return store->fields[TFIndex].offset - curOffset;
	}

	return store->size - curOffset;
}

void UpdateCRC_GSS(GenericSerializedStructure* store)
{
#ifdef ENABLE_GG_SAFETY_CHECKS
	uint64_t newCRC = 0;
	store->desc.crc = 0;
	newCRC = crc64(newCRC, store, store->size); // could use utils lib
	store->desc.crc = newCRC;
#endif
}

int CheckCRC_GSS(GenericSerializedStructure* store)
{
#ifdef ENABLE_GG_SAFETY_CHECKS
	uint64_t oldCRCVal = store->desc.crc;
	UpdateCRC_GSS(store);
	uint64_t newCRCVal = store->desc.crc;
	store->desc.crc = oldCRCVal; // restore old value good or bad
	return oldCRCVal != newCRCVal;
#else
	return 1;
#endif
}
