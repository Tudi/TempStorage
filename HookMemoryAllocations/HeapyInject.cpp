#include "StdAfx.h"
#include <stdio.h>
#include <mutex>
#include <vector>
#include <memory>
#include <numeric>
#include <thread>
#include <fstream>
#include <iomanip>
#include <algorithm>

#include "HeapProfiler.h"
#include "MinHook.h"
#include "dbghelp.h"
#include <tlhelp32.h>
#include "HeapyDLLLoadHook.h"
#include "MemoryFence.h"

typedef void * (__cdecl *PtrMalloc)(size_t);
typedef void (__cdecl *PtrFree)(void *);


// Hook tables. (Lot's of static data, but it's the only way to do this.)
std::mutex hookTableMutex;
int nUsedMallocHooks = 0; 
int nUsedFreeHooks = 0; 
PtrMalloc mallocHooks[numHooks];
PtrFree freeHooks[numHooks];
PtrMalloc originalMallocs[numHooks];
PtrFree originalFrees[numHooks];

void *mallocHooksDllInst[numHooks];
void *FreeHooksDllInst[numHooks];
char mallocHooksDllInstNames[numHooks][500];
char FreeHooksDllInstNames[numHooks][500];

// TODO?: Special case for debug build malloc/frees?

HeapProfiler *heapProfiler;

// Mechanism to stop us profiling ourself.
static __declspec( thread ) int _depthCount = 0; // use thread local count

struct PreventSelfProfile
{
	PreventSelfProfile(){_depthCount++;	}
	~PreventSelfProfile(){ _depthCount--; }

	inline bool shouldProfile() { return _depthCount <= 1; }
private:
	PreventSelfProfile(const PreventSelfProfile&){}
	PreventSelfProfile& operator=(const PreventSelfProfile&){}
};

void PreventEverProfilingThisThread()
{
	_depthCount++;
}

//!! this expects wide char type pointer
template<typename T>
void WideCharStr2Str(T *pwstr, char *str, int maxlen)
{
    char *wstr = (char*)pwstr;
    if (str == NULL)
        return;
    str[0] = 0;
    if (wstr == NULL)
        return;
    //luck based check to see this string is indeed wide char.
    if (wstr[0] != 0 && wstr[1] != 0)
    {
        strcpy(str, (char*)wstr);
        return;
    }
    //assume all wide char chars are simple ascii chars. Why use widechar anyway ?
    int i;
    for (i = 0; i < maxlen * 2 && wstr[i] != 0; i += 2)
        str[i / 2] = (char)wstr[i];
    if(i/2<maxlen)
        str[i/2] = 0;
}

// Malloc hook function. Templated so we can hook many mallocs.
template <int N>
void * __cdecl mallocHook(size_t size)
{
	PreventSelfProfile preventSelfProfile;

    void * p = originalMallocs[N]( size + GetFenceSize() );
    p = InitFence(p, size);
	if(preventSelfProfile.shouldProfile())
    {
		StackTrace trace;
		trace.trace();
		heapProfiler->malloc2(p, size, trace, N);
	}

    return p;
}

//#define NEVER_DEALLOCATE_MEM    1   //check if corruption happens at runtime or after deallocation

// Free hook function.
template <int N>
void  __cdecl freeHook(void * p)
{
    return;
	PreventSelfProfile preventSelfProfile;

    int WeAllocatedIt = 0;
	if(preventSelfProfile.shouldProfile())
    {
		StackTrace trace;
		trace.trace();
        heapProfiler->free2(p, trace, N);
	}
#if CHECK_WRITE_AFTER_DEALLOC != 1 && !defined(NEVER_DEALLOCATE_MEM)
    originalFrees[N](p);
#endif
}

// Template recursion to init a hook table.
template<int N> struct InitNHooks
{
    static void initHook()
    {
        InitNHooks<N-1>::initHook();  // Compile time recursion. 

		mallocHooks[N-1] = &mallocHook<N-1>;
		freeHooks[N-1] = &freeHook<N-1>;
    }
};
 
template<> struct InitNHooks<0>
{
    static void initHook(){}
};

// Callback which recieves addresses for mallocs/frees which we hook.
BOOL CALLBACK enumSymbolsCallback(PSYMBOL_INFO symbolInfo, ULONG symbolSize, PVOID userContext)
{
	std::lock_guard<std::mutex> lk(hookTableMutex);
	PreventSelfProfile preventSelfProfile;

	PCSTR moduleName = (PCSTR)userContext;
	
    if (symbolInfo->Address == (ULONG64)mallocHooks[nUsedMallocHooks] || symbolInfo->Address == (ULONG64)freeHooks[nUsedFreeHooks])
    {
        return true;
    }
	// Hook mallocs.
	if(strcmp(symbolInfo->Name, "malloc") == 0)
    {
		if(nUsedMallocHooks >= numHooks)
        {
            TRACE0("All malloc hooks used up!\n");
			return true;
		}
        TRACE2("Hooking malloc from module %s into malloc hook num %d.\n", moduleName, nUsedMallocHooks);
		if(MH_CreateHook((void*)symbolInfo->Address, mallocHooks[nUsedMallocHooks],  (void **)&originalMallocs[nUsedMallocHooks]) != MH_OK)
            TRACE0("Create hook malloc failed!\n");

		if(MH_EnableHook((void*)symbolInfo->Address) != MH_OK)
            TRACE0("Enable malloc hook failed!\n");

        mallocHooksDllInst[nUsedMallocHooks] = (void*)symbolInfo->ModBase;
        strcpy(mallocHooksDllInstNames[nUsedMallocHooks], moduleName);

		nUsedMallocHooks++;
	}
	// Hook frees.
	else if(strcmp(symbolInfo->Name, "free") == 0)
    {
		if(nUsedFreeHooks >= numHooks)
        {
            TRACE0("All free hooks used up!\n");
			return true;
		}
		TRACE2("Hooking free from module %s into free hook num %d.\n", moduleName, nUsedFreeHooks);
		if(MH_CreateHook((void*)symbolInfo->Address, freeHooks[nUsedFreeHooks],  (void **)&originalFrees[nUsedFreeHooks]) != MH_OK)
            TRACE0("Create hook free failed!\n");

		if(MH_EnableHook((void*)symbolInfo->Address) != MH_OK)
			TRACE0("Enable free failed!\n");

        FreeHooksDllInst[nUsedFreeHooks] = (void*)symbolInfo->ModBase;
        strcpy(FreeHooksDllInstNames[nUsedFreeHooks], moduleName);

		nUsedFreeHooks++;
	}

	return true;
}
/*
int strpos(const char *LongStr, const char *ShortStr)
{
    int Ind1 = 0;
    while (LongStr[Ind1] != 0)
    {
        int Ind2 = 0;
        while (LongStr[Ind1 + Ind2] == ShortStr[Ind2] && LongStr[Ind1 + Ind2] != 0)
            Ind2++;
        if (ShortStr[Ind2] == 0)
            return Ind1;
        Ind1++;
    }
    return -1;
}

static int counter = 0;
void HeapyTrace(const char *s1, const char *s2)
{
    FILE *f = fopen("d:/temp/heapy.txt", "at");
    if (f)
    {
        if(strpos(s1,".dll")==0)
            fprintf(f, "dll[%d] = LoadLibrary(\"%s\");\n", counter++, s2);
        else
            fprintf(f, "dll[%d] = LoadLibrary(\"%s.dll\");\n", counter++, s2);
        fclose(f);
    }
}*/

// Callback which recieves loaded module names which we search for malloc/frees to hook.
BOOL CALLBACK enumModulesCallback(PCSTR ModuleName, DWORD_PTR BaseOfDll, PVOID UserContext)
{
	// TODO: Hooking msvcrt causes problems with cleaning up stdio - avoid for now.
//	if(strcmp(ModuleName, "msvcrt") == 0) 
//		return true;
//    if (strcmp(ModuleName, "ucrtbase") == 0)
//        return true;

 /*   char TName[500];
    WideCharStr2Str(ModuleName, TName, sizeof(TName));
    HeapyTrace("Dll Loaded: %s\n", TName); */
    SymEnumSymbols(GetCurrentProcess(), BaseOfDll, "malloc", enumSymbolsCallback, (void*)ModuleName);
    SymEnumSymbols(GetCurrentProcess(), BaseOfDll, "free", enumSymbolsCallback, (void*)ModuleName);
	return true;
}

void printTopAllocationReport(int numToPrint)
{

    if (heapProfiler->HasData()==false)
        return;
	std::vector<std::pair<StackTrace, size_t>> allocsSortedBySize;
	heapProfiler->getAllocationSiteReport(allocsSortedBySize);

	// Sort retured allocation sites by size of memory allocated, descending.
	std::sort(allocsSortedBySize.begin(), allocsSortedBySize.end(), 
		[](const std::pair<StackTrace, size_t> &a, const std::pair<StackTrace, size_t> &b)
        {	return a.second < b.second;	}
	);
	

	std::ofstream stream("Heapy_Profile.txt",  std::ios::out | std::ios::app);
	stream << "=======================================\n\n";
	stream << "Printing top allocation points.\n\n";
	// Print top allocations sites in ascending order.
	auto precision = std::setprecision(5);
	size_t totalPrintedAllocSize = 0;
	size_t numPrintedAllocations = 0;
	double bytesInAMegaByte = 1024*1024;
	for(size_t i = (size_t)(std::max)(int64_t(allocsSortedBySize.size())-numToPrint, int64_t(0)); i < allocsSortedBySize.size(); ++i)
    {

		if(allocsSortedBySize[i].second == 0)
			continue;

		stream << "Alloc size " << precision << allocsSortedBySize[i].second/bytesInAMegaByte << "Mb, stack trace: \n";
		allocsSortedBySize[i].first.print(stream);
		stream << "\n";

		totalPrintedAllocSize += allocsSortedBySize[i].second;
		numPrintedAllocations++;
	}

	size_t totalAlloctaions = std::accumulate(allocsSortedBySize.begin(), allocsSortedBySize.end(), size_t(0),
		[](size_t a,  const std::pair<StackTrace, size_t> &b)
        {	return a + b.second; }
	);

	stream << "Top " << numPrintedAllocations << " allocations: " << precision <<  totalPrintedAllocSize/bytesInAMegaByte << "Mb\n";
	stream << "Total allocations: " << precision << totalAlloctaions/bytesInAMegaByte << "Mb" << 
		" (difference between total and top " << numPrintedAllocations << " allocations : " << (totalAlloctaions - totalPrintedAllocSize)/bytesInAMegaByte << "Mb)\n\n";
}
/*
struct CatchExit
{
	~CatchExit()
    {
		PreventSelfProfile p;
		printTopAllocationReport(25);
	}
};
CatchExit catchExit; */

int heapProfileReportThread()
{
	PreventEverProfilingThisThread();
	while(true)
    {
		Sleep(10000); 
		printTopAllocationReport(25);
	}
}

int CheckHeapWriteAfterPointerDelete()
{
    PreventEverProfilingThisThread();
    while (true)
    {
        Sleep(1000);
        heapProfiler->CheckNoBadPointerUsage();
    }
}

VOID NTAPI MyLdrDllNotification( ULONG NotificationReason, PCLDR_DLL_NOTIFICATION_DATA NotificationData, PVOID Context )
{
    switch (NotificationReason)
    {
        case LDR_DLL_NOTIFICATION_REASON_LOADED:
        {
/*            char TName[500];
            WideCharStr2Str(NotificationData->Loaded.FullDllName->Buffer, TName, sizeof(TName));
            HeapyTrace("Dll Loaded: %s\n", TName); */
            SymEnumSymbols(GetCurrentProcess(), (DWORD_PTR)NotificationData->Loaded.DllBase, "malloc", enumSymbolsCallback, (void*)NotificationData->Loaded.BaseDllName);
            SymEnumSymbols(GetCurrentProcess(), (DWORD_PTR)NotificationData->Loaded.DllBase, "free", enumSymbolsCallback, (void*)NotificationData->Loaded.BaseDllName);
        }break;
        case LDR_DLL_NOTIFICATION_REASON_UNLOADED:
        {
//            char TName[500];
//            WideCharStr2Str(NotificationData->Unloaded.FullDllName->Buffer, TName, sizeof(TName));
//            HeapyTrace("Dll Unloaded: %s\n", TName);
        }break;
    }
}

void setupHeapProfiling()
{
    PreventSelfProfile preventSelfProfile;
    // We use printfs thoughout injection becasue it's just safer/less troublesome
	// than iostreams for this sort of low-level/hacky/threaded work.
	printf("Injecting library...\n");

	nUsedMallocHooks = 0;
	nUsedFreeHooks = 0;

	// Create our hook pointer tables using template meta programming fu.
	InitNHooks<numHooks>::initHook(); 

	// Init min hook framework.
	MH_Initialize(); 

	// Init dbghelp framework.
	if(!SymInitialize(GetCurrentProcess(), NULL, true))
		printf("SymInitialize failed\n");

	// Yes this leaks - cleauing it up at application exit has zero real benefit.
	// Might be able to clean it up on CatchExit but I don't see the point.
	heapProfiler = new HeapProfiler(); 

	// Trawl though loaded modules and hook any mallocs and frees we find.
	SymEnumerateModules(GetCurrentProcess(), enumModulesCallback, NULL);

    HMODULE hModule = GetModuleHandleW(L"NTDLL.DLL");

    pfnLdrRegisterDllNotification pLdrRegisterDllNotification = (pfnLdrRegisterDllNotification)GetProcAddress(hModule, "LdrRegisterDllNotification");
    pfnLdrUnregisterDllNotification pLdrUnregisterDllNotification = (pfnLdrUnregisterDllNotification)GetProcAddress(hModule, "LdrUnregisterDllNotification");
    void *pvCookie = NULL;

    pLdrRegisterDllNotification(0, MyLdrDllNotification, NULL, &pvCookie);

    //   
/*    if (pvCookie)
    {
        pLdrUnregisterDllNotification(pvCookie);
        pvCookie = NULL;
    } */

//	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&heapProfileReportThread, NULL, 0, NULL);
#if CHECK_WRITE_AFTER_DEALLOC == 1
    CreateThread(0, 0, (LPTHREAD_START_ROUTINE)&CheckHeapWriteAfterPointerDelete, NULL, 0, NULL);
#endif
}
