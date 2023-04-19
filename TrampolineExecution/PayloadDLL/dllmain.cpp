// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "MinHook.h"
#include <stdio.h>
#include <thread>

#pragma comment(lib, "libMinHook.x64.lib") 
void LogMessage(const char* format, ...);

#define USE_VALUES_FOR_DUMMY_TARGET_APP
#ifdef USE_VALUES_FOR_DUMMY_TARGET_APP
typedef void (*FUNCTION)(int*);
FUNCTION fpFunction = NULL;

char* HookedAddress = (char*)(0x0000000140011532 - 0x0000000140000000); // got this from IDA, need to readdress based on loaded module address
// second value comes from : Imagebase   : 140000000
//char* HookedAddress = (char*)(0x7FF668191532 - 0x7FF668191000); // got this from x64dbg while debugging it. 
		// Base address is copied from the start of the actual first line address inside debugger(not the load address 00007FF668180000 )

// needs to have the same calling convention and parameters at the original function !
__declspec(noinline) void __stdcall DetourFunctionForDummyTargetAppIncreaseValue(int *val)
{
	*val = *val + 2;
	unsigned char* BasePointerVal = (unsigned char*)val;
	LogMessage("Encrypt DB4 got called with RCX=%p\n", BasePointerVal);
	FILE* f;
	errno_t er = fopen_s(&f, "d:\\temp\\MaybeEncKey.txt", "at");
	if (f)
	{
		fprintf(f, "=====================================\n");
		fprintf(f, "Maybe Hashing algo : %d\n", BasePointerVal[0x100]);
		fprintf(f, "Maybe estimated SAF file size : %d\n", *(int*)&BasePointerVal[0x4c]);
		fprintf(f, "MaybeKey : ");
		for (int i = 0; i < 32; i++)
		{
			fprintf(f, "%02x ", BasePointerVal[0x140 + i]);
		}
		fprintf(f, "\n");
		fclose(f);
	}
}

void* DetouredToFunction = &DetourFunctionForDummyTargetAppIncreaseValue;
#else
typedef void (*FUNCTION)(long long,long long *);
FUNCTION fpFunction = NULL;

char* HookedAddress = (char*)(0x7FF75E20E290 - 0x7FF75DE10000);

// needs to have the same calling convention and parameters at the original function !
__declspec(noinline) void __stdcall DetourFunctionForEncryptData(long long a, long long*b)
{
	unsigned char* BasePointerVal = (unsigned char*)a;
	LogMessage("Encrypt DB4 got called with RCX=%p\n", BasePointerVal);
	FILE* f;
	errno_t er = fopen_s(&f, "d:\\temp\\MaybeEncKey.txt", "at");
	if (f)
	{
		fprintf(f, "=====================================\n");
		fprintf(f, "Maybe Hashing algo : %d\n", BasePointerVal[0x100]);
		fprintf(f, "Maybe estimated SAF file size : %d\n", *(int*)&BasePointerVal[0x4c]);
		fprintf(f, "MaybeKey : ");
		for (int i = 0; i < 32; i++)
		{
			fprintf(f, "%02x ", BasePointerVal[0x140 + i]);
		}
		fprintf(f, "\n");
		fclose(f);
	}

//	fpFunction(a,b);
}

void* DetouredToFunction = &DetourFunctionForEncryptData;
#endif

#define ENABLE_LOG_MESSAGES
#ifdef ENABLE_LOG_MESSAGES
void LogMessage(const char* format,...)
{
	FILE* f;
	errno_t er = fopen_s(&f, "PayloadLog.txt", "at");
	if (f)
	{
		va_list args;
		va_start(args, format);
		char buffer[512];
		vsnprintf(buffer, sizeof(buffer), format, args);
		fprintf(f, "%s", buffer);
		fclose(f);
		va_end(args);
	}
}
#else
#define LogMessage(x,...);
#endif

void HookAddress()
{
	HANDLE hProcess = GetModuleHandle(NULL);
	LogMessage("Current process is loaded on address : %p\n", hProcess);
	LogMessage("Hook Address from debugger : %p\n", HookedAddress);
	HookedAddress = (char*)((unsigned __int64)hProcess + (unsigned __int64)HookedAddress);
	LogMessage("New address to hook: %p\n", HookedAddress);
	LogMessage("New function address: %p\n", DetouredToFunction);

	// Initialize MinHook.
	if (MH_Initialize() != MH_OK)
	{
		LogMessage("Failed to initialize MinHook\n");
		return;
	}
	// Create a hook for target address, in disabled state.
	if (MH_CreateHook(HookedAddress, DetouredToFunction, reinterpret_cast<LPVOID*>(&fpFunction)) != MH_OK)
	{
		LogMessage("Failed to create hook\n");
		return;
	}

	// Enable the hook for MessageBoxW.
	if (MH_EnableHook(HookedAddress) != MH_OK)
	{
		LogMessage("Failed to enable hook\n");
		return;
	}
	LogMessage("Hooking done\n");
}

void UnHookAddress()
{
	LogMessage("Unloading hook\n");
	// Disable the hook for MessageBoxW.
	if (MH_DisableHook(HookedAddress) != MH_OK)
	{
		LogMessage("Failed to disable hook\n");
		return;
	}

	// Uninitialize MinHook.
	if (MH_Uninitialize() != MH_OK)
	{
		LogMessage("Failed to deinitialize hook lib\n");
		return;
	}
}

static int AppIsAlive = 1;
void threadFunction()
{
	int threadCounter = 0;
	while (AppIsAlive == 1)
	{
		Sleep(1000);
	}
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    static int initHookOnce = 0;
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
        if (initHookOnce == 0)
        {
            initHookOnce = 1;
            HookAddress();
//			std::thread myThread(threadFunction);
        }
		break;
    case DLL_THREAD_DETACH:
		break;
    case DLL_PROCESS_DETACH:
		if (initHookOnce == 1)
		{
			initHookOnce = 2;
//			LogMessage("Ignored unload attempt. Reason %d\n", ul_reason_for_call);
//			AppIsAlive = 0;
			UnHookAddress();
		}
        break;
    }
    return TRUE;
}

