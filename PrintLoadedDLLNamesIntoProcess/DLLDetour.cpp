// HookDllEntryPoint.cpp by Jim Harkins (jimhark), Nov 2010

#include "stdafx.h"
#include <stdio.h>
#include <winternl.h>
#include <list>

typedef NTSTATUS(WINAPI *pfnZwQueryInformationProcess)(
    __in       HANDLE ProcessHandle,
    __in       PROCESSINFOCLASS ProcessInformationClass,
    __out      PVOID ProcessInformation,
    __in       ULONG ProcessInformationLength,
    __out_opt  PULONG ReturnLength);

HMODULE hmodNtdll = LoadLibrary(_T("ntdll.dll"));

pfnZwQueryInformationProcess pZwQueryInformationProcess = (pfnZwQueryInformationProcess)GetProcAddress( hmodNtdll, "ZwQueryInformationProcess");

typedef BOOL(WINAPI *PDLLMAIN) ( __in  HINSTANCE hinstDLL,  __in  DWORD fdwReason, __in  LPVOID lpvReserved);

struct DLLEntryPointStore
{
    PVOID       DLLBase;
    PDLLMAIN    OldEntry;
    PDLLMAIN    *OldEntryStore;
    short       *UnicodeName;
    bool        MarkedUnloaded;
};

#include "StrTools.h"

static int counter = 0;
void DetourTrace(const char *s1, const char *s2)
{
    FILE *f = fopen("d:/temp/detour.txt", "at");
    if (f)
    {
        char OutBuff[500];
        StrSimplePrint( OutBuff, sizeof( OutBuff ), "dll[%d] = LoadLibrary(\"%s\");\n", counter++, s2);
        fputs( OutBuff, f );
        fclose(f);
    }
}
void DetourTrace2(const char *s1)
{
    FILE *f = fopen("d:/temp/detour_.txt", "at");
    if (f)
    {
        fputs( s1, f );
        fclose(f);
    }
}

//this has to leak memory or else it will get destroyed before process exit
std::list< DLLEntryPointStore* > *DetourList = NULL;
int TotalDLLsUnLoaded = 0;

void RestoreAllHooks()
{
    for (auto itr = begin(*DetourList); itr != end(*DetourList); itr++)
        if( (*itr)->MarkedUnloaded == false )
            *(*itr)->OldEntryStore = (*itr)->OldEntry;  
}

static int HaveAccessToCRT = 1;
BOOL WINAPI DllMain_advapi32( __in  HINSTANCE hinstDLL, __in  DWORD fdwReason, __in  LPVOID lpvReserved)
{
    char *pszReason;
    int IsProcessDetach = 0;

    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        pszReason = "DLL_PROCESS_ATTACH";
        break;
    case DLL_PROCESS_DETACH:
        pszReason = "DLL_PROCESS_DETACH";
        TotalDLLsUnLoaded++;
        IsProcessDetach = 1;
        break;
    case DLL_THREAD_ATTACH:
        pszReason = "DLL_THREAD_ATTACH";
        break;
    case DLL_THREAD_DETACH:
        pszReason = "DLL_THREAD_DETACH";
        break;
    default:
        pszReason = "*UNKNOWN*";
        break;
    }

    //try to find ourself
    PDLLMAIN pDllMain_advapi32 = NULL;
    char TraceBuf[2000];
    TraceBuf[0] = 0;
    if (HaveAccessToCRT == 1)    //we will have no access to basic C functions after a while. Learn to implement all by yourself
    {
        int TotalDLLsLoaded = (*DetourList).size();
        for (auto itr = begin(*DetourList); itr != end(*DetourList); itr++)
            if ((*itr)->DLLBase == hinstDLL)
            {
                pDllMain_advapi32 = (*itr)->OldEntry;
                (*itr)->MarkedUnloaded = true;
                *(*itr)->OldEntryStore = (*itr)->OldEntry;  //restore old entry, maybe we will avoid a crash now
                //barbaric WideCharConversion
                char NameBuf[2000];
                WideCharStr2Str1((*itr)->UnicodeName, NameBuf, sizeof(NameBuf));
//                sprintf_s(TraceBuf, sizeof(TraceBuf), "1) DllMain(0x % .8X, %s, 0x % .8X) TotalDLLs(%d), Unloaded(%d) - %s\n", (int)hinstDLL, pszReason, (int)lpvReserved, TotalDLLsLoaded, TotalDLLsUnLoaded, NameBuf);
                StrSimplePrint(TraceBuf, sizeof(TraceBuf), "Unloading TotalDLLs(%d), Unloaded(%d) - %s\n", TotalDLLsLoaded, TotalDLLsUnLoaded, NameBuf);
                //            if (strpos(NameBuf, "MSVCP140D") > 0)
                //                _CrtDumpMemoryLeaks();
                if (strpos1(NameBuf, "CRTBASE") >= 0)
                {
                    RestoreAllHooks();
                    HaveAccessToCRT = 0;
                }
                break;
            }
        if (pDllMain_advapi32 == NULL)
            sprintf_s(TraceBuf, sizeof(TraceBuf), "1) DllMain(0x % .8X, %s, 0x % .8X)\n", (int)hinstDLL, pszReason, (int)lpvReserved);

        TRACE0(TraceBuf);
        DetourTrace2(TraceBuf);
    }

    if (NULL == pDllMain_advapi32)
    {
        return FALSE;
    }
    else
    {
        return (*pDllMain_advapi32)( hinstDLL, fdwReason, lpvReserved );
    }
}

void __cdecl HookAllDllEntryPoints()
{
    PROCESS_BASIC_INFORMATION pbi = { 0 };
    ULONG ulcbpbi = 0;

    NTSTATUS nts = (*pZwQueryInformationProcess)(GetCurrentProcess(), ProcessBasicInformation, &pbi, sizeof(pbi), &ulcbpbi);
    if (nts != 0)
        return;

    DetourList = new std::list< DLLEntryPointStore* >;
    PLIST_ENTRY pcurModule = pbi.PebBaseAddress->Ldr->InMemoryOrderModuleList.Flink;

    while (pcurModule != &pbi.PebBaseAddress->Ldr->InMemoryOrderModuleList)
    {
        PLDR_DATA_TABLE_ENTRY pldte = (PLDR_DATA_TABLE_ENTRY)(CONTAINING_RECORD(pcurModule, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks));

        //detour this DLL
        DLLEntryPointStore *tStore = new DLLEntryPointStore;
        tStore->DLLBase = pldte->DllBase;
        tStore->OldEntry = (PDLLMAIN)pldte->Reserved3[0];
        tStore->OldEntryStore = (PDLLMAIN*)&pldte->Reserved3[0];
        tStore->UnicodeName = (short*)pldte->FullDllName.Buffer;
        tStore->MarkedUnloaded = false;
        pldte->Reserved3[0] = DllMain_advapi32;
        DetourList->push_back(tStore);

        pcurModule = pcurModule->Flink;

        //log this DLL as we are tracing it
        char NameBuf[2000];
        WideCharStr2Str1(pldte->FullDllName.Buffer, NameBuf, sizeof(NameBuf));
        DetourTrace("", NameBuf);

    }

    return;
}
