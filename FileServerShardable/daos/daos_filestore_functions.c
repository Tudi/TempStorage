#include <daos_filestore_functions.h>
#include <request_response_definitions.h>
#include <stdlib.h>
#include <string.h>

//
// Prototypes
//

static DaosId_t getFileStoreId(const void* item);
static uint32_t fileStoreItemBinarySize(const void* item);

//
// Variables
//

static DaosItemFunctions_t daosFileFunctions = 
{
    .getPersistentItemId      = getFileStoreId,
    .freeCachedList           = NULL,
    .binaryToCachedItem       = NULL,
    .persistentItemToBinary   = NULL,
    .binaryToPersistentItem   = NULL,
    .persistentItemBinarySize = fileStoreItemBinarySize
};

//
// External interface
//

DaosItemFunctions_t* getDaosFileStoreFunctions()
{
    return &daosFileFunctions;
}

//
// Local functions
//

static DaosId_t getFileStoreId(const void* item)
{
    const struct FSPacketSaveFile* fsb = (const FSPacketSaveFile*) item;

    return fsb->data.id;
}

static uint32_t fileStoreItemBinarySize(const void* item)
{
    const struct FSPacketSaveFile* file = (const struct FSPacketSaveFile*) item;
    return file->header.size + sizeof(FSPacketHeader);
}
