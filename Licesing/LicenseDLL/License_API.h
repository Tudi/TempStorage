#pragma once

#ifndef LIBRARY_API
	#include "stdafx.h"
#endif

enum GraceReasonCodes
{
	GRC_LICENSE_IS_VALID	= 0,
	GRC_LICENSE_EXPIRED_GRACE_AVAILABLE = 1,
	GRC_LICENSE_EXPIRED_GRACE_EXPIRED = 2,
	GRC_LICENSE_IS_VALID_GRACE_TRIGGERED = 3,
	GRC_LICENSE_UNEXPECTED_VALUE
};

typedef void(*NotificationCallback)(int, char*);

/**
* @file License_API.h
* @author Jozsa Bodnar Istvan
* @date 9 May 2017
* @brief Main API functions to be used by projects that consume license module.
* C style wrapper API function calls to simplify usage of Licensing module. These should be all the functions you need to use for licensing. Rest can be obfuscated.
*
*/
extern "C"
{
	// C style activation key fetch
	LIBRARY_API int	GetActivationKey(int ProjectId, int FeatureId, char *StoreResult, int MaxResultSize);
	// for extra security we could detach ourself to a new thread and return result later to an unknown buffer
	LIBRARY_API void GetActivationKeyAsync(int ProjectId, int FeatureId, void *CallBackFunc);
	// for user notification about license incomming expiration
	LIBRARY_API int	GetRemainingTime(time_t *LicenseTime, time_t *GraceTime, int *GraceReasonCode);
	// Register a function that will get called on license change related events
	LIBRARY_API int	NotificationRegister( NotificationCallback );
	// Use this deallocator for all the function returns that duplicate data. Mostly used when constructing a license
	LIBRARY_API int FreeDup(void *);
}

#define CURENT_LICENSE_NODE_VER				1
#define ERROR_INVALID_PROJECT_FEAURE		(-2)
#define WARNING_NO_LICENSE_FOUND			(-3)
#define WARNING_MISSING_LICENSE_FIELD		(-4)
#define ERROR_LICENSE_EXPIRED				(-5)
#define ERROR_COULD_NOT_LOAD_LIC_FILE		(-6)
#define ERROR_COULD_NOT_STORE_GRACE_PERIOD	(-7)
#define ERROR_LICENSE_INVALID				(-8)
#define ERROR_MISSING_PARAMETER				(-9)
#define ERROR_DISABLED_FUNCTION				(-10)
#define ERROR_MISSING_ENCRYPTION_KEY		(-11)
#define ERROR_NO_DECODABLE_LICENSE_FOUND	(-12)
#define ERROR_API_INVALID_USAGE				(-13)
#define ERROR_COULD_NOT_CREATE_OBJECT		(-14)
#define ERROR_COULD_NOT_QUERY_GRACE_PERIOD	(-15)
#define ERROR_LICENSE_IS_CORRUPTED			(-16)
#define ERROR_RETURN_BUFFER_TOO_SMALL		(-17)
#define ERROR_COULD_NOT_SAVE_LICENSE		(-18)
#define ERROR_COULD_NOT_LOAD_GRACE_PERIOD	(-19)
#define ERROR_COULD_NOT_CREATE_THREAD		(-20)
#define ERROR_BAD_PATH_NAME					(-21)
