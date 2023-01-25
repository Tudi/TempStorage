#ifndef BINARY_UTILS_H
#define BINARY_UTILS_H

#include <time.h>
#include <stdbool.h>
#include <stdint.h>

uint8_t* booleanToBinary(uint8_t* byteStream, bool b);
const uint8_t* binaryToBoolean(const uint8_t* byteStream, bool* b);

#define SKIP_BINARY_BOOLEAN(BYTESTREAM) ((BYTESTREAM) + sizeof(bool))
#define BOOLEAN_BINARY_SIZE ((uint32_t) sizeof(bool))

uint8_t* uint8ToBinary(uint8_t* byteStream, uint8_t u);
const uint8_t* binaryToUint8(const uint8_t* byteStream, uint8_t* u);

#define SKIP_BINARY_UINT8(BYTESTREAM) ((BYTESTREAM) + sizeof(uint8_t))
#define UINT8_BINARY_SIZE ((uint32_t) sizeof(uint8_t))

uint8_t* uint16ToBinary(uint8_t* byteStream, uint16_t u);
const uint8_t* binaryToUint16(const uint8_t* byteStream, uint16_t* u);

#define SKIP_BINARY_UINT16(BYTESTREAM) ((BYTESTREAM) + sizeof(uint16_t))
#define UINT16_BINARY_SIZE ((uint32_t) sizeof(uint16_t))

uint8_t* int32ToBinary(uint8_t* byteStream, int32_t i);
const uint8_t* binaryToInt32(const uint8_t* byteStream, int32_t* i);

#define SKIP_BINARY_INT32(BYTESTREAM) ((BYTESTREAM) + sizeof(int32_t))
#define INT32_BINARY_SIZE ((uint32_t) sizeof(int32_t))

uint8_t* int64ToBinary(uint8_t* byteStream, int64_t i);
const uint8_t* binaryToInt64(const uint8_t* byteStream, int64_t* i);

#define SKIP_BINARY_INT64(BYTESTREAM) ((BYTESTREAM) + sizeof(int64_t))
#define INT64_BINARY_SIZE ((uint32_t) sizeof(int64_t))

uint8_t* stringToBinary(uint8_t* byteStream, const char* str);
const uint8_t* binaryToString(const uint8_t* byteStream, char** str);
const uint8_t* binaryToLowerString(const uint8_t* byteStream, char** str);
const uint8_t* skipBinaryString(const uint8_t* byteStream);
uint32_t stringBinarySize(const char* str);

#endif // BINARY_UTILS_H
