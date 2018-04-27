#pragma once

#include <map>
#include <vector>

//load a map file to be able to search for an address to get function name
// compile an EXE with /MAP flag to generate map files

#define SKIP_LOAD_FUNCTIONS_WITH_DECORATED_NAMES	1

class SymbolInfoStore
{
public:
	SymbolInfoStore(unsigned int pBaseAddr, char *pFuncName, char *pModule)
	{
		BaseAddr = pBaseAddr;
		FuncName = pFuncName;
		Module = pModule;
	}
	unsigned int BaseAddr;
	char *FuncName;
	char *Module;
};

class SymbolLookupService
{
public:
	SymbolLookupService();
	char *GetFuncName(void *addr);
private:
	char							*BaseAdress;
	char							*AllocationBase;
	std::vector<SymbolInfoStore*>	FunctionNames;
	std::map<void *, char*>			FunctionNamesFound;
};

//for the sake of lazy loading
SymbolLookupService *GetSymStore();