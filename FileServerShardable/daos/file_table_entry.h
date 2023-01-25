#ifndef FILE_TABLE_TYPES_H
#define FILE_TABLE_TYPES_H

#include <daos_definitions.h>

typedef
struct
{
    DaosOffset_t offset;
    DaosSize_t size;
} FileTableEntry_t;

#endif // FILE_TABLE_TYPES_H
