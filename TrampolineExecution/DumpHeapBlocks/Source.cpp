#include <Windows.h>
#include <DbgHelp.h>
#include <iostream>
#include <vector>
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <iomanip>
#include <Shlwapi.h>
#include <psapi.h>
#include <intrin.h>

#pragma comment( lib, "shlwapi.lib")

void EnableDebugPriv()
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

DWORD GetProcId(const char* pn)
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
                if (!_stricmp(pE.szExeFile, pn))
                {
                    procId = pE.th32ProcessID;
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

size_t usage;

void show_modules(HANDLE process, int skipFree, int skipReserved) 
{
    FILE* f;
    errno_t openErr = fopen_s(&f, "allocs.dmp", "wb");
    if (f == NULL)
    {
        printf("Failed to open output file. Aborting\n");
        return;
    }
    unsigned char* p = NULL;
    MEMORY_BASIC_INFORMATION info;

    for (p = NULL; VirtualQueryEx(process, p, &info, sizeof(info)) == sizeof(info); p += info.RegionSize)
    {
        if (skipFree && info.State == MEM_FREE)
        {
            continue;
        }
        if (skipReserved && info.State == MEM_RESERVE)
        {
            continue;
        }
        printf("%#10.10llx (%6zdK)\t", (unsigned __int64)info.BaseAddress, info.RegionSize / 1024);

        switch (info.State) {
        case MEM_COMMIT:
            printf("Committed");
            break;
        case MEM_RESERVE:
            printf("Reserved");
            break;
        case MEM_FREE:
            printf("Free");
            break;
        }
        printf("\t");
        switch (info.Type) {
        case MEM_IMAGE:
            printf("Code Module");
            break;
        case MEM_MAPPED:
            printf("Mapped     ");
            break;
        case MEM_PRIVATE:
            printf("Private    ");
        }
        printf("\t");

        if ((info.State == MEM_COMMIT) && (info.Type == MEM_PRIVATE))
            usage += info.RegionSize;

        int guard = 0, nocache = 0;

        if (info.AllocationProtect & PAGE_NOCACHE)
            nocache = 1;
        if (info.AllocationProtect & PAGE_GUARD)
            guard = 1;

        info.AllocationProtect &= ~(PAGE_GUARD | PAGE_NOCACHE);

        switch (info.AllocationProtect) {
        case PAGE_READONLY:
            printf("Read Only");
            break;
        case PAGE_READWRITE:
            printf("Read/Write");
            break;
        case PAGE_WRITECOPY:
            printf("Copy on Write");
            break;
        case PAGE_EXECUTE:
            printf("Execute only");
            break;
        case PAGE_EXECUTE_READ:
            printf("Execute/Read");
            break;
        case PAGE_EXECUTE_READWRITE:
            printf("Execute/Read/Write");
            break;
        case PAGE_EXECUTE_WRITECOPY:
            printf("COW Executable");
            break;
        }

        if (guard)
            printf("\tguard page");
        if (nocache)
            printf("\tnon-cachable");

        DWORD oldProtect;
        if (VirtualProtectEx(process, info.BaseAddress, info.RegionSize, PAGE_READWRITE, &oldProtect))
        {
            char* buffer = (char*)malloc(info.RegionSize);
            SIZE_T bytesRead;
            if (buffer != NULL && ReadProcessMemory(process, info.BaseAddress, buffer, info.RegionSize, &bytesRead))
            {
                fwrite(&bytesRead, 1, 8, f);
                fwrite(buffer, 1, bytesRead, f);
#ifdef USE_DUMMY_TARGET_APP_TO_SCAN_VALUE
                printf("\t");
                for (size_t i = 0; i < bytesRead; i++)
                {
                    if (strcmp(&buffer[i], "You got me") == 0)
                    {
                        printf("Found it at %zd", i);
                    }
                }
                // Print the first 16 bytes of the memory block in hexadecimal format
                for (int i = 0; i < 8; i++)
                {
    //                std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)buffer[i] << " ";
    //                printf("%c", buffer[i]);
                }
#endif
            }
            if (!VirtualProtectEx(process, info.BaseAddress, info.RegionSize, oldProtect, &oldProtect))
            {
                printf("Failed to restore memory protection");
            }
            free(buffer);
        }

        printf("\n");
    }
    fclose(f);
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
        printf("Process Not found (0x%lX)\n", GetLastError());
        return EXIT_FAILURE;
    }

    EnableDebugPriv();

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, procId);
    if (hProc == INVALID_HANDLE_VALUE)
    {
        return -1;
    }
    HMODULE hProcess = GetProcessModule(hProc, pname.c_str());

    show_modules(hProc, 1, 1);

    CloseHandle(hProc);

    return 0;
}
