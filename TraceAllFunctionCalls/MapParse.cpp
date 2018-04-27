#include <windows.h>
#include <stdio.h>
#include <ImageHlp.h>
#include "MapParse.h"

static SymbolLookupService *AddrNameStore = NULL;
SymbolLookupService *GetSymStore()
{
	if (AddrNameStore == NULL)
		AddrNameStore = new SymbolLookupService;
	return AddrNameStore;
}

size_t ReadLine(FILE *f, char *out, size_t OutSize)
{
	//rad normal line
	size_t ind = 0;
	while (fread(&out[ind], 1, 1, f) && out[ind] != '\n')
		ind++;
	out[ind] = 0;
	return ind;
}

int GetCharPos(char *str, char Needle, int start, size_t Maxlen)
{
	for (int i = start; str[i] != 0 && i < (int)Maxlen; i++)
		if (str[i] == Needle)
			return i;
	return -1;
}

int SkipToNextData(char *str, int start, size_t Maxlen)
{
	//skip valid characters
	int i = start;
	for (; str[i] != 0 && i < (int)Maxlen; i++)
		if (str[i] == ' ' || str[i] == '\t' || str[i] == '\n')
			break;
	//skip non valid characters
	for (; str[i] != 0 && i < (int)Maxlen; i++)
		if (!(str[i] == ' ' || str[i] == '\t' || str[i] == '\n'))
			break;
	return i;
}

SymbolLookupService::SymbolLookupService()
{
	MEMORY_BASIC_INFORMATION mbi;
	size_t buf_size = VirtualQuery((void*)SkipToNextData, &mbi, sizeof(mbi));
	if (buf_size == 0)
	{
		printf("Error : VirtualQuery failed. Premature Exit.\n");
		return;
	}
	BaseAdress = (char*)mbi.BaseAddress;
	AllocationBase = (char*)mbi.AllocationBase;

	TCHAR moduleName[MAX_PATH];
	DWORD MFNret = GetModuleFileName((HMODULE)mbi.AllocationBase, moduleName, MAX_PATH);
	if (MFNret == 0)
	{
		printf("Error : Could not get module name. Premature Exit.\n");
		return;
	}

	//change the exe to map
	size_t LastChar = strlen(moduleName);
	moduleName[LastChar - 3] = 'm';
	moduleName[LastChar - 2] = 'a';
	moduleName[LastChar - 1] = 'p';
	FILE *MapFile = NULL;
	errno_t OpenRes = fopen_s( &MapFile, moduleName, "rt");
	unsigned int ImageBaseAddress;
	if (MapFile)
	{
		char FileLine[2000];

		//read until we find the public function export table with line ___ImageBase
		do
		{
			size_t LineLen = ReadLine(MapFile, FileLine, sizeof(FileLine));
//			if (strstr(FileLine, " Address ") )
			if (strstr(FileLine, " ___ImageBase ") )
			{
				//extract RVA+Base
				int RVABase = SkipToNextData(FileLine, 1, LineLen);
				RVABase = SkipToNextData(FileLine, RVABase, LineLen);
				ImageBaseAddress = strtoul(&FileLine[RVABase], NULL, 16);
				break;
			}
		} while (!feof(MapFile));

		//read all addresses and function names. Example of line : 0002:000015b0       _main                      004125b0 f   main.obj
		do
		{
			size_t LineLen = ReadLine(MapFile, FileLine, sizeof(FileLine));

			//end of file reached
			if (LineLen == 0 || FileLine[0] == '\n')
				break;	

			//non virtualized address of the function
			int AddrStart = GetCharPos(FileLine, ':', 0, LineLen) + 1;
//			unsigned int FunctionBinaryAddr = strtoul(&FileLine[AddrStart], NULL, 16);

			//name of the function
			int FuncNameStart = SkipToNextData(FileLine, AddrStart, LineLen);
			int FuncNameEnd = GetCharPos(FileLine, ' ', FuncNameStart, LineLen) - 1;			
			char funcName[2000];
//			if (0 == UnDecorateSymbolName(&FileLine[FuncNameStart], funcName, sizeof(funcName), UNDNAME_NO_MS_KEYWORDS | UNDNAME_NO_ACCESS_SPECIFIERS | UNDNAME_NO_FUNCTION_RETURNS | UNDNAME_NO_ALLOCATION_MODEL | UNDNAME_NO_ALLOCATION_LANGUAGE | UNDNAME_NO_MEMBER_TYPE))
			{
				for (int i = (FuncNameEnd - FuncNameStart); i >=0 ; i--)
					funcName[i] = FileLine[i + FuncNameStart];
				funcName[FuncNameEnd - FuncNameStart + 1] = 0;
			}

			// base addre + rel add
			int RebasedAddr = SkipToNextData(FileLine, FuncNameEnd, LineLen);
			unsigned int FunctionBinaryAddr = strtoul(&FileLine[RebasedAddr], NULL, 16);
			if (FunctionBinaryAddr < ImageBaseAddress)
			{
				printf("Error : Function pointer is before image base address. Premature Exit.\n");
				return;
			}
			FunctionBinaryAddr -= ImageBaseAddress;

			//module name. probably an obj file
			char ModuleName[2000];
			int ModuleNameStart = GetCharPos(FileLine, ' ', RebasedAddr, LineLen) + 5;
			strcpy_s(ModuleName, sizeof(ModuleName), &FileLine[ModuleNameStart]);

			FunctionNames.push_back( new SymbolInfoStore(FunctionBinaryAddr, _strdup(funcName), _strdup(ModuleName)) );
		} while (!feof(MapFile));

		fclose(MapFile);
	}
}

char *SymbolLookupService::GetFuncName(void *addr)
{
	//maybe this is not the first time we search for this function
	std::map<void *, char*>::iterator i = FunctionNamesFound.find(addr);
	if (i != FunctionNamesFound.end())
		return (*i).second;

	//if we load additional DLLs, their virtualization / reallocation might be different from the main function, we need to calculate this every time
	MEMORY_BASIC_INFORMATION mbi;
	size_t buf_size = VirtualQuery(addr, &mbi, sizeof(mbi));
	unsigned __int64 BaseAddr2 = (unsigned __int64)addr - (unsigned __int64)mbi.AllocationBase;

	//find the closest smallest pointer
	char *FuncName = NULL;
	for (std::vector<SymbolInfoStore*>::iterator i = FunctionNames.begin(); i != FunctionNames.end(); i++)
	{
		if ((unsigned __int64)BaseAddr2 < (unsigned __int64)(*i)->BaseAddr)
		{
			if (i != FunctionNames.begin())
				i--;
			FuncName = (*i)->FuncName;
			break;
		}
	}

	//cache it, in case we ask for it again
	FunctionNamesFound[addr] = FuncName;
	return FuncName;
}
