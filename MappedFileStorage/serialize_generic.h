#ifndef _SERIALIZE_TYPES_H_
#define _SERIALIZE_TYPES_H_

/*
* Designed specifically for static storage data, or network data syncronization
* Advantages :
* - directly serializable into any storage format : HDD, RAM, Network
* - does not need full deserialization for accessing specific fields(columnar database)
* - can copy / duplicate / move the content without the need of updates
* - support for adding / removing fields of a structure. Ex : db file entry
* Disadvantages :
* - larger size than simple struct
* - slower than a simple struct
* - does not support field data size change( ex: strings )
*/

#define MIN_ALLOC_SIZE_FIELD_GSS (65*1024)	// minimum amount of bytes to allocate to avoid to many allocations
											// adjust this based on your use case
#define MIN_ALLOC_SIZE_GSS	(128*MIN_ALLOC_SIZE_FIELD_GSS)	

// enable safety checks if data size/speed is not an issue
//#define ENABLE_GG_SAFETY_CHECKS
#define ENABLE_GSS_ASSERTS

#pragma pack(push, 1) // needs to be mapable to file content no matter of compile settings
typedef struct GenericSerializedStructureFieldDescriptor
{
	unsigned int	offset; // byte offset since the beggining of data block. Can be used to derive field size also
	unsigned short	type;	// type id. Should be unique project wide
}GenericSerializedStructureFieldDescriptor;

typedef struct GenericSerializedStructureSafetyFields
{
	unsigned short		version;	// byte offset since the beggining of data block. Can be used to derive field size also
	unsigned short		type;		// type id of the block
	unsigned long long	crc;		// data integrity check
}GenericSerializedStructureSafetyFields;

// generic header to store the inplace structure
typedef struct GenericSerializedStructure
{	
	unsigned int	size;								 // bytes needed to read this whole structure. Used by fread or network read. 
														 // Includes self size
	unsigned short	fieldCount;							 // number of fields in the 'fields' array
#ifdef ENABLE_GG_SAFETY_CHECKS
	GenericSerializedStructureSafetyFields desc;		 // block type (structure type)
#endif
	GenericSerializedStructureFieldDescriptor fields[0]; // data will be written here. Do not change size.
}GenericSerializedStructure;
#pragma pack(pop)

/// <summary>
/// Create empty storage and initialize it
/// </summary>
/// <param name="maxFields">Max number of fields you are allowed to write to it</param>
/// <param name="in_out_allocated">reserve memory for fields that will be written later</param>
/// <returns></returns>
void* create_GSS(int maxFields, unsigned int*in_out_allocated);

/// <summary>
/// When you know the size of the data you want to write. Example and 'int'
/// </summary>
/// <param name="store"></param>
/// <param name="in_out_allocated"></param>
/// <param name="TFIndex"></param>
/// <param name="TFID"></param>
/// <param name="data"></param>
/// <param name="size"></param>
/// <returns></returns>
int setData_GSS(GenericSerializedStructure** store, unsigned int* in_out_allocated,
	unsigned short TFIndex, unsigned short TFID, void* data, unsigned int size);

/// <summary>
/// When you wish to add data that the size is not clear. Ex you want to serialize a structure
/// </summary>
/// <param name="store"></param>
/// <param name="in_out_allocated"></param>
/// <param name="TFIndex"></param>
/// <param name="TFID"></param>
/// <param name="data"></param>
/// <param name="sizeReserve"></param>
/// <returns></returns>
int startDataWrite_GSS(GenericSerializedStructure** store, unsigned int* in_out_allocated,
	unsigned short TFIndex, unsigned short TFID, unsigned char** data, unsigned int sizeReserve);
void endDataWrite_GSS(GenericSerializedStructure* store, unsigned int size);

/// <summary>
/// Get the pointer to the data for a field
/// </summary>
/// <param name="store"></param>
/// <param name="TFIndex"></param>
/// <param name="TFID"></param>
/// <param name="data"></param>
/// <returns></returns>
int getData_GSS(const GenericSerializedStructure* store,
	unsigned short TFIndex, unsigned short TFID, const unsigned char** data);

/// <summary>
/// Get the size of the data of a field. Ex string
/// </summary>
/// <param name="store"></param>
/// <param name="TFIndex"></param>
/// <param name="TFID"></param>
/// <returns></returns>
int getSize_GSS(const GenericSerializedStructure* store,
	unsigned short TFIndex, unsigned short TFID);

#define serializeBackWardCompatible(store, bytesAllocated, FieldName, FieldId, SerializeFunc, Field) \
{ \
	unsigned char *writeToLocationIn = NULL; \
	if( startDataWrite_GSS(&store, &bytesAllocated, FieldName, FieldId, &writeToLocationIn, MIN_ALLOC_SIZE_FIELD_GSS) == 0 ) \
	{ \
		unsigned char *writeToLocationOut = SerializeFunc(writeToLocationIn, Field); \
		size_t WriteSize = writeToLocationOut - writeToLocationIn; \
		endDataWrite_GSS(store, WriteSize); \
	}\
}

#define deSerializeBackWardCompatible(store, FieldName, FieldId, deSerializeFunc, Field) \
{ \
	unsigned char *srcDataLocation = NULL; \
	getData_GSS(store, FieldName, FieldId, &srcDataLocation); \
	deSerializeFunc(srcDataLocation, &Field); \
}

void UpdateCRC_GSS(GenericSerializedStructure* store);
int CheckCRC_GSS(GenericSerializedStructure* store);

#endif