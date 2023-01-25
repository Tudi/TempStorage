#ifndef DAOS_ITEM_FUNCTIONS_H
#define DAOS_ITEM_FUNCTIONS_H

#include <daos_definitions.h>
#include <stdint.h>

typedef int (*DaosBinaryToItem_t)(const uint8_t* byteStream, void** item);

typedef
struct
{
    DaosId_t (*getPersistentItemId)(const void* item);
    void (*freeCachedList)(void** items, uint32_t numItems);
    DaosBinaryToItem_t binaryToCachedItem;
    uint8_t* (*persistentItemToBinary)(uint8_t* byteStream, const void* item);
    DaosBinaryToItem_t binaryToPersistentItem;
    uint32_t (*persistentItemBinarySize)(const void* item);
} DaosItemFunctions_t;

#endif // DAOS_ITEM_FUNCTIONS_H
