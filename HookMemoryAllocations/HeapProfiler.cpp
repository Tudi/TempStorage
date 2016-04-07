#include "StdAfx.h"

#include "HeapProfiler.h"
#include <Windows.h>
#include <stdio.h>
#include "dbghelp.h"

#include <algorithm>
#include <iomanip>
#include "MemoryFence.h"

StackTrace::StackTrace() : hash(0)
{
	memset(backtrace, 0, sizeof(void*)*backtraceSize);
}

void StackTrace::trace()
{
	CaptureStackBackTrace(0, backtraceSize, backtrace, &hash);
}

void StackTrace::print(std::ostream &stream) const 
{
	HANDLE process = GetCurrentProcess();

	const int MAXSYMBOLNAME = 128 - sizeof(IMAGEHLP_SYMBOL);
	char symbol64_buf[sizeof(IMAGEHLP_SYMBOL) + MAXSYMBOLNAME] = {0};
	IMAGEHLP_SYMBOL *symbol = reinterpret_cast<IMAGEHLP_SYMBOL*>(symbol64_buf);
	symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
	symbol->MaxNameLength = MAXSYMBOLNAME - 1;

	// Print out stack trace. Skip the first frame (that's our hook function.)
	for(size_t i = 1; i < backtraceSize; ++i)
    { 
		if(backtrace[i])
        {
			// Output stack frame symbols if available.
			if(SymGetSymFromAddr(process, (DWORD64)backtrace[i], 0, symbol)){

				stream << "    " << symbol->Name;

				// Output filename + line info if available.
				IMAGEHLP_LINE lineSymbol = {0};
				lineSymbol.SizeOfStruct = sizeof(IMAGEHLP_LINE);
				DWORD displacement;
				if(SymGetLineFromAddr(process, (DWORD64)backtrace[i], &displacement, &lineSymbol))
					stream << "    " << lineSymbol.FileName << ":" << lineSymbol.LineNumber;

				stream << "    (" << std::setw(sizeof(void*)*2) << std::setfill('0') << backtrace[i] <<  ")\n";
			}
            else
				stream << "    <no symbol> " << "    (" << std::setw(sizeof(void*)*2) << std::setfill('0') << backtrace[i] <<  ")\n";
		}
        else
			break;
	}
}

void StackTrace::printOnlineFile(FILE *f) const
{
    HANDLE process = GetCurrentProcess();

    const int MAXSYMBOLNAME = 128 - sizeof(IMAGEHLP_SYMBOL);
    char symbol64_buf[sizeof(IMAGEHLP_SYMBOL) + MAXSYMBOLNAME] = { 0 };
    IMAGEHLP_SYMBOL *symbol = reinterpret_cast<IMAGEHLP_SYMBOL*>(symbol64_buf);
    symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
    symbol->MaxNameLength = MAXSYMBOLNAME - 1;

    // Print out stack trace. Skip the first frame (that's our hook function.)
    for (size_t i = 1; i < backtraceSize; ++i)
    {
        if (backtrace[i])
        {
            // Output stack frame symbols if available.
            if (SymGetSymFromAddr(process, (DWORD64)backtrace[i], 0, symbol))
            {
                fprintf(f, "%s:", symbol->Name);

                // Output filename + line info if available.
                IMAGEHLP_LINE lineSymbol = { 0 };
                lineSymbol.SizeOfStruct = sizeof(IMAGEHLP_LINE);
                DWORD displacement;
                if (SymGetLineFromAddr(process, (DWORD64)backtrace[i], &displacement, &lineSymbol))
                    fprintf(f, "%s:%d:", lineSymbol.FileName, lineSymbol.LineNumber);

                fprintf(f, "%p\n", backtrace[i]);
            }
            else
                fprintf(f, "%p\n", backtrace[i]);
        }
        else
            break;
    }
    fprintf(f, "\n");
}

#define ONLY_REPORT_ALLOCATIONS
#ifdef ONLY_REPORT_ALLOCATIONS
void HeapProfiler::malloc2(void *ptr, size_t size, const StackTrace &trace, int N)
{
    std::lock_guard<std::mutex> lk(mutex);
    if (stackTraces.find(trace.hash) == stackTraces.end())
    {
        stackTraces[trace.hash].trace = trace;
        stackTraces[trace.hash].AllocatorIndex = N;
    }
    stackTraces[trace.hash].allocations[ptr] = size;
    ptrs[ptr] = trace.hash;
}

int HeapProfiler::free2(void *ptr, const StackTrace &trace, int N)
{
    std::lock_guard<std::mutex> lk(mutex);
    auto it = ptrs.find(ptr);
    if (it != ptrs.end())
    {
        StackHash stackHash = it->second;
        auto SizeItr = stackTraces[stackHash].allocations.find(ptr);
        size_t Size = SizeItr->second;
        stackTraces[stackHash].allocations.erase(SizeItr);
        ptrs.erase(it);

        char FileName[500];
        sprintf_s(FileName, sizeof(FileName), "d:/temp/Allocations_%p.txt", FreeHooksDllInst[N]);
        FILE *f = fopen(FileName, "at");
        if (f)
        {
            if (FreeHooksDllInst[N] != mallocHooksDllInst[stackTraces[stackHash].AllocatorIndex])
                fprintf(f, "Not getting deallocated from same DLL?\n");
            fprintf(f, "%s is deallocating a block of size %d at %p. Alloc Index %d. Dealloc index %d\n", FreeHooksDllInstNames[N], (int)Size, ptr, stackTraces[stackHash].AllocatorIndex, N);
            stackTraces[stackHash].trace.printOnlineFile(f);
            trace.printOnlineFile(f);
            fprintf(f, "\n");
            fclose(f);
        }

        return 1;
    }
    return 0;
}
#else
void HeapProfiler::malloc2(void *ptr, size_t size, const StackTrace &trace, int N)
{
	std::lock_guard<std::mutex> lk(mutex);
    if (stackTraces.find(trace.hash) == stackTraces.end())
    {
        stackTraces[trace.hash].trace = trace;
        stackTraces[trace.hash].allocations[ptr] = size;
        ptrs[ptr] = trace.hash;
    }
}

int HeapProfiler::free2(void *ptr, const StackTrace &trace, int N)
{
	std::lock_guard<std::mutex> lk(mutex);
	auto it = ptrs.find(ptr);
	if(it != ptrs.end())
    {
#if CHECK_WRITE_AFTER_DEALLOC == 1
        int size = stackTraces[trace.hash].allocations[ptr];
        InitDeallocatedBlock(ptr, size);
        DeletedStackTraces[trace.hash].trace = trace;
        DeletedStackTraces[trace.hash].allocations[ptr] = size;
        DeletedPtrs[ptr] = trace.hash;
        ASSERT( CheckFence(ptr,size) == 0 );
#endif
        StackHash stackHash = it->second;
		stackTraces[stackHash].allocations.erase(ptr); 
		ptrs.erase(it);
        return 1;
	}
    return 0;
}
#endif
void HeapProfiler::getAllocationSiteReport(std::vector<std::pair<StackTrace, size_t>> &allocs)
{
	std::lock_guard<std::mutex> lk(mutex);
	allocs.clear();

	// For each allocation point.
	for(auto &traceInfo : stackTraces)
    {
		// Sum up the size of all the allocations made.
		size_t sumOfAlloced = 0;
		for(auto &alloc : traceInfo.second.allocations)
			sumOfAlloced += alloc.second;

		// Add to alloation site report.
		allocs.push_back(std::make_pair(traceInfo.second.trace, sumOfAlloced));
	}
}

void HeapProfiler::CheckNoBadPointerUsage()
{
    std::lock_guard<std::mutex> lk(mutex);

    // For each allocation point.
    for (auto &traceInfo : DeletedStackTraces)
    {
        for (auto &alloc : traceInfo.second.allocations)
            if (CheckDeallocatedBlock(alloc.first,alloc.second))
            {
                printf("bad pointer usage at ");
                ASSERT(false);
            }
    }
}