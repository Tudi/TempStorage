#include <windows.h>
#include "FunctionCallHooks.h"
#include "MapParse.h"

#define AN_UNLIKELY_RANDOM_VALUE	123456789
static volatile int BlocRecursive = 1;

//#define WINDOWS_SUPPORT
#ifdef WINDOWS_SUPPORT
extern "C" void __declspec(naked) _cdecl _penter(void)
{
	_asm {
		push eax
		push ebx
		push ecx
		push edx
		push ebp
		push edi
		push esi
	}
	//delay logging values until we say so
	if (BlocRecursive == AN_UNLIKELY_RANDOM_VALUE)
	{
		//stop tracking functions while we work
		BlocRecursive = 1;

		char *PrevFunctionName;
		void *stack[2];
		if (CaptureStackBackTrace(0, 2, stack, NULL) != 1)
			PrevFunctionName = NULL;
		else
			PrevFunctionName = GetSymStore()->GetFuncName(stack[0]);
		if (PrevFunctionName != NULL)
			printf_s("In stack %s %p\n", PrevFunctionName, stack[0]);
		else
			printf_s("Unknown func name\n");

		//reenable function hooking
		BlocRecursive = AN_UNLIKELY_RANDOM_VALUE;
	}
	_asm {
		pop esi
		pop edi
		pop ebp
		pop edx
		pop ecx
		pop ebx
		pop eax
		ret
	}
}
#else
void _stdcall LogFunctionEntrance(char *FuncName)
{
	char FileName[500];
	sprintf_s(FileName, sizeof(FileName), "CallLog_%d_%d.txt", (int)GetCurrentProcessId(), (int)GetCurrentThreadId());
	FILE *f;
	errno_t openres = fopen_s( &f, FileName, "at");
	if (f)
	{
		fprintf(f, "%s\n", FuncName); // could use fwrite...
		fclose(f);
	}
}
extern "C" void __declspec(naked) _cdecl _penter(void)
{
	_asm {
		push eax
		push ebx
		push ecx
		push edx
		push ebp
		push edi
		push esi
	}
	//delay logging values until we say so
	if (BlocRecursive == AN_UNLIKELY_RANDOM_VALUE)
	{
		//stop tracking functions while we work
		BlocRecursive = 1;

		char *PrevFunctionName = NULL;
		void* it;
		__asm mov DWORD PTR[it], ebp
		if(it)
		{
			void* rm[2];
			BOOL err = ReadProcessMemory(GetCurrentProcess(), it, (LPVOID)rm, sizeof(rm), NULL);
			if (err)
			{
				void *PrevFunctionAddr = (void*)rm[1];
				PrevFunctionName = GetSymStore()->GetFuncName(PrevFunctionAddr);
			}
		}

		LogFunctionEntrance(PrevFunctionName);

		//reenable function hooking
		BlocRecursive = AN_UNLIKELY_RANDOM_VALUE;
	}
	_asm {
		pop esi
		pop edi
		pop ebp
		pop edx
		pop ecx
		pop ebx
		pop eax
		ret
	}
}
#endif

void StartLogFunctionEntrances()
{
	BlocRecursive = AN_UNLIKELY_RANDOM_VALUE;
}

extern "C" void __declspec(naked) _cdecl _pexit(void)
{
	//	printf("Hook test - exit\n");
}

#ifdef DBGHELP_DOES_NOT_CRASH_PROCESS
//#include "DbgHelp.h"
//#include <WinBase.h>
#pragma comment(lib, "Dbghelp.lib")

void printStack(char *Output, int MaxSize)
{

	Output[0] = 0;
	unsigned int   i;
	void         * stack[100];
	unsigned short frames;
	SYMBOL_INFO  * symbol;
	HANDLE         process;

	process = GetCurrentProcess();
	SymInitialize(process, NULL, TRUE);
	frames = CaptureStackBackTrace(0, 100, stack, NULL);
	symbol = (SYMBOL_INFO *)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
	symbol->MaxNameLen = 255;
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

	for (i = 1; i < frames; i++)
	{
		SymFromAddr(process, (DWORD64)(stack[i]), 0, symbol);

		sprintf_s(Output, MaxSize, "%s%i: %s - 0x%0X\n", Output, frames - i - 1, symbol->Name, (unsigned int)symbol->Address);
	}

	free(symbol);
}

void GetLast_PenterFunctionName(TCHAR *Output, int MaxSize)
{

	Output[0] = 0;
	Output[1] = 0;

	void         * stack[100];
	HANDLE         process;
	bool ret;

	process = GetCurrentProcess();
	ret = SymInitialize(process, NULL, TRUE);
	if (CaptureStackBackTrace(2, 1, stack, NULL) != 1)
		return;

	SYMBOL_INFO  * symbol;
	symbol = (SYMBOL_INFO *)calloc(sizeof(SYMBOL_INFO) + 2 * MAX_SYM_NAME * sizeof(TCHAR), 1);
	symbol->MaxNameLen = MAX_SYM_NAME;
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
	//	SymFromAddr(process, (DWORD64)(ParentAddress), 0, symbol);
	ret = SymFromAddr(process, (DWORD64)(stack[0]), 0, symbol);
	if (ret == false)
	{
		GetLastError2();
	}
	else
	{
		if (symbol->Name[0] != 0)
			sprintf_s(Output, MaxSize, "%s - 0x%0I64X\n", symbol->Name, symbol->Address);
	}
	free(symbol);
}

void getFuncInfo(ADDR addr, TCHAR *funcName, unsigned int BuffSize)
{
	funcName[0] = 0;

	bool ret;
	ret = SymInitialize(GetCurrentProcess(), NULL, FALSE);
	//	TCHAR modShortNameBuf[MAX_PATH];

	MEMORY_BASIC_INFORMATION mbi;
	size_t buf_size = VirtualQuery((void*)addr, &mbi, sizeof(mbi));

	TCHAR moduleName[MAX_PATH];
	DWORD MFNret = GetModuleFileName((HMODULE)mbi.AllocationBase, moduleName, MAX_PATH);

	//	errno_t splitres = _splitpath_s(moduleName, NULL, 0, NULL, 0, modShortNameBuf, sizeof(modShortNameBuf), NULL, 0);

	ret = SymLoadModule(GetCurrentProcess(), NULL, moduleName, NULL, (DWORD)mbi.AllocationBase, 0);
	DWORD SymOpts = SymGetOptions();
	//	SymOpts |= SYMOPT_CASE_INSENSITIVE | SYMOPT_LOAD_LINES | SYMOPT_OMAP_FIND_NEAREST | SYMOPT_LOAD_ANYTHING | SYMOPT_INCLUDE_32BIT_MODULES | SYMOPT_DEBUG;
	ret = SymSetOptions(SymOpts & ~SYMOPT_UNDNAME);

	DWORD symDisplacement = 0;
	BYTE symbolBuffer[sizeof(IMAGEHLP_SYMBOL) + 1024];
	PIMAGEHLP_SYMBOL pSymbol = (PIMAGEHLP_SYMBOL)&symbolBuffer[0];
	// Following not per docs, but per example...
	pSymbol->SizeOfStruct = sizeof(symbolBuffer);
	pSymbol->MaxNameLength = 1023;

	void  *stack[1];
	if (CaptureStackBackTrace(2, 1, stack, NULL) != 1)
		return;

	if (!SymGetSymFromAddr(GetCurrentProcess(), (DWORD)stack[0], &symDisplacement, pSymbol))
	{
		GetLastError2();
		// Couldn't retrieve symbol (no debug info?)
		strcpy_s(funcName, BuffSize, "<unknown symbol>");
	}
	else
	{
		// Unmangle name, throwing away decorations
		// that don't affect uniqueness:
		if (0 == UnDecorateSymbolName(pSymbol->Name, funcName, BuffSize, UNDNAME_NO_MS_KEYWORDS | UNDNAME_NO_ACCESS_SPECIFIERS | UNDNAME_NO_FUNCTION_RETURNS | UNDNAME_NO_ALLOCATION_MODEL | UNDNAME_NO_ALLOCATION_LANGUAGE | UNDNAME_NO_MEMBER_TYPE))
			strcpy_s(funcName, BuffSize, pSymbol->Name);
	}
	SymUnloadModule(GetCurrentProcess(), (DWORD)mbi.AllocationBase);
	SymCleanup(GetCurrentProcess());
}
#endif