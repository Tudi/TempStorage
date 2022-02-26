#include "serialize_generic.h"
#include <stddef.h>
#include <inttypes.h>
#include <stdlib.h>
#include <memory.h>

static void AppendUniqueFields(GenericSerializedStructure** store, int fieldName, int fieldSize)
{
	*store = realloc((*store), (*store)->size + fieldSize);
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
			setInt32ArrayFieldValue(*store, fieldName, i, SFT_NON_USED_UNKNOWN_VAUE);
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
		return NULL;
	}

	ret->size = sizeof(GenericSerializedStructure);
	AppendUniqueFields(&ret, GSSFN_FIELD_INDEXES, maxFieldNames * sizeof(ret->fieldIndexes[0]));
#ifndef DISABLE_SERIALIZE_SAFETY_CHECKS
	AppendUniqueFields(&ret, GSSFN_FIELD_TYPES, maxFieldNames * sizeof(ret->fieldIndexes[0]));
	AppendUniqueFields(&ret, GSSFN_FIELD_SIZES, maxFieldNames * sizeof(ret->fieldIndexes[0]));
	appendGenericSerializableStructData(&ret, GSSFN_FIELD_MAX_INDEX, &maxFieldNames, sizeof(maxFieldNames));
	appendGenericSerializableStructData(&ret, GSSFN_FIELD_VERSION, &version, sizeof(version));
	uint64_t crc = 0;
	appendGenericSerializableStructData(&ret, GSSFN_FIELD_CRC, &crc, sizeof(crc));
#endif
	return ret;
}

#ifndef DISABLE_SERIALIZE_SAFETY_CHECKS
static int getGenericSerializableStructFieldCount(GenericSerializedStructure* store)
{
	int calculatedSize = ((store)->fieldIndexes[GSSFN_FIELD_TYPES] - (store)->fieldIndexes[GSSFN_FIELD_INDEXES]) / sizeof((store)->fieldIndexes[0]);
	int savedSize = getInt32FieldValue(store, GSSFN_FIELD_MAX_INDEX);
	if (calculatedSize != savedSize)
	{
		return 0;
	}
	return savedSize;
}

void UpdateCRC(GenericSerializedStructure* store)
{
}

int CheckCRC(GenericSerializedStructure* store)
{
	return 0;
}
#endif

int appendGenericSerializableStructData(GenericSerializedStructure** store, int fieldName, void* fieldData, int fieldSize)
{
	void* newStore = realloc((*store), (*store)->size + fieldSize);
	if (newStore == NULL)
	{
		return __LINE__;
	}
	*store = (GenericSerializedStructure*)newStore;

	setInt32ArrayFieldValue((*store), GSSFN_FIELD_INDEXES, fieldName, (*store)->size);
	// you can reserver memory that you will fill out later. This is for complicated inmplace arrays like k_vec
	if (fieldData != NULL)
	{
		memcpy((char*)(*store) + (*store)->size, fieldData, fieldSize);
	}
	(*store)->size += fieldSize;

	return 0;
}

int appendGenericSerializableStructDataSafe(GenericSerializedStructure** store, int fieldName, SerializableFieldTypes dataType, void* fieldData, int fieldSize)
{
#ifndef DISABLE_SERIALIZE_SAFETY_CHECKS
	// Sanity checks
	if ((*store) == NULL)
	{
		return __LINE__;
	}
	
	int fieldNameMax = getGenericSerializableStructFieldCount(*store);

	// Sanity checks
	if (fieldName < 0 || fieldName >= fieldNameMax
		|| fieldData == NULL || fieldSize <= 0 || dataType < 0)
	{
		return __LINE__;
	}

	// field already has data
	if ((*store)->fieldIndexes[fieldName] != SFT_NON_USED_UNKNOWN_VAUE)
	{
		return __LINE__;
	}

	setInt32ArrayFieldValue((*store), GSSFN_FIELD_TYPES, fieldName, dataType);
	setInt32ArrayFieldValue((*store), GSSFN_FIELD_SIZES, fieldName, fieldSize);
#endif
	appendGenericSerializableStructData(store, fieldName, fieldData, fieldSize);

	return 0;
}

int setGenericSerializableStructData(GenericSerializedStructure* store, int fieldName, char* fieldData, int fieldSize)
{
	char* dataStart = (char*)store;
	char* fieldStart = (char*)(dataStart + store->fieldIndexes[fieldName]);
	memcpy(fieldStart, fieldData, fieldSize);

	return 0;
}

int setGenericSerializableStructDataSafe(GenericSerializedStructure* store, int fieldName, SerializableFieldTypes dataType, char* fieldData, int fieldSize)
{
#ifndef DISABLE_SERIALIZE_SAFETY_CHECKS
	if (store == NULL)
	{
		return __LINE__;
	}

	int fieldNameMax = getGenericSerializableStructFieldCount(store);

	// Sanity checks
	if (fieldName < 0 || fieldName >= fieldNameMax
		|| fieldData == NULL || fieldSize <= 0 || dataType < 0)
	{
		return __LINE__;
	}

	// field already has data
	if (store->fieldIndexes[fieldName] == SFT_NON_USED_UNKNOWN_VAUE)
	{
		return __LINE__;
	}

	int dataSize = getInt32ArrayFieldValue(store, GSSFN_FIELD_SIZES, fieldName);
	if(dataSize != fieldSize)
	{
		return __LINE__;
	}

	int prevDataType = getInt32ArrayFieldValue(store, GSSFN_FIELD_TYPES, fieldName);
	if (prevDataType != dataType)
	{
		return __LINE__;
	}
#endif
	setGenericSerializableStructData(store, fieldName, fieldData, fieldSize);

	return 0;
}

void setInt32ArrayFieldValue(GenericSerializedStructure* serializedStruct, int fieldName, int index, int value)
{
	char* dataStart = (char*)serializedStruct;
	int* fieldStart = (int*)(dataStart + serializedStruct->fieldIndexes[fieldName]);
	fieldStart[index] = value;
}

void setInt32FieldValue(GenericSerializedStructure* serializedStruct, int fieldName, int value)
{
	setInt32ArrayFieldValue(serializedStruct, fieldName, 0, value);
}

int getInt32ArrayFieldValue(GenericSerializedStructure* serializedStruct, int fieldName, int index)
{
	char* dataStart = (char*)serializedStruct;
	int* fieldStart = (int*)(dataStart + serializedStruct->fieldIndexes[fieldName]);
	return fieldStart[index];
}

int getInt32FieldValue(GenericSerializedStructure* serializedStruct, int fieldName)
{
	return getInt32ArrayFieldValue(serializedStruct, fieldName, 0);
}


void getGenericSerializableStructData(GenericSerializedStructure* store, int fieldName, char** out_data)
{
	*out_data = (char*)store + store->fieldIndexes[fieldName];
}

int getGenericSerializableStructDataSafe(GenericSerializedStructure* store, int fieldName, SerializableFieldTypes dataType, char** out_data, int* out_size)
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
#endif
	// Retrieve the pointer to the data requested
	getGenericSerializableStructData(store, fieldName, out_data);
	return 0;
}