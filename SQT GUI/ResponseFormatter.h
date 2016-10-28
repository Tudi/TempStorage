#pragma once

#define BUFFSTORE_CURRENT_VERSION 1

enum ResultStoreValueTypes
{
    VT_FIRST_UNUSED_TYPE,       // 0 can be used for many things. That's why we will not use it

    VT_CAN_DO_IQ,               // Can this node perform IQ ?
    VT_CAN_DO_OQ,               // Can this node perform OQ ?
    VT_CAN_DO_OQMVM,            // Can this node perform OQMVM ?

    VT_NODE_LIST_COUNT,         // after this value a list of names will come that we will send as a vector in JSON
    VT_NODE_LIST_NAME,          // name of a node that is part of a list of names

    //you should send these 4 at the same time in the same order ( as a structure )
    VT_NODE_PERFORM_NAME,       // tell API to perform for this node
    VT_NODE_PERFORM_IQ,         // tell API to perform IQ for this node
    VT_NODE_PERFORM_OQ,         // tell API to perform OQ for this node
    VT_NODE_PERFORM_OQMVM,      // tell API to perform OQMVM for this node

    VT_ZIP_COMPRESSED_PACKET,   // the value is a whole "datastore" in a packet format. I only recon using this if data returned is larger than 64k.
    VT_SSL_ENCRYPTED,           // the value is a whole "datastore" in an encrypted format. Session variables should be included as additional packets
    VT_SESSION_ID,              // if this comunication is not API like but instead we store session variables by some session server
    VT_ERROR_ID,                // there should be a table where we define application specific error IDs
    VT_ERROR_STRING,            // based on error ID and localization we might be able to return an error string when it's possible

    VT_COUNT,                   // last in this enum that will signal the number of Value types
};

enum DSStoreErrorCodes
{
    DS_ERROR_NONE,
    DS_ERROR_COULD_NOT_STORE_VALUE,
};

#pragma pack(push, 1)
struct ValStore
{
    int     Type;
    int     Len;
    char    Value;    // only a placeholder for a dynamic length value
};
struct DataStore
{
    int         Size;       // size of this buffer that needs to get parsed. Might be the same as network packet
    char        Ver;        // version of the packet handler we should use to handle parsing. This is only used if we intend to have multiversioning. Right now this is not planned. Versioning could be handled by packet types or recursive parsing
    char        NoRealloc;  // Buffer Pools or 3rd party allocaters will pass a buffer for us to be able to handle deallocation themself
    int         AllocSize;  // Probably the same as "Size". Unless external allocator is used 
    int         Count;      // Count of values stored in this response
};

// List of value specific structures
#pragma pack(pop, 1)

/*
    Add a new value to the response.
    Value types are hardcoded
    Will return the location of the value store so later you can reuse it
*/
DataStore *DS_PackToResult( DataStore *Buf, int Type, int Len, const void *val, ValStore **StoredAt = NULL, int *Error = NULL );
//wrappers for the above function
DataStore *DS_PackToResult( DataStore *Buf, int Type, const char val ); 
DataStore *DS_PackToResult( DataStore *Buf, int Type, const short val ); 
DataStore *DS_PackToResult( DataStore *Buf, int Type, const int val ); 

/*
    Get to pointer to a specific value
*/
ValStore *DS_GetValue( DataStore *Buf, int index );
#define DS_GetStringValue( ValStorePointer ) ((char*)&ValStorePointer->Value)
#define DS_GetByteValue( ValStorePointer ) (ValStorePointer->Value)
#define DS_GetIntValue( ValStorePointer ) (*(int*)&ValStorePointer->Value)

/*
    Create a new packet from an existing packet. Packets can be recursively handled
*/
DataStore *DS_CompressData( DataStore *Buf );
/*
    Create a new packet from an existing packet. Packets can be recursively handled
*/
DataStore *DS_EncryptData( DataStore *Buf );

/*
    Do deinitialization of a response buffer. Depending on "new" or "alloc" should also reflect disposing
*/
void Dispose_DS( DataStore **Buf );