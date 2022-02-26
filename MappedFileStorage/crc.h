#ifndef _CRC_H_
#define _CRC_H_

#include <inttypes.h>

int crc32(int crc, const void* buf, int size);
uint64_t crc64(uint64_t crc, const void* buffer, uint64_t size);

#endif 