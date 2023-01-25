#ifndef FILE_TABLE_H
#define FILE_TABLE_H

#include <file_table_entry.h>
#include <daos_definitions.h>
#include <stdio.h>

int getFileTableEntry(FILE* file, DaosId_t id, DaosOffset_t* itemOffsetInFile,
    DaosSize_t* itemSize);
int getFileTableVersion(FILE* file, DaosFileVersion_t* libraryVersion,
    DaosFileVersion_t* fileVersion);
int getFileTable(FILE* file, DaosFileVersion_t* fileVersion, DaosCount_t* capacity,
    DaosCount_t* numItemsInFile, FileTableEntry_t** fileTable);

int writeInitialFileTableToFile(FILE* file, DaosCount_t numItemsPerFile);
int updateFileTableInFile(FILE* file, DaosId_t id, DaosSize_t itemSize);

#endif // FILE_TABLE_H
