#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <iomanip>
#include <Shlwapi.h>
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
                    procId = pE.th32ProcessID;
                    print("Process : 0x%lX\n", pE);
                    break;
                }
            } while (Process32Next(hSnap, &pE));
        }
    }
    CloseHandle(hSnap);
    return procId;
}


BOOL InjectDLL(DWORD procID, const char* dllPath)
{
    BOOL WPM = 0;

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, procID);
    if (hProc == INVALID_HANDLE_VALUE)
    {
        return -1;
    }
    void* loc = VirtualAllocEx(hProc, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    WPM = WriteProcessMemory(hProc, loc, dllPath, strlen(dllPath) + 1, 0);
    if (!WPM)
    {
        CloseHandle(hProc);
        return -1;
    }
    print("DLL Injected Succesfully 0x%lX\n", WPM);
    HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, loc, 0, 0);
    if (!hThread)
    {
        VirtualFree(loc, strlen(dllPath) + 1, MEM_RELEASE);
        CloseHandle(hProc);
        return -1;
    }
    print("Thread Created Succesfully 0x%lX\n", hThread);
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hProc);
    VirtualFree(loc, strlen(dllPath) + 1, MEM_RELEASE);
    CloseHandle(hThread);
    return 0;
}

int main(int argc, const char **argv)
{
    if (argc < 3)
    {
        printf("Usage:%s [ProcessName] [DLLName]\n",argv[0]);
        return 0;
    }
    std::string pname, dllpath;
    pname = argv[1];
    dllpath = argv[2];

//    print("process name (The name of process to inject ) :");
//    std::cin >> pname;
//    print("dll path (Full path to the desired dll ) : ");
//    std::cin >> dllpath;
//    system("cls");

    if (PathFileExists(dllpath.c_str()) == FALSE)
    {
        print("DLL '%s' File does NOT exist!", argv[2]);
        return EXIT_FAILURE;
    }
    DWORD procId = 0;
    procId = GetProcId(pname.c_str());
    if (procId == NULL)
    {
        print("Process Not found (0x%lX)\n", GetLastError());
        print("Here is a list of available process \n", GetLastError());
        Sleep(3500);
        system("cls");
        GetProcId("skinjbir", 0b10100111001);
    }
    else
        InjectDLL(procId, dllpath.c_str());
    system("pause");
    return EXIT_SUCCESS;
}
