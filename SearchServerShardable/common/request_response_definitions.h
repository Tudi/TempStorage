#ifndef PROFILE_REQUEST_RESPONSE_H
#define PROFILE_REQUEST_RESPONSE_H

#include <stdint.h>

// Requests

#define GET_PROFILE             1
#define SAVE_PROFILE            2
#define START_SEARCH            3
#define GET_SEARCH_STATUS       4
#define GET_SEARCH_RESULTS      5
#define GET_COMPANY             6
#define SAVE_COMPANY            7
#define END_SEARCH              8
#define GET_MULTIPLE_PROFILES   9
#define END_CONNECTION          10
#define PING                    11
#define GET_SYSTEM_INFO         12
#define SET_LOG_LEVEL           13
#define SAVE_MULTIPLE_PROFILES  14 
#define SAVE_MULTIPLE_COMPANIES 15 

#define MAX_REQUEST_CODE        SAVE_MULTIPLE_COMPANIES

// Response codes

#define NULL_RESPONSE    255
#define SUCCESS_RESPONSE 0

#define ERROR_GENERIC_RESPONSE           1
#define ERROR_MALFORMED_REQUEST_RESPONSE 2
#define ERROR_JSON_LIBRARY               3
#define ERROR_NONEXISTENT_ITEM_RESPONSE  4
#define ERROR_MEMORY_ALLOCATION_RESPONSE 5
#define ERROR_ID_TOO_LARGE               6
#define ERROR_INVALID_ID                 7
#define ERROR_INVALID_SAVE_DESTINATION   8
#define ERROR_INVALID_VALUE              9
#define ERROR_FILE_LOCK                  10

// Limits

#define REQUEST_MAX_SIZE             (30 * 1024 * 1024)
#define MAX_NUM_ITEMS_IN_GET_REQUEST 100
#define MAX_NUM_ITEMS_IN_REQUEST     MAX_NUM_ITEMS_IN_GET_REQUEST

#endif // PROFILE_REQUEST_RESPONSE_H
