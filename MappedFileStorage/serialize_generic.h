#ifndef _SERIALIZE_TYPES_H_
#define _SERIALIZE_TYPES_H_

/*
* Designed specifically for static storage data, or network data syncronization
* Advantages :
* - directly serializable into any storage format : HDD, RAM, Network
* Disadvantages :
* - can only append dynamic size fields. You need to rebuild the structure before store
* - slightly larger size than simple struct
*/
typedef enum SerializableFieldTypes
{
	SFT_NON_USED_UNKNOWN_VAUE = -1,
	SFT_1BYTE,
	SFT_2BYTE,
	SFT_4BYTE_FIXED,
	SFT_4BYTE_FLOAT,
	SFT_8BYTE_FIXED,
	SFT_8BYTE_FLOAT,
	SFT_NULL_STRING, // null terminated string
	SFT_1BYTE_ARRAY, // array of bytes
	SFT_4BYTE_FIXED_ARRAY,
	SFT_4BYTE_FLOAT_ARRAY,
	SFT_MAX_USED_VALUE_FOR_BOUNDS_CHECK
}SerializableFieldTypes;

typedef enum GenericSerializedStructureFieldNames
{
	GSSFN_DATA_NOT_PRESENT_IN_STRUCTURE = -1,
	GSSFN_FIELD_INDEXES, // must have index 0 generic for any structure
	GSSFN_FIELD_TYPES, // generic for any structure
	GSSFN_FIELD_SIZES, // another safety check
	GSSFN_FIELD_VERSION, // might help avoid loading incorrect version data
	GSSFN_MAX_FIELD_NAME
}GenericSerializedStructureFieldNames;

typedef struct GenericSerializedStructure
{
	// bytes needed to read to read this whole structure. Used by fread or network read
	int size;
	// data will be written here. Do not change size.
	int fieldIndexes[0];
}GenericSerializedStructure;

void* createGenericSerializableStruct(int maxFieldNames, int version);

int appendGenericSerializableStructData(GenericSerializedStructure** store, int fieldName, SerializableFieldTypes dataType, void* fieldData, int fieldSize);
//int setGenericSerializableStructData(GenericSerializedStructure* store, int fieldName, SerializableFieldTypes dataType, char* fieldData, int fieldSize);
int getGenericSerializableStructData(GenericSerializedStructure* store, int fieldName, SerializableFieldTypes dataType, char** out_data, int* out_size);

void setInt32FieldValue(GenericSerializedStructure* serializedStruct, int fieldName, int value);
int getInt32FieldValue(GenericSerializedStructure* serializedStruct, int fieldName);

void setInt32ArrayFieldValue(GenericSerializedStructure* serializedStruct, int fieldName, int index, int value);
int getInt32ArrayFieldValue(GenericSerializedStructure* serializedStruct, int fieldName, int index);
#endif