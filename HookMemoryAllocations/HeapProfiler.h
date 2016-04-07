#pragma once
#include <ostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <mutex>

const int backtraceSize = 64;
typedef unsigned long StackHash;

struct StackTrace{
	void *backtrace[backtraceSize];
	StackHash hash;

	StackTrace();
	void trace(); 
	void print(std::ostream &stream) const;
    void printOnlineFile(FILE *f) const;
};

class HeapProfiler{
public:
	void    malloc2(void *ptr, size_t size, const StackTrace &trace, int N);
	int     free2(void *ptr, const StackTrace &trace, int N);

	// Return a list of allocation sites (a particular stack trace) and the amount
	// of memory currently allocated by each site.
	void getAllocationSiteReport(std::vector<std::pair<StackTrace, size_t>> &allocs);
    bool HasData() { return !stackTraces.empty(); }
    void CheckNoBadPointerUsage();
private:
	std::mutex mutex;
	struct TraceInfo{
		StackTrace trace;
		std::unordered_map<void *, size_t> allocations;
        int AllocatorIndex; //there should be a chance that allocator and deallocator use the same index ( come from same DLL )
	};
	std::unordered_map<StackHash, TraceInfo>    stackTraces;
	std::unordered_map<void*, StackHash>        ptrs;

    std::unordered_map<StackHash, TraceInfo>    DeletedStackTraces;
    std::unordered_map<void*, StackHash>        DeletedPtrs;
};

void setupHeapProfiling();
const int numHooks = 128;
extern void *mallocHooksDllInst[numHooks];
extern void *FreeHooksDllInst[numHooks];
extern char mallocHooksDllInstNames[numHooks][500];
extern char FreeHooksDllInstNames[numHooks][500];
