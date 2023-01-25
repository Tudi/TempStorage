#ifndef PROFILE_REQUEST_RESPONSE_H
#define PROFILE_REQUEST_RESPONSE_H

#include <stdint.h>

typedef enum FileServerPacketTypes
{
    FSPT_NOT_USED_PACKET_TYPE = 0, // avoid 0 values to detect uninitialized data
    FSPT_GET_FILE = 1, // return the file content that was previously saved
    FSPT_SAVE_FILE = 2, // save file to local drive
    FSPT_END_CONNECTION = 3, // terminate a persistent connection
    FSPT_PING = 4, // used for keepalive a persistent connection
    FSPT_RESPONSE_CODE = 5, // response code(mostly used in case of an error)
    FSPT_MAX_REQUEST_CODE // always the last value to be able to detect invalid values
}FileServerPacketTypes;

#pragma pack(push, 1) // needs to be mapable to file content no matter to compile settings 

static const char* g_packet_type_str[FSPT_MAX_REQUEST_CODE + 1] __attribute__((unused)) =
{
    [FSPT_NOT_USED_PACKET_TYPE] = "UNINITIALIZED_PACKET_DATA",
    [FSPT_GET_FILE] = "CLIENT_GET_FILE",
    [FSPT_SAVE_FILE] = "CLIENT_SAVE_FILE",
    [FSPT_END_CONNECTION] = "CLIENT_END_PERSISTENT_CONNECTION",
    [FSPT_PING] = "CLIENT_KEEPALIVE_PERSISTENT_CONNECTION",
    [FSPT_RESPONSE_CODE] = "CLIENT_SERVER_RESPONSE_CODE",
};

// packet fragment present in every packet
typedef struct FSPacketHeader
{
    uint32_t size; // buffer needed to store the payload part of the packet. Size does not include header size
    uint8_t type;  // packet type. Example : save score, get scores
}FSPacketHeader;

typedef struct FSPacketDataSave
{
    uint8_t   type;        // type of the score file : company, industry, title, profile ....
    uint32_t  id;          // ID of the "company" this score file belongs to
    uint8_t   fileData[0]; // placeholder file content. Unknown size
}FSPacketDataSave;

// Full-Packet sent by client to save a file
typedef struct FSPacketSaveFile
{
    FSPacketHeader   header;
    FSPacketDataSave data;
}FSPacketSaveFile;

typedef struct FSPacketDataGet
{
    uint8_t   type;        // type of the score file : company, industry, title, profile ....
    uint32_t  id;          // ID of the "company" this score file belongs to
    uint8_t   fileData[0]; // placeholder file content. Unknown size
}FSPacketDataGet;

// Full-Packet sent by client to get a file
typedef struct FSPacketGetFile
{
    FSPacketHeader   header;
    FSPacketDataGet  data;
}FSPacketGetFile;

// packet data when expecting only a reply code
typedef struct FSPacketDataResponseCode
{
    uint32_t responseCode; // see error code list for values "app_errors.h"
    char     responseStr[0]; // dynamic size. At least 1 byte for 0 terminated string
}FSPacketDataResponseCode;

// full packet containing header and data
typedef struct FSPacketResponseCode
{
    FSPacketHeader            header;
    FSPacketDataResponseCode  data;
}FSPacketResponseCode;

typedef struct FSPacketEndConnection
{
    FSPacketHeader    header;
    uint8_t           data[0]; // no data in the packet
}FSPacketEndConnection;

typedef struct FSPacketPing
{
    FSPacketHeader    header;
    uint8_t           data[0]; // no data in the packet
}FSPacketPing;

#pragma pack(pop)

// Limits

#define PACKET_MAX_SIZE             (4 * 1024 * 1024)
#define MAX_STRING_LEN				0x0FFFFF // codacy complains about unlimited strings. This values should be used when max len is unknown

#endif // PROFILE_REQUEST_RESPONSE_H
