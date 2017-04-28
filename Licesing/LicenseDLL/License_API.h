#pragma once

#ifndef LIBRARY_API
	#include "stdafx.h"
#endif

enum GraceReasonCodes
{
	GRC_LICENSE_IS_VALID	= 0,
	GRC_LICENSE_EXPIRED_GRACE_AVAILABLE = 1,
	GRC_LICENSE_EXPIRED_GRACE_EXPIRED = 2,
};

extern "C"
{
	// C style activation key fetch
	LIBRARY_API int	GetActivationKey(int ProjectId, int FeatureId, char *StoreResult, int MaxResultSize);
	// for extra security we could detach ourself to a new thread and return result later to an unknown buffer
	LIBRARY_API void GetActivationKeyAsync(int ProjectId, int FeatureId, void *CallBackFunc);
	// for user notification about license incomming expiration
	LIBRARY_API int	GetRemainingTime(time_t *LicenseTime, time_t *GraceTime, int *GraceReasonCode);
	// Use this deallocator for all the function returns that duplicate data. Mostly used when constructing a license
	LIBRARY_API int FreeDup(void *);
#ifdef _DEBUG
	//for debugging only
#endif
}
