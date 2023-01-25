#ifndef DAOS_H
#define DAOS_H

#include <file_table_entry.h>
#include <daos_definitions.h>
#include <item_functions.h>
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

Daos_t daos_init(DaosFileVersion_t version, const char* directory, const char* filePrefix, const char* fileExtension,
    uint8_t numIdDigits, DaosCount_t numItemsPerFile, const ItemFunctions_t* itemFunctions);
void daos_free(Daos_t daos);
bool daos_isNull(Daos_t daos);

int daos_getItem(Daos_t daos, DaosId_t id, void** item);
int daos_saveItem(Daos_t daos, const void* item, bool* isNewItem);

int daos_getFileVersion(Daos_t daos, const char* fullpathFilename, DaosFileVersion_t* libraryVersion,
    DaosFileVersion_t* fileVersion);
int daos_getFileTable(Daos_t daos, const char* fullpathFilename, DaosFileVersion_t* fileVersion,
    DaosCount_t* capacity, DaosCount_t* numItemsInFile, FileTableEntry_t** fileTable);

int daos_loadAllCachedItemsFromFile(Daos_t daos, const char* fullpathFilename, void*** items,
    DaosCount_t* numItems);

#endif // DAOS_H
