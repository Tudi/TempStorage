#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <iomanip>
#include <Shlwapi.h>
#include <psapi.h>
#include <intrin.h>
#pragma comment( lib, "shlwapi.lib")


#define print(format, ...) fprintf (stderr, format, __VA_ARGS__)

DWORD GetProcId(const char* pn, unsigned short int fi = 0b1101)
{
    DWORD procId = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnap != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 pE;
        pE.dwSize = sizeof(pE);

        if (Process32First(hSnap, &pE))
        {
            if (!pE.th32ProcessID)
                Process32Next(hSnap, &pE);
            do
            {
                if (fi == 0b10100111001)
                    std::cout << pE.szExeFile << u8"\x9\x9\x9" << pE.th32ProcessID << std::endl;
                if (!_stricmp(pE.szExeFile, pn))
                {
                    printf("Process : 0x%lX\n", pE.th32ProcessID);
                    break;
                }
            } while (Process32Next(hSnap, &pE));
        }
    }
    CloseHandle(hSnap);
    return procId;
}

HMODULE GetProcessModule(HANDLE processHandle, const char* moduleName)
{
    HMODULE moduleHandles[1024];
    DWORD cbNeeded;
    if (EnumProcessModules(processHandle, moduleHandles, sizeof(moduleHandles), &cbNeeded)) 
    {
        for (int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) 
        {
            TCHAR moduleName[MAX_PATH];
            if (GetModuleBaseName(processHandle, moduleHandles[i], moduleName, sizeof(moduleName) / sizeof(TCHAR)))
            {
                if (strcmp(moduleName, moduleName) == 0) 
                {
                    printf("Found module handle: 0x%p\n", moduleHandles[i]);
                    return moduleHandles[i];
                }
            }
        }
    }
    else 
    {
        printf("Failed to enumerate process modules (error %lu)\n", GetLastError());
    }

    return NULL;
}

typedef void (*FUNCTION)();

#define MAGIC_VALUE_TO_SEARCH 0xBAD01337
// will copy the body of this function and write it to the specified address
// you can not use external function calls as the virtual addressing will mess it up
#pragma optimize( "", off )
extern "C" __declspec(noinline) void MyFuncBody(int *arg)
{
    int mystackvar1 = MAGIC_VALUE_TO_SEARCH; // do not remove this. Used to determine where actual code starts

    *arg = 0;
    *arg = 0;
    *arg = 0;
    *arg = 0;
    *arg = 0;
    *arg = 0;
    *arg = 0;
    *arg = 0;
    *arg = 0; // should fill the function with NOPs

    int mystackvar2 = MAGIC_VALUE_TO_SEARCH; // do not remove this. Used to determine where actual code ends
}
#pragma optimize( "", on ) 

void GetFunctionDataAndSize(FUNCTION &funcOffset, unsigned __int64 &funcSize)
{
    int tempPointer;
    MyFuncBody(&tempPointer);

    printf("address of my function: %p\n", &MyFuncBody);

    funcOffset = NULL;
    funcSize = 0;

    // get the beggining of function code without decorator
    char* funcBody = (char*)*MyFuncBody;
    for (unsigned __int64 offset = 0; offset < 0xffff; offset++)
    {
        int val = *(int*)&funcBody[offset];
        if ( val == MAGIC_VALUE_TO_SEARCH)
        {
            funcOffset = (FUNCTION)(funcBody + offset + sizeof(int));
            break;
        }
    }
    printf("Actual start address of my function : %p\n", funcOffset);

    funcBody = (char*)funcOffset;
    for (unsigned __int64 offset = 0; offset < 0xffff; offset++)
    {
        int val = *(int*)&funcBody[offset];
        if (val == MAGIC_VALUE_TO_SEARCH)
        {
            funcSize = offset - 3; // size of the mov operation
            break;
        }
    }

    printf("Size of my function : %lld\n", funcSize);
}

BOOL InjectCode(DWORD procID, const char *processName)
{
    BOOL WPM = 0;

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, procID);
    if (hProc == INVALID_HANDLE_VALUE)
    {
        return -1;
    }

    FUNCTION newFuncOffset;
    unsigned __int64 newFuncByteSize;
    GetFunctionDataAndSize(newFuncOffset, newFuncByteSize);

    if (newFuncOffset == NULL || newFuncByteSize == 0)
    {
        return -1;
    }

    char * HookedAddress = (char*)(0x00000001400126B3 - 0x0000000140000000); // the offset we want to edit in target application
    HANDLE hProcess = GetProcessModule(hProc, processName);
    printf("Current process is loaded on address : %p\n", hProcess);
    printf("Hook Address from debugger : %p\n", HookedAddress);
    HookedAddress = (char*)((unsigned __int64)hProcess + (unsigned __int64)HookedAddress);
    printf("Overwrite process address : %p\n", HookedAddress);

    unsigned long		Protection = 0;

    VirtualProtect((void*)HookedAddress, newFuncByteSize, PAGE_EXECUTE_READWRITE, &Protection);
    WriteProcessMemory(hProc, (void*)HookedAddress, newFuncOffset, newFuncByteSize, 0);
    VirtualProtect((void*)HookedAddress, newFuncByteSize, Protection, &Protection);
    CloseHandle(hProc);

    return 0;
}

void EnableDebugPriv(void)
{
    HANDLE              hToken;
    LUID                SeDebugNameValue;
    TOKEN_PRIVILEGES    TokenPrivileges;

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &SeDebugNameValue))
        {
            TokenPrivileges.PrivilegeCount = 1;
            TokenPrivileges.Privileges[0].Luid = SeDebugNameValue;
            TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

            if (AdjustTokenPrivileges(hToken, FALSE, &TokenPrivileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
            {
                CloseHandle(hToken);
            }
            else
            {
                CloseHandle(hToken);
                throw std::exception("Couldn't adjust token privileges!");
            }
        }
        else
        {
            CloseHandle(hToken);
            throw std::exception("Couldn't look up privilege value!");
        }
    }
    else
    {
        throw std::exception("Couldn't open process token!");
    }
}

int main(int argc, const char** argv)
{
    if (argc < 2)
    {
        printf("Usage:%s [ProcessName]\n", argv[0]);
        return 0;
    }
    std::string pname;
    pname = argv[1];
    DWORD procId = 0;
    procId = GetProcId(pname.c_str());
    if (procId == NULL)
    {
        print("Process Not found (0x%lX)\n", GetLastError());
        return EXIT_FAILURE;
    }

    EnableDebugPriv();
    InjectCode(procId, pname.c_str());
    return EXIT_SUCCESS;
}