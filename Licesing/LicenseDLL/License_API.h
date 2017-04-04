#pragma once

#ifndef LIBRARY_API
	#include "stdafx.h"
#endif

extern "C"
{
	// C style activation key fetch
	LIBRARY_API int	GetActivationKey(int ProjectId, int FeatureId, char *StoreResult, int MaxResultSize);
	// for extra security we could detach ourself to a new thread and return result later to an unknown buffer
	LIBRARY_API void GetActivationKeyAsync(int ProjectId, int FeatureId, void *CallBackFunc);
	//grace period related functions
	LIBRARY_API int	IsLicenseInGracePeriod(int *Result);
#ifdef _DEBUG
	//for debugging only
#endif
}
