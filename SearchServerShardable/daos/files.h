#ifndef FILES_H
#define FILES_H

#include <daos_definitions.h>
#include <item_functions.h>
#include <stdio.h>
#include <stdbool.h>

int readItemFromFile(FILE* file, DaosOffset_t itemOffset, DaosSize_t itemSize, uint8_t* buffer);
int loadFileBuffer(FILE* file, DaosOffset_t offset, DaosSize_t size, uint8_t* buffer, DaosSize_t* numBytesRead);
int processItemRead(FILE* file, const uint8_t* buffer, DaosSize_t itemSize, BinaryToItem_t binaryToItem
    , void** item, int fileVersion);

int writeItemToNewFile(FILE* file, const char* directory, DaosCount_t numItemsPerFile,
    const ItemFunctions_t* itemFunctions, const void* item, DaosSize_t* itemSize);
int writeItemToExistingFile(FILE* tempFile, FILE* existingFile, DaosOffset_t existingItemOffset,
    DaosSize_t existingItemSize, const ItemFunctions_t* itemFunctions, const void* item,
    DaosSize_t* itemSize, bool* newItem);

#endif // FILES_H
