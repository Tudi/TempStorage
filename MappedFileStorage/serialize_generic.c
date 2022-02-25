#include "serialize_generic.h"
#include <stddef.h>
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
			(*store)->fieldIndexes[i] = -1;
		}
		(*store)->fieldIndexes[fieldName] = (*store)->size;
	}
	else if (fieldName == GSSFN_FIELD_TYPES || fieldName == GSSFN_FIELD_SIZES)
	{
		for (size_t i = 0; i < fieldSize / sizeof((*store)->fieldIndexes[0]); i++)
		{
			setInt32ArrayFieldValue(*store, fieldName, i, -1);
		}
	}
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
	AppendUniqueFields(&ret, GSSFN_FIELD_TYPES, maxFieldNames * sizeof(ret->fieldIndexes[0]));
	AppendUniqueFields(&ret, GSSFN_FIELD_SIZES, maxFieldNames * sizeof(ret->fieldIndexes[0]));
	appendGenericSerializableStructData(&ret, GSSFN_FIELD_VERSION, SFT_4BYTE_FIXED, &version, sizeof(version));
	return ret;
}

static int getGenericSerializableStructFieldCount(GenericSerializedStructure* store)
{
	return ((store)->fieldIndexes[GSSFN_FIELD_TYPES] - (store)->fieldIndexes[GSSFN_FIELD_INDEXES]) / sizeof((store)->fieldIndexes[0]);
}

int appendGenericSerializableStructData(GenericSerializedStructure** store, int fieldName, SerializableFieldTypes dataType, void* fieldData, int fieldSize)
{
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
	if ((*store)->fieldIndexes[fieldName] != -1)
	{
		return __LINE__;
	}

	void *newStore = realloc((*store), (*store)->size + fieldSize);
	if (newStore == NULL)
	{
		return __LINE__;
	}
	*store = (GenericSerializedStructure * )newStore;

	setInt32ArrayFieldValue((*store), GSSFN_FIELD_INDEXES, fieldName, (*store)->size);
	setInt32ArrayFieldValue((*store), GSSFN_FIELD_TYPES, fieldName, dataType);
	setInt32ArrayFieldValue((*store), GSSFN_FIELD_SIZES, fieldName, fieldSize);
	memcpy((char*)(*store) + (*store)->size, fieldData, fieldSize);
	(*store)->size += fieldSize;

	return 0;
}

int setGenericSerializableStructData(GenericSerializedStructure* store, int fieldName, SerializableFieldTypes dataType, char* fieldData, int fieldSize)
{
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
	if (store->fieldIndexes[fieldName] == -1)
	{
		return __LINE__;
	}

	int dataSize = getInt32ArrayFieldValue(store, GSSFN_FIELD_SIZES, fieldName);
	if(dataSize != fieldSize)
	{
		return __LINE__;
	}

	char* dataStart = (char*)store;
	char* fieldStart = (int*)(dataStart + store->fieldIndexes[fieldName]);
	memcpy(fieldStart, fieldData, fieldSize);
	setInt32ArrayFieldValue(store, GSSFN_FIELD_TYPES, fieldName, dataType);

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

int getGenericSerializableStructData(GenericSerializedStructure* store, int fieldName, SerializableFieldTypes dataType, char** out_data, int* out_size)
{
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
	if (store->fieldIndexes[fieldName] == -1)
	{
		*out_data = NULL;
		*out_size = 0;
		return 0;
	}

	*out_size = getInt32ArrayFieldValue(store, GSSFN_FIELD_SIZES, fieldName);

	// Retrieve the pointer to the data requested
	*out_data = (char*)store + store->fieldIndexes[fieldName];

	// Some fields might require to know the size of the data in the field
	// For example you could "assert" if you are expecting 4 bytes for an 'int' but received different
/*	if (fieldName + 1 == fieldNameMax)
	{
		*out_size = store->size - store->fieldIndexes[fieldName];
	}
	else
	{
		*out_size = store->fieldIndexes[fieldName + 1] - store->fieldIndexes[fieldName];
	}*/
	return 0;
}