#ifndef _SERIALIZE_TYPES_H_
#define _SERIALIZE_TYPES_H_

/*
* Designed specifically for static storage data, or network data syncronization
* Advantages :
* - directly serializable into any storage format : HDD, RAM, Network
* - does not need full deserialization for accessing specific fields(columnar database)
* - can copy / duplicate / move the content without the need of updates
* Disadvantages :
* - can't increase the size of dynamic size fields(strings). You can append to the end, which leaves unused space gaps
*		You need to rebuild the structure before store if you wish to avoid gaps
* - larger size than simple struct
* - slower than a simple struct
*/

// in case you really want to save data space and need the speed difference : define this
// for small projects you probably want to enable it
//#define DISABLE_SERIALIZE_SAFETY_CHECKS

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
	SFT_8BYTE_FIXED_ARRAY,
	SFT_8BYTE_FLOAT_ARRAY,
	// project specific types
	SFT_TIME_ID_KVECT,
	SFT_ID_ID_KVECT,
	SFT_ID_KVECT,
	SFT_ID_STR_KVECT,
	SFT_POSITION_KVECT,
	SFT_EDUCATION_KVECT,
	SFT_STR_KVECT,
	SFT_EMAIL_KVECT,
	SFT_SOCIAL_URL_KVECT,
	SFT_PHONENUMBER_KVECT,
	SFT_TAG_KVECT,
	SFT_MAX_USED_VALUE_FOR_BOUNDS_CHECK
}SerializableFieldTypes;

// These need to be included into any structure you wish to serialize
typedef enum GenericSerializedStructureFieldNames
{
	GSSFN_DATA_NOT_PRESENT_IN_STRUCTURE = -1,
	GSSFN_FIELD_INDEXES, // must have index 0 generic for any structure
#ifndef DISABLE_SERIALIZE_SAFETY_CHECKS
	GSSFN_FIELD_TYPES, // generic for any structure
	GSSFN_FIELD_SIZES, // another safety check
	GSSFN_FIELD_MAX_INDEX, // another safety check
	GSSFN_FIELD_VERSION, // might help avoid loading incorrect version data
	GSSFN_FIELD_CRC, // data integrity check
#endif
	GSSFN_MAX_FIELD_NAME
}GenericSerializedStructureFieldNames;

// generic header to store the inplace structure
typedef struct GenericSerializedStructure
{
	// bytes needed to read this whole structure. Used by fread or network read. 
	// Includes self size
	int size;
	// data will be written here. Do not change size.
	int fieldIndexes[0];
}GenericSerializedStructure;

void* createGenericSerializableStruct(int maxFieldNames, int version);

#ifndef DISABLE_SERIALIZE_SAFETY_CHECKS
void UpdateCRCGenericSerializableStruct(GenericSerializedStructure* store);
int CheckCRCGenericSerializableStruct(GenericSerializedStructure* store);
int getGenericSerializableStructData(GenericSerializedStructure* store, int fieldName, SerializableFieldTypes dataType, char** out_data, int* out_size);
int getInt32FieldValue(GenericSerializedStructure* serializedStruct, int fieldName);
int getInt32ArrayFieldValue(GenericSerializedStructure* serializedStruct, int fieldName, int index);
// adds a footer and header of 4 bytes before and after a field to check for data out of bounds writes
#define ADD_BOUNCE_CHECK
#else
#define UpdateCRCGenericSerializableStruct(x)
#define CheckCRCGenericSerializableStruct(x) 0
#define getGenericSerializableStructData(store,fieldName,dataType,out_data,out_size) *out_data = ((char*)store + store->fieldIndexes[fieldName]);
#define getInt32FieldValue(serializedStruct,fieldName) (*(int32_t*)((char*)store + store->fieldIndexes[fieldName])
#define getInt32ArrayFieldValue(serializedStruct, fieldName, index) (*(int32_t*)((char*)store + store->fieldIndexes[fieldName] + index * sizeof(int32_t)))
#endif

int appendGenericSerializableStructData(GenericSerializedStructure** store, int fieldName, SerializableFieldTypes dataType, void* fieldData, int fieldSize);
int setGenericSerializableStructData(GenericSerializedStructure* store, int fieldName, SerializableFieldTypes dataType, void* fieldData, int fieldSize);

void setInt32FieldValue(GenericSerializedStructure* serializedStruct, int fieldName, int value);

void setInt32ArrayFieldValue(GenericSerializedStructure* serializedStruct, int fieldName, int index, int value);
#endif