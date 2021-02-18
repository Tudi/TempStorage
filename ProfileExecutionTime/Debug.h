#pragma once

#define REMOVE_CALLSTACK_LOGGING
#define USE_DUMP_STATS_ON_EXIT

//states defined for our callback
enum FunctionCallState
{
    CALL_STATE_NOT_SPECIFIED = 0,
    CALL_STATE_STARTED = 1,
    CALL_STATE_FINISHED = 2,
};

void ProfileLine(const char* File, const char* Func, int line, const char* comment, FunctionCallState FuncStart);

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
    AutoCloseFunctionProfiling(const char* File, const char* Func, int line, const char* comment, FunctionCallState FuncStart);
    ~AutoCloseFunctionProfiling();
protected:
    const char* pFile;
    char* pFunc;
    int pLine;
};