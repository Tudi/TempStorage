#ifndef _APP_ERRORS_H_
#define _APP_ERRORS_H_

// lists of error codes function will return
typedef enum ApplicationErrorCodes
{
	ERR_NO_ERROR = 0,
	ERR_FAILED_TO_OPEN_CONFIG_FILE,
	ERR_CONFIG_FILE_EMPTY,
	ERR_MISSING_CONF_VALUE,
	ERR_ID_MISSING_FROM_ENTRY,
	ERR_FAILED_TO_ALLOCATE_MEMORY,
	ERR_FAILED_TO_CREATE_THREAD,
	ERR_FAILED_TO_JOIN_THREAD,
}ApplicationErrorCodes;

#endif