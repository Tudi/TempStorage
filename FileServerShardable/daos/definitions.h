#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define RECORD_HEADER_SIZE  3
#define RECORD_TRAILER_SIZE 3
#define EMPTY_RECORD_SIZE   (RECORD_HEADER_SIZE + RECORD_TRAILER_SIZE)

#define RECORD_HEADER_BINARY  0xcc, 0xaa, 0x99
#define RECORD_TRAILER_BINARY 0x99, 0xaa, 0xcc

#define BINARY_ITEM_AND_EXTRAS_MAX_SIZE (64 * 1024 * 1024)
#define BINARY_ITEM_MAX_SIZE            (BINARY_ITEM_AND_EXTRAS_MAX_SIZE - EMPTY_RECORD_SIZE)

#define DAOS_FILE_BUFFER_SIZE (4 * BINARY_ITEM_AND_EXTRAS_MAX_SIZE)

#endif // DEFINITIONS_H
