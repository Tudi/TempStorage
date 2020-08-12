#pragma once

#define REMOVE_CALLSTACK_LOGGING
#define USE_DUMP_STATS_ON_EXIT

void ProfileLine(const char* File, const char* Func, int line, char* comment, int FuncStart);

#ifdef USE_DUMP_STATS_ON_EXIT
class AutoCleanupProfiler
{
public:
    AutoCleanupProfiler();
    ~AutoCleanupProfiler();
};
#endif

//create a stack variable. Destructor will auto call profiling stop for this function. Only available for c++ projects
class AutoCloseFunctionProfiling
{
public:
    AutoCloseFunctionProfiling(const char* File, const char* Func, int line, char* comment, int FuncStart);
    ~AutoCloseFunctionProfiling();
protected:
    const char* pFile;
    const char* pFunc;
    int pLine;
};