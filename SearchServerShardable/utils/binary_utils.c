#include <binary_utils.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

//
// bool
//

uint8_t* booleanToBinary(uint8_t* byteStream, bool b)
{
    memcpy(byteStream, &b, sizeof(b));
    return byteStream + sizeof(b);
}

const uint8_t* binaryToBoolean(const uint8_t* byteStream, bool* b)
{
    memcpy(b, byteStream, sizeof(*b));
    return byteStream + sizeof(*b);
}

//
// uint8_t
//

uint8_t* uint8ToBinary(uint8_t* byteStream, uint8_t u)
{
    memcpy(byteStream, &u, sizeof(u));
    return byteStream + sizeof(u);
}

const uint8_t* binaryToUint8(const uint8_t* byteStream, uint8_t* u)
{
    memcpy(u, byteStream, sizeof(*u));
    return byteStream + sizeof(*u);
}

//
// uint16_t
//

uint8_t* uint16ToBinary(uint8_t* byteStream, uint16_t u)
{
    memcpy(byteStream, &u, sizeof(u));
    return byteStream + sizeof(u);
}

const uint8_t* binaryToUint16(const uint8_t* byteStream, uint16_t* u)
{
    memcpy(u, byteStream, sizeof(*u));
    return byteStream + sizeof(*u);
}

//
// int32_t
//

uint8_t* int32ToBinary(uint8_t* byteStream, int32_t i)
{
    memcpy(byteStream, &i, sizeof(i));
    return byteStream + sizeof(i);
}

const uint8_t* binaryToInt32(const uint8_t* byteStream, int32_t* i)
{
    memcpy(i, byteStream, sizeof(*i));
    return byteStream + sizeof(*i);
}

//
// int64_t
//

uint8_t* int64ToBinary(uint8_t* byteStream, int64_t i)
{
    memcpy(byteStream, &i, sizeof(i));
    return byteStream + sizeof(i);
}

const uint8_t* binaryToInt64(const uint8_t* byteStream, int64_t* i)
{
    memcpy(i, byteStream, sizeof(*i));
    return byteStream + sizeof(*i);
}

//
// char*
//

uint8_t* stringToBinary(uint8_t* byteStream, const char* str)
{
    size_t strLength = strlen(str) + 1;

    if(strLength > UINT16_MAX) { return NULL; }

    uint16_t sizeField = (uint16_t) strLength;

    memcpy(byteStream, &sizeField, sizeof(sizeField));
    memcpy(byteStream + sizeof(sizeField), str, strLength * sizeof(char));

    return byteStream + sizeof(sizeField) + strLength * sizeof(char);
}

const uint8_t* binaryToString(const uint8_t* byteStream, char** str)
{
    uint16_t strLength = 0;

    memcpy(&strLength, byteStream, sizeof(strLength));
    if(strLength == 0) { return NULL; }

    char* auxStr = malloc(strLength * sizeof(char));
    if(auxStr == NULL) { return NULL; }

    const uint8_t* auxByteStream = byteStream + sizeof(strLength);
    if(auxByteStream[strLength - 1] != '\0')
    {
        free(auxStr);
        return NULL;
    }

    memcpy(auxStr, auxByteStream, strLength);
    *str = auxStr;

    return auxByteStream + strLength;
}

const uint8_t* binaryToLowerString(const uint8_t* byteStream, char** str)
{
    uint16_t strLength = 0;

    memcpy(&strLength, byteStream, sizeof(strLength));
    if(strLength == 0) { return NULL; }

    char* auxStr = malloc(strLength * sizeof(char));
    if(auxStr == NULL) { return NULL; }

    const uint8_t* auxByteStream = byteStream + sizeof(strLength);
    if(auxByteStream[strLength - 1] != '\0')
    {
        free(auxStr);
        return NULL;
    }

    memcpy(auxStr, auxByteStream, strLength);

    for(uint16_t i = 0; i < strLength; ++i) {
        auxStr[i] = tolower(auxStr[i]);
    }

    *str = auxStr;

    return auxByteStream + strLength;
}

const uint8_t* skipBinaryString(const uint8_t* byteStream)
{
    uint16_t strLength = 0;

    memcpy(&strLength, byteStream, sizeof(strLength));
    if(strLength == 0) { return NULL; }

    return byteStream + sizeof(strLength) + strLength;
}

uint32_t stringBinarySize(const char* str)
{
    return sizeof(uint16_t) + (strlen(str) + 1) * sizeof(char);
}
