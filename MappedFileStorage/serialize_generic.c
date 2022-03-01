#include "serialize_generic.h"
#include "crc.h"
#include <stddef.h>
#include <inttypes.h>
#include <stdlib.h>
#include <memory.h>
#ifndef DISABLE_SERIALIZE_SAFETY_CHECKS
#include <assert.h>
#define serialize_assert(x) assert(x)
#else
#define serialize_assert(x)
#endif

#define BOUNDS_CHECK_DATA_TYPE unsigned int
#define BOUNDS_CHECK_HEADER 0x0BADBEEF
#define BOUNDS_CHECK_FOOTER 0xFEEDFACE

#ifndef DISABLE_SERIALIZE_SAFETY_CHECKS
static void setInt32ArrayFieldValueNoChecks(GenericSerializedStructure* serializedStruct, int fieldName, int index, int value)
{
	char* dataStart = (char*)serializedStruct;
	int* fieldStart = (int*)(dataStart + serializedStruct->fieldIndexes[fieldName]);
	fieldStart[index] = value;
}

static void* getGenericSerializableStructData_(GenericSerializedStructure* store, int fieldName)
{
	return ((char*)store + store->fieldIndexes[fieldName]);
}
#endif

static int setGenericSerializableStructData_(GenericSerializedStructure* store, int fieldName, void* fieldData, int fieldSize)
{
	char* dataStart = (char*)store;
	char* fieldStart = (char*)(dataStart + store->fieldIndexes[fieldName]);
	memcpy(fieldStart, fieldData, fieldSize);

	return 0;
}

static void AppendUniqueFields(GenericSerializedStructure** store, int fieldName, int fieldSize)
{
	void* newStore = realloc((*store), (*store)->size + fieldSize);
	if (newStore == NULL)
	{
		serialize_assert(0);
		return;
	}
	*store = (GenericSerializedStructure*)newStore;
	(*store)->fieldIndexes[fieldName] = (*store)->size;
	if (fieldName == GSSFN_FIELD_INDEXES)
	{
		for (size_t i = 0; i < fieldSize / sizeof((*store)->fieldIndexes[0]); i++)
		{
			(*store)->fieldIndexes[i] = SFT_NON_USED_UNKNOWN_VAUE;
		}
		(*store)->fieldIndexes[fieldName] = (*store)->size;
	}
#ifndef DISABLE_SERIALIZE_SAFETY_CHECKS
	else if (fieldName == GSSFN_FIELD_TYPES || fieldName == GSSFN_FIELD_SIZES)
	{
		for (size_t i = 0; i < fieldSize / sizeof((*store)->fieldIndexes[0]); i++)
		{
			setInt32ArrayFieldValueNoChecks(*store, fieldName, i, SFT_NON_USED_UNKNOWN_VAUE);
		}
	}
#endif
	(*store)->size += fieldSize;
}

void* createGenericSerializableStruct(int maxFieldNames, int version)
{
	GenericSerializedStructure* ret = malloc(sizeof(GenericSerializedStructure));
	if (ret == NULL)
	{
		serialize_assert(0);
		return NULL;
	}

	ret->size = sizeof(GenericSerializedStructure);
	AppendUniqueFields(&ret, GSSFN_FIELD_INDEXES, maxFieldNames * sizeof(ret->fieldIndexes[0]));
#ifndef DISABLE_SERIALIZE_SAFETY_CHECKS
	AppendUniqueFields(&ret, GSSFN_FIELD_TYPES, maxFieldNames * sizeof(int));
	setInt32ArrayFieldValueNoChecks(ret, GSSFN_FIELD_TYPES, GSSFN_FIELD_INDEXES, SFT_4BYTE_FIXED_ARRAY);
	setInt32ArrayFieldValueNoChecks(ret, GSSFN_FIELD_TYPES, GSSFN_FIELD_TYPES, SFT_4BYTE_FIXED_ARRAY);
	AppendUniqueFields(&ret, GSSFN_FIELD_SIZES, maxFieldNames * sizeof(int));
	setInt32ArrayFieldValueNoChecks(ret, GSSFN_FIELD_SIZES, GSSFN_FIELD_INDEXES, maxFieldNames * sizeof(int));
	setInt32ArrayFieldValueNoChecks(ret, GSSFN_FIELD_SIZES, GSSFN_FIELD_TYPES, maxFieldNames * sizeof(int));
	setInt32ArrayFieldValueNoChecks(ret, GSSFN_FIELD_SIZES, GSSFN_FIELD_SIZES, maxFieldNames * sizeof(int));
	AppendUniqueFields(&ret, GSSFN_FIELD_MAX_INDEX, sizeof(maxFieldNames));
	setInt32ArrayFieldValueNoChecks(ret, GSSFN_FIELD_MAX_INDEX, 0, maxFieldNames);
	appendGenericSerializableStructData(&ret, GSSFN_FIELD_VERSION, SFT_4BYTE_FIXED, &version, sizeof(version));
	uint64_t crc = 0;
	appendGenericSerializableStructData(&ret, GSSFN_FIELD_CRC, SFT_8BYTE_FIXED , &crc, sizeof(crc));
#endif
	return ret;
}

#ifndef DISABLE_SERIALIZE_SAFETY_CHECKS
static int getInt32ArrayFieldValueNoChecks(GenericSerializedStructure* serializedStruct, int fieldName, int index)
{
	char* dataStart = (char*)serializedStruct;
	int* fieldStart = (int*)(dataStart + serializedStruct->fieldIndexes[fieldName]);
	return fieldStart[index];
}

static int getGenericSerializableStructFieldCount(GenericSerializedStructure* store)
{
	int calculatedSize = ((store)->fieldIndexes[GSSFN_FIELD_TYPES] - (store)->fieldIndexes[GSSFN_FIELD_INDEXES]) / sizeof((store)->fieldIndexes[0]);
	int savedSize = getInt32ArrayFieldValueNoChecks(store, GSSFN_FIELD_MAX_INDEX, 0);
	if (calculatedSize != savedSize)
	{
		serialize_assert(0);
		return 0;
	}
	return savedSize;
}

void UpdateCRCGenericSerializableStruct(GenericSerializedStructure* store)
{
	uint64_t newCRC = 0;
	setGenericSerializableStructData_(store, GSSFN_FIELD_CRC, &newCRC, sizeof(newCRC));
	newCRC = crc64(newCRC, store, store->size); // could use utils lib
	setGenericSerializableStructData_(store, GSSFN_FIELD_CRC, &newCRC, sizeof(newCRC));
}

int CheckCRCGenericSerializableStruct(GenericSerializedStructure* store)
{
	uint64_t *oldCRC = getGenericSerializableStructData_(store, GSSFN_FIELD_CRC);
	uint64_t oldCRCVal = *oldCRC;
	UpdateCRCGenericSerializableStruct(store);
	uint64_t*newCRC = getGenericSerializableStructData_(store, GSSFN_FIELD_CRC);
	uint64_t newCRCVal = *newCRC;
	setGenericSerializableStructData_(store, GSSFN_FIELD_CRC, oldCRC, sizeof(oldCRC));
	return oldCRCVal != newCRCVal;
}

static int safetyChecksOnFieldOperation(GenericSerializedStructure* store, int fieldName, SerializableFieldTypes dataType, void* fieldData, int fieldSize)
{
	if (store == NULL)
	{
		serialize_assert(0);
		return __LINE__;
	}

	int fieldNameMax = getGenericSerializableStructFieldCount(store);

	// Sanity checks
	if (fieldName < 0 || fieldName >= fieldNameMax
		|| fieldSize < 0 || dataType < SFT_NON_USED_UNKNOWN_VAUE || 
		dataType >= SFT_MAX_USED_VALUE_FOR_BOUNDS_CHECK)
	{
		serialize_assert(0);
		return __LINE__;
	}

	// field already has data
	if (store->fieldIndexes[fieldName] == SFT_NON_USED_UNKNOWN_VAUE)
	{
		serialize_assert(0);
		return __LINE__;
	}

	if (store->fieldIndexes[fieldName] >= store->size)
	{
		serialize_assert(0);
		return __LINE__;
	}

	int dataSize = getInt32ArrayFieldValueNoChecks(store, GSSFN_FIELD_SIZES, fieldName);
	if (fieldSize != 0 && dataSize != fieldSize)
	{
		serialize_assert(0);
		return __LINE__;
	}

	int prevDataType = getInt32ArrayFieldValueNoChecks(store, GSSFN_FIELD_TYPES, fieldName);
	if (dataType != SFT_NON_USED_UNKNOWN_VAUE && prevDataType != dataType)
	{
		serialize_assert(0);
		return __LINE__;
	}
	return 0;
}
#endif

static int appendGenericSerializableStructData_(GenericSerializedStructure** store, int fieldName, void* fieldData, int fieldSize)
{
	void* newStore = realloc((*store), (*store)->size + fieldSize);
	if (newStore == NULL)
	{
		serialize_assert(0);
		return __LINE__;
	}
	*store = (GenericSerializedStructure*)newStore;

	setInt32ArrayFieldValue((*store), GSSFN_FIELD_INDEXES, fieldName, (*store)->size);

	// you can reserver memory that you will fill out later. This is for complicated inmplace arrays like k_vec
	if (fieldData != NULL)
	{
		char* start = (char*)(*store) + (*store)->size;

#ifdef ADD_BOUNCE_CHECK
		*(BOUNDS_CHECK_DATA_TYPE*)start = BOUNDS_CHECK_HEADER;
		start += sizeof(BOUNDS_CHECK_DATA_TYPE);

		memcpy(start, fieldData, fieldSize - 2 * sizeof(BOUNDS_CHECK_DATA_TYPE));

		start += fieldSize - 2 * sizeof(BOUNDS_CHECK_DATA_TYPE);
		*(BOUNDS_CHECK_DATA_TYPE*)start = BOUNDS_CHECK_FOOTER;
#else
		memcpy(start, fieldData, fieldSize);
#endif
	}

	(*store)->size += fieldSize;

	return 0;
}

int appendGenericSerializableStructData(GenericSerializedStructure** store, int fieldName, SerializableFieldTypes dataType, void* fieldData, int fieldSize)
{
#ifndef DISABLE_SERIALIZE_SAFETY_CHECKS
	// Sanity checks
	if ((*store) == NULL)
	{
		serialize_assert(0);
		return __LINE__;
	}
	
	int fieldNameMax = getGenericSerializableStructFieldCount(*store);

	// Sanity checks
	if (fieldName < 0 || fieldName >= fieldNameMax
		|| fieldData == NULL || fieldSize <= 0 || dataType < 0)
	{
		serialize_assert(0);
		return __LINE__;
	}

	// field already has data
	if ((*store)->fieldIndexes[fieldName] != SFT_NON_USED_UNKNOWN_VAUE)
	{
		serialize_assert(0);
		return __LINE__;
	}

	setInt32ArrayFieldValue((*store), GSSFN_FIELD_TYPES, fieldName, dataType);

#ifdef ADD_BOUNCE_CHECK
	fieldSize += 2 * sizeof(BOUNDS_CHECK_DATA_TYPE);
#endif
	setInt32ArrayFieldValue((*store), GSSFN_FIELD_SIZES, fieldName, fieldSize);
#endif

	appendGenericSerializableStructData_(store, fieldName, fieldData, fieldSize);

	return 0;
}

int setGenericSerializableStructData(GenericSerializedStructure* store, int fieldName, SerializableFieldTypes dataType, void* fieldData, int fieldSize)
{
#ifndef DISABLE_SERIALIZE_SAFETY_CHECKS
	safetyChecksOnFieldOperation(store, fieldName, dataType, fieldData, fieldSize);
#endif
	setGenericSerializableStructData_(store, fieldName, fieldData, fieldSize);

	return 0;
}

void setInt32ArrayFieldValue(GenericSerializedStructure* serializedStruct, int fieldName, int index, int value)
{
#ifndef DISABLE_SERIALIZE_SAFETY_CHECKS
	safetyChecksOnFieldOperation(serializedStruct, fieldName, SFT_NON_USED_UNKNOWN_VAUE, NULL, 0);
#endif
	char* dataStart = (char*)serializedStruct;
	int* fieldStart = (int*)(dataStart + serializedStruct->fieldIndexes[fieldName]);
	fieldStart[index] = value;
}

void setInt32FieldValue(GenericSerializedStructure* serializedStruct, int fieldName, int value)
{
#ifndef DISABLE_SERIALIZE_SAFETY_CHECKS
	safetyChecksOnFieldOperation(serializedStruct, fieldName, SFT_4BYTE_FIXED, NULL, 0);
#endif
	setInt32ArrayFieldValue(serializedStruct, fieldName, 0, value);
}

#ifndef DISABLE_SERIALIZE_SAFETY_CHECKS
int getInt32ArrayFieldValue(GenericSerializedStructure* serializedStruct, int fieldName, int index)
{
	safetyChecksOnFieldOperation(serializedStruct, fieldName, SFT_NON_USED_UNKNOWN_VAUE, NULL, 0);
	char* dataStart = (char*)serializedStruct;
	int* fieldStart = (int*)(dataStart + serializedStruct->fieldIndexes[fieldName]);
	return fieldStart[index];
}

int getInt32FieldValue(GenericSerializedStructure* serializedStruct, int fieldName)
{
	safetyChecksOnFieldOperation(serializedStruct, fieldName, SFT_4BYTE_FIXED, NULL, 0);
	return getInt32ArrayFieldValue(serializedStruct, fieldName, 0);
}

int getGenericSerializableStructData(GenericSerializedStructure* store, int fieldName, SerializableFieldTypes dataType, char** out_data, int* out_size)
{
#ifndef DISABLE_SERIALIZE_SAFETY_CHECKS
	// Sanity checks
	if (store == NULL)
	{
		*out_data = NULL;
		*out_size = 0;
		return 1;
	}

	int fieldNameMax = getGenericSerializableStructFieldCount(store);
	if (fieldName < 0 || fieldName >= fieldNameMax
		|| *out_data == NULL || out_size == NULL || dataType <= SFT_NON_USED_UNKNOWN_VAUE
		|| dataType >= SFT_MAX_USED_VALUE_FOR_BOUNDS_CHECK)
	{
		*out_data = NULL;
		*out_size = 0;
		return 1;
	}

	int existingDataType = getInt32ArrayFieldValue(store, GSSFN_FIELD_TYPES, fieldName);
	if(existingDataType != dataType)
	{
		*out_data = NULL;
		*out_size = 0;
		return 1;
	}

	// field is not present ?
	if (store->fieldIndexes[fieldName] == SFT_NON_USED_UNKNOWN_VAUE)
	{
		*out_data = NULL;
		*out_size = 0;
		return 0;
	}

	// Some fields might require to know the size of the data in the field
	// For example you could "assert" if you are expecting 4 bytes for an 'int' but received different
	int outSizeCrossCheck = SFT_NON_USED_UNKNOWN_VAUE;
	if (fieldName + 1 == fieldNameMax)
	{
		outSizeCrossCheck = store->size - store->fieldIndexes[fieldName];
	}
	else if(store->fieldIndexes[fieldName + 1] != SFT_NON_USED_UNKNOWN_VAUE)
	{
		outSizeCrossCheck = store->fieldIndexes[fieldName + 1] - store->fieldIndexes[fieldName];
	}
	int outSize = getInt32ArrayFieldValue(store, GSSFN_FIELD_SIZES, fieldName);
	if (outSize != outSizeCrossCheck && outSizeCrossCheck != SFT_NON_USED_UNKNOWN_VAUE)
	{
		*out_data = NULL;
		*out_size = 0;
		return 0;
	}

	*out_size = outSize;
#else
	*out_size = SFT_NON_USED_UNKNOWN_VAUE;
#endif

	// Retrieve the pointer to the data requested
	*out_data = getGenericSerializableStructData_(store, fieldName);

#ifdef ADD_BOUNCE_CHECK
	if (*out_data != NULL)
	{
		if(*(BOUNDS_CHECK_DATA_TYPE*)(*out_data) != BOUNDS_CHECK_HEADER)
		{
			*out_data = NULL;
			*out_size = 0;
			serialize_assert(0);
			return 0;
		}
		char* pointerToFooter = (*out_data) + outSize - sizeof(BOUNDS_CHECK_DATA_TYPE);
		if (*(BOUNDS_CHECK_DATA_TYPE*)pointerToFooter != BOUNDS_CHECK_FOOTER)
		{
			*out_data = NULL;
			*out_size = 0;
			serialize_assert(0);
			return 0;
		}
		(*out_data) += sizeof(BOUNDS_CHECK_DATA_TYPE); // skip header
		*out_size -= 2 * sizeof(BOUNDS_CHECK_DATA_TYPE);
	}
#endif

	return 0;
}
#endif