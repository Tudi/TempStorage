#ifndef DAOS_H
#define DAOS_H

#include <file_table_entry.h>
#include <daos_definitions.h>
#include <daos_item_functions.h>
#include <stdbool.h>

// Types

typedef
struct
{
    void* d;
} Daos_t;

// Constants

#define Daos_NULL ((Daos_t) { .d = NULL })

// Functions

Daos_t daos_init(const char* directory, uint32_t dirDepth, uint32_t dirCount, const char* filePrefix, const char* fileExtension,
    uint8_t numIdDigits, DaosCount_t numItemsPerFile, DaosItemFunctions_t* itemFunctions);
void daos_free(Daos_t daos);
bool daos_isNull(Daos_t daos);

int daos_getItem(Daos_t daos, DaosId_t id, void** buffer, void** item, uint32_t* itemDataSize);
int daos_saveItem(Daos_t daos, const void* item, uint32_t itemSize);

int daos_getFileVersion(Daos_t daos, const char* filename, DaosFileVersion_t* libraryVersion,
    DaosFileVersion_t* fileVersion);
int daos_getFileTable(Daos_t daos, const char* filename, DaosFileVersion_t* fileVersion,
    DaosCount_t* capacity, DaosCount_t* numItemsInFile, FileTableEntry_t** fileTable);

#endif // DAOS_H
