#ifndef ITEM_FUNCTIONS_H
#define ITEM_FUNCTIONS_H

#include <daos_definitions.h>
#include <json_object.h>
#include <stdint.h>
#include <stdbool.h>

//
// Types
//

typedef int (*BinaryToItem_t)(const uint8_t* byteStream, void** item, int fileVersion);

typedef
struct
{
    // Persistent item

    DaosId_t (*getPersistentItemId)(const void* item);
    void (*freePersistentItem)(void* item);
    uint8_t* (*persistentItemToBinary)(uint8_t* byteStream, const void* item);
    BinaryToItem_t binaryToPersistentItem;
    uint32_t (*persistentItemBinarySize)(const void* item);
    struct json_object* (*persistentItemToJson)(const void* item);
    int (*jsonToPersistentItem)(void** item, const struct json_object* obj);
    int (*jsonToPersistentList)(void** lst, const struct json_object* obj, const char* jsonKey);
    void (*freePersistentList)(void* lst);
    uint32_t (*persistentListCount)(const void* lst);
    void* (*getItemFromPersistentList)(void* lst, uint32_t index);
    bool (*persistentItemShouldBeCached)(const void* item);
    bool (*isItemFile)(const char* filename);
    bool (*isDestinationServerCorrect)(const void* item, DaosCount_t numServers, const DaosId_t serverId);

    // Cached item

    DaosId_t (*getCachedItemId)(const void* item);
    void* (*generateCachedItem)(const void* item);
    void (*freeCachedList)(void** items, uint32_t numItems);
    void (*freeCachedItem)(void* item);
    BinaryToItem_t binaryToCachedItem;
    DaosCount_t (*generateCacheIndex)(DaosId_t id, DaosCount_t numServers);
} ItemFunctions_t;

#define ITEM_FUNCTIONS_ZERO_VALUE ((ItemFunctions_t) { \
    .getPersistentItemId = NULL, .freePersistentItem = NULL, .persistentItemToBinary = NULL, \
    .binaryToPersistentItem = NULL, .persistentItemBinarySize = NULL, \
    .persistentItemToJson = NULL, .jsonToPersistentItem = NULL, \
    .jsonToPersistentList = NULL, .freePersistentList = NULL, \
    .persistentListCount = NULL, .getItemFromPersistentList = NULL, \
    .persistentItemShouldBeCached = NULL, .isItemFile = NULL, .isDestinationServerCorrect = NULL, \
    .getCachedItemId = NULL, .generateCachedItem = NULL, .freeCachedList = NULL, \
    .freeCachedItem = NULL, .binaryToCachedItem = NULL, .generateCacheIndex = NULL })

#endif // ITEM_FUNCTIONS_H
