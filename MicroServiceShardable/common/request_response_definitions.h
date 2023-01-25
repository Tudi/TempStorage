#ifndef PROFILE_REQUEST_RESPONSE_H
#define PROFILE_REQUEST_RESPONSE_H

#include <stdint.h>

// Requests

typedef enum SimilarityScorePacketTypes
{
    SSPT_NOT_USED_PACKET_TYPE = 0, // avoid 0 values to detect uninitialized data
    SSPT_GET_SCORE = 1, // based on request, bundle up a ferrari-c compatible similarity score array
    SSPT_SAVE_SCORE    = 2, // save similarity score file to local drive
    SSPT_END_CONNECTION = 3, // terminate a persistent connection
    SSPT_PING = 4, // used for keepalive a persistent connection
    SSPT_RESPONSE_CODE = 5, // only a response code is sent. Ex : error
    SSPT_GET_SCORE_ASCII = 6, // not yet implemented
    SSPT_REPLY_SCORE = 7, // server replies to SSPT_GET_SCORE
    SSPT_SAVE_ID_ARRAY = 8, // save ID array file
    SSPT_MAX_REQUEST_CODE // always the last value to be able to detect invalid values
}SimilarityScorePacketTypes;

#pragma pack(push, 1) // needs to be mapable to file content no matter to compile settings 

static const char* g_packet_type_str[SSPT_MAX_REQUEST_CODE + 1] __attribute__((unused)) =
{
    [SSPT_NOT_USED_PACKET_TYPE] = "UNINITIALIZED_PACKET_DATA",
    [SSPT_GET_SCORE] = "CLIENT_GET_SIMILARITY_SCORE",
    [SSPT_SAVE_SCORE] = "CLIENT_SAVE_SIMILARITY_SCORE_FILE",
    [SSPT_PING] = "CLIENT_KEEPALIVE_PERSISTENT_CONNECTION",
    [SSPT_RESPONSE_CODE] = "CLIENT_SERVER_RESPONSE_CODE",
    [SSPT_GET_SCORE_ASCII] = "CLIENT_GET_SIMILARITY_SCORE_ASCII",
    [SSPT_REPLY_SCORE] = "SERVERT_REPLY_SIMILARITY_SCORE",
};

// packet fragment present in every packet
typedef struct SSPacketHeader
{
    uint32_t size; // buffer needed to store the payload part of the packet. Size does not include header size
    uint8_t type;  // packet type. Example : save score, get scores
}SSPacketHeader;

typedef struct SSPacketDataSendScore
{
    uint8_t   type;        // type of the score file : company = 1, industry, title, profile
    uint32_t  id;        // ID of the "company" this score file belongs to
    uint8_t   fileData[0]; // similarity score file. See file structure in score_manager. This is the file as is from ML
}SSPacketDataSendScore;

// Packet sent by ML to save a score file
typedef struct SSPacketSendScore
{
    SSPacketHeader         header;
    SSPacketDataSendScore  data;
}SSPacketSendScore;

// packet data when expecting only a reply code
typedef struct SSPacketDataResponseCode
{
    uint32_t responseCode; // see error code list for values "app_errors.h"
}SSPacketDataResponseCode;

// full packet containing header and data
typedef struct SSPacketResponseCode
{
    SSPacketHeader            header;
    SSPacketDataResponseCode  data;
}SSPacketResponseCode;

typedef struct SSPacketDataScoreRequestBlock
{
    uint32_t        count;   // number of ids in the list
    uint8_t         type;    // type of Ids. Ex : title, idustry...
    uint32_t        id[0];   // placeholder signaling a dynamic unknown size
}SSPacketDataScoreRequestBlock;

typedef struct SSPacketScoreRequest
{
    SSPacketHeader                   header;
    SSPacketDataScoreRequestBlock    data[0]; // placeholder signaling a dynamic unknown size
}SSPacketScoreRequest;

typedef struct SSPacketScoreReply
{
    SSPacketHeader  header;
    char            data[0];     // placeholder signaling a dynamic unknown size
}SSPacketScoreReply;

typedef struct SSPacketEndConnection
{
    SSPacketHeader    header;
    uint8_t           data[0]; // no data in the packet
}SSPacketEndConnection;

typedef struct SSPacketPing
{
    SSPacketHeader    header;
    uint8_t           data[0]; // no data in the packet
}SSPacketPing;

#pragma pack(pop)

// Limits

#define PACKET_MAX_SIZE             (130 * 1024 * 1024) // ex: 500 files, each with 100k values .. 20GB data
#define MAX_ID_ARRAY_SIZE            500 // Initially it was 5
#define MAX_IDS_in_SCORE_FILE       1000000 // around 100k is normal
#define MAX_STRING_LEN				0x0FFFFF // codacy complains about unlimited strings. This values should be used when max len is unknown
#define MAX_PATH_LEN    			16000 // used when handling files

#ifndef TRUE
    #define TRUE    1
#endif
#ifndef FALSE
    #define FALSE 0
#endif

#endif // PROFILE_REQUEST_RESPONSE_H
