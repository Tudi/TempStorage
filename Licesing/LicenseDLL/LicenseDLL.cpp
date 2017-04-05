// LicenseDLL.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "License_Grace.h"
#include "License.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	//disable warning for unused variables
	(void)(hModule);
	(void)(lpReserved);

	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		InitLogging();
		//check if license is expired
//		char TempStore[500];
//		GetActivationKey(0, 0, TempStore, sizeof(TempStore));
		//start watchdog timer to countdown remaining seconds
		StartLicenseGraceWatchdogThread();
	}
	else if( ul_reason_for_call == DLL_PROCESS_DETACH )
	{
		EndLicenseGraceWatchdogThread();
		ShutDownLogging();
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif
