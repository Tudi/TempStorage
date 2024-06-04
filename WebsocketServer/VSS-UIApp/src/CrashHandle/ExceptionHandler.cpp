#include "StdAfx.h"

#include <windows.h>
#include <dbghelp.h>

#pragma comment(lib, "Dbghelp.lib")

int GenerateMinidump(const char *szFileName, EXCEPTION_POINTERS* pExPtrs)
{
    // Initialize the Debug Help Library
    if (!SymInitialize(GetCurrentProcess(), NULL, TRUE)) {
        // Handle initialization failure
        return 1;
    }

    // Set up the minidump file name and create it
    HANDLE hFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        // Handle file creation failure
        return 1;
    }

    // Generate the minidump
    MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
    exceptionInfo.ThreadId = GetCurrentThreadId();
    exceptionInfo.ExceptionPointers = pExPtrs;
    exceptionInfo.ClientPointers = FALSE;

//    MINIDUMP_TYPE mdt = (MINIDUMP_TYPE)(MiniDumpWithHandleData | MiniDumpWithProcessThreadData | MiniDumpWithThreadInfo);
//    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, mdt, &exceptionInfo, NULL, NULL);
    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &exceptionInfo, NULL, NULL);

    // Clean up
    CloseHandle(hFile);
    SymCleanup(GetCurrentProcess());

    return 0;
}

#if defined(_M_X64) || defined(_M_IX86) || defined(_M_ARM64)
static BOOL PreventSetUnhandledExceptionFilter()
{
    HMODULE hKernel32 = LoadLibrary(("kernel32.dll"));
    if (hKernel32 == NULL)
        return FALSE;
    void* pOrgEntry = GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
    if (pOrgEntry == NULL)
        return FALSE;

#ifdef _M_IX86
    // Code for x86:
    // 33 C0                xor         eax,eax
    // C2 04 00             ret         4
    unsigned char szExecute[] = { 0x33, 0xC0, 0xC2, 0x04, 0x00 };
#elif _M_X64
    // 33 C0                xor         eax,eax
    // C3                   ret
    unsigned char szExecute[] = { 0x33, 0xC0, 0xC3 };
#else
#error "The following code only works for x86 and x64!"
#endif

    DWORD dwOldProtect = 0;
    BOOL  bProt = VirtualProtect(pOrgEntry, sizeof(szExecute), PAGE_EXECUTE_READWRITE, &dwOldProtect);

    SIZE_T bytesWritten = 0;
    BOOL   bRet = WriteProcessMemory(GetCurrentProcess(), pOrgEntry, szExecute, sizeof(szExecute),
        &bytesWritten);

    if ((bProt != FALSE) && (dwOldProtect != PAGE_EXECUTE_READWRITE))
    {
        DWORD dwBuf;
        VirtualProtect(pOrgEntry, sizeof(szExecute), dwOldProtect, &dwBuf);
    }
    return bRet;
}
#else
#pragma message("This code works only for x86, x64 and arm64!")
#endif

//static TCHAR s_szExceptionLogFileName[_MAX_PATH] = ("\\exceptions.log"); // default
static BOOL  s_bUnhandledExeptionFilterSet = FALSE;
static LONG __stdcall CrashHandlerExceptionFilter(EXCEPTION_POINTERS* pExPtrs)
{
#ifdef _M_IX86
    if (pExPtrs->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
    {
        static char MyStack[1024 * 128]; // be sure that we have enough space...
        // it assumes that DS and SS are the same!!! (this is the case for Win32)
        // change the stack only if the selectors are the same (this is the case for Win32)
        //__asm push offset MyStack[1024*128];
        //__asm pop esp;
        __asm mov eax, offset MyStack[1024 * 128];
        __asm mov esp, eax;
    }
#endif

    TCHAR szModName[_MAX_PATH];
    DWORD dTick = GetTickCount();
    sprintf_s(szModName, ".\\Exceptions\\%u.log", dTick);

    FILE* f;
    errno_t err = fopen_s(&f, szModName, "wt");
    if (f && err == NO_ERROR)
    {
        StackWalker sw;
        sw.ShowCallstack(GetCurrentThread(), pExPtrs->ContextRecord);
        fprintf(f, "%s", sw.m_sOutString);
        fclose(f);

        // try to send it to the server
        AddLogEntry(LogDestinationFlags::LDF_SERVER_FILE, LogSeverityValue::LogSeverityCritical, LogSourceGroups::LogSourceCrashHandler, 0, 0, sw.m_sOutString);
    }

    // generating a minidump might fail in case of a stack corruption
    sprintf_s(szModName, ".\\Exceptions\\%u.dmp", dTick);
    GenerateMinidump(szModName, pExPtrs);

#ifdef POPUP_WINDOW_ON_CRASH
    TCHAR lString[500];
    sprintf_s(lString, "*** Unhandled Exception! See console output for more infos!\n"
        "   ExpCode: 0x%8.8X\n"
        "   ExpFlags: %d\n"
        "   ExpAddress: 0x%8.8p\n"
        "   Please report!",
        pExPtrs->ExceptionRecord->ExceptionCode, pExPtrs->ExceptionRecord->ExceptionFlags,
        pExPtrs->ExceptionRecord->ExceptionAddress);
    FatalAppExit((UINT) - 1, lString);
#endif

    return EXCEPTION_CONTINUE_SEARCH;
}

static void InitUnhandledExceptionFilter()
{
//    TCHAR szModName[_MAX_PATH];
//    szModName[0] = 0;
//    if (GetModuleFileName(NULL, szModName, sizeof(szModName) / sizeof(TCHAR)) != 0)
    {
//        strcat_s(s_szExceptionLogFileName, szModName);
//        sprintf_s(szModName, "%s_%u.log", szModName, GetTickCount());
    }
    if (s_bUnhandledExeptionFilterSet == FALSE)
    {
        // set global exception handler (for handling all unhandled exceptions)
        SetUnhandledExceptionFilter(CrashHandlerExceptionFilter);
#if defined _M_X64 || defined _M_IX86 || defined _M_ARM64
        PreventSetUnhandledExceptionFilter();
#endif
        s_bUnhandledExeptionFilterSet = TRUE;
    }
}

void ExceptionHandlerInit()
{
    InitUnhandledExceptionFilter();
}