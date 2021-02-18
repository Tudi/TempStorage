#include <windows.h>
#include <map>
#include <vector>
#include <algorithm>
#include "Debug.h"

static double               PCFreq = 0.0;         // CPU speed bazed timer. Must be divided to be the same on all PC
__int64                     CounterStart;

#define PERFCOUNTERMAXSTACKSIZE     1000
struct ThreadDebugContext
{
    __int64                 CounterPrev;
    std::map<int, __int64>   FuncCallState;
    std::map<int, __int64>   FuncCallStart;      // !! does not support recursive calls !!
    std::map<int, __int64>   FuncCallDepthStart; // should be as above, but supports recursive calls. !! you need to make sure call depth will not get messed up !!
    std::map<int, __int64>   FuncCallSum;
    std::map<int, __int64>   FuncCallSumRaw;     // substract call times below us
    std::map<int, __int64>   FuncCallCount;
    std::map<int, char*>    FuncCallNames;
    FILE* FileDebug;
//    int                     PrevFuncCallNameCRC;
    int                     CallDepth;
    __int64                  NextSaveStamp;
    std::map<int, __int64>   FuncCallMissingEnd;
    std::map<int, __int64>   FuncCallMissingBegin;
    unsigned int            CallStack[PERFCOUNTERMAXSTACKSIZE];
};

std::map<DWORD, ThreadDebugContext*> ThreadsDebugged;

//get our CPU TickCount
void StartCounter()
{
    LARGE_INTEGER li;
    if (!QueryPerformanceFrequency(&li))
        return;

    PCFreq = double(li.QuadPart) / 1000.0;

    QueryPerformanceCounter(&li);
    CounterStart = li.QuadPart;
}

double GetCounter()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return double(li.QuadPart - CounterStart) / PCFreq;
}

double GetCounterDiff(__int64& CounterPrev)
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    double ret = (li.QuadPart - CounterPrev) / PCFreq;
    CounterPrev = li.QuadPart;
    return ret;
}

__int64 GetTick()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return li.QuadPart;
}

//generic implementation of hashing table
static unsigned int crc32_tab[] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3,	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de,	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,	0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5,	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,	0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940,	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,	0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

//generic implementation of hashing alg
int crc32(int crc, const void* buf, size_t size)
{
    const char* p;

    p = (const char*)buf;
    crc = crc ^ ~0U;

    while (size--)
        crc = crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);

    return crc ^ ~0U;

}
void PrintReport();

//callback function we used to instrument the CPP files
void ProfileLine(const char* File, const char* Func, int line, const char* comment, FunctionCallState FuncStart = CALL_STATE_NOT_SPECIFIED)
{
    __int64 TickNow = GetTick();    //skip recording time spent in this func
    __int64 CallDiff = 0;
    DWORD ThreadId = GetCurrentThreadId();
    ThreadDebugContext* Context;
    if (ThreadsDebugged.find(ThreadId) == ThreadsDebugged.end())
    {
        Context = new ThreadDebugContext;
        Context->CallDepth = 0;
        Context->CounterPrev = 0;
//        Context->PrevFuncCallNameCRC = 0;
        Context->FileDebug = 0;
        Context->NextSaveStamp = 0;
        ThreadsDebugged[ThreadId] = Context;
    }
    else
        Context = ThreadsDebugged[ThreadId];
#ifndef REMOVE_CALLSTACK_LOGGING
    if (Context->FileDebug == 0)
    {
        char FileName[500];
//        sprintf_s(FileName, 500, "d:/temp/perf_%d.txt", ThreadId);
        sprintf_s(FileName, 500, "perf_%d.txt", ThreadId);
#endif
        //first time initialize
        if (Context->CounterPrev == 0)
        {
            Context->CounterPrev = 1;
#ifndef REMOVE_CALLSTACK_LOGGING
            Context->FileDebug = fopen(FileName, "wt");
#endif
            if (CounterStart == 0)
            {
                StartCounter();
                CounterStart = 1;
            }
            //get the EXE name
#ifndef REMOVE_CALLSTACK_LOGGING
            TCHAR szFileName[MAX_PATH];
            szFileName[0] = 0;
            DWORD ret = GetModuleFileName(NULL, szFileName, MAX_PATH);
            fprintf(Context->FileDebug, "%ls \n", szFileName);
#endif
        }
#ifndef REMOVE_CALLSTACK_LOGGING
        //we keep closing to file in case we want to take a look at it while running
        else
            Context->FileDebug = fopen(FileName, "at");
    }
    if (Context->FileDebug != 0)
#endif
    {
        //Sanity check. I would not do this :P
        if (comment == NULL)
            comment = "";
//        int namecrc = crc32(0, Func, strlen(Func)) + line;
        int namecrc = crc32(0, Func, strlen(Func));
        if (FuncStart == CALL_STATE_STARTED)
        {

            //first time call for this function name ? Add the string to our name list. Duping is not a must, but i fear the destructors
            if (Context->FuncCallStart.find(namecrc) == Context->FuncCallStart.end())
                Context->FuncCallNames[namecrc] = _strdup(Func);

            if (Context->FuncCallState[namecrc] == CALL_STATE_STARTED)
                Context->FuncCallMissingEnd[namecrc] = namecrc;

            Context->FuncCallState[namecrc] = CALL_STATE_STARTED; //Just entered the function
            Context->FuncCallStart[namecrc] = TickNow;
            Context->FuncCallDepthStart[Context->CallDepth] = TickNow;

#ifndef REMOVE_CALLSTACK_LOGGING
            for (int i = 0; i < Context->CallDepth; i++)
                fprintf(Context->FileDebug, "\t");
            fprintf(Context->FileDebug, "%f - %f : %s - %s - %d - %s\n", GetCounter(), 0.0f, File, Func, line, comment);
#endif

//            Context->PrevFuncCallNameCRC = namecrc;
            Context->CallDepth++;
            //try to track call depth
            if (Context->CallDepth < _countof(Context->CallStack))
                Context->CallStack[Context->CallDepth] = namecrc;
        }
        else if (FuncStart == CALL_STATE_FINISHED)
        {
            //in case of error when function start is missing and only function end is present
            if (Context->FuncCallStart.find(namecrc) == Context->FuncCallStart.end())
            {
                Context->FuncCallNames[namecrc] = _strdup(Func);
                Context->FuncCallMissingBegin[namecrc] = namecrc;
            }

            if (Context->FuncCallState[namecrc] == CALL_STATE_FINISHED && Context->CallStack[Context->CallDepth - 1] != namecrc)
                Context->FuncCallMissingBegin[namecrc] = namecrc;
            //            if( FuncCallState[namecrc] == CALL_STATE_STARTED )    //recursive calls are possible
            {
                Context->FuncCallState[namecrc] = CALL_STATE_FINISHED; //exited the function
                CallDiff = TickNow - Context->FuncCallStart[namecrc];
                if (CallDiff < 0)
                    CallDiff = 0;

                if (Context->CallDepth < _countof(Context->CallStack))
                {
                    Context->FuncCallSumRaw[namecrc] += CallDiff;
                    if (Context->CallDepth > 1)
                    {
                        unsigned int ParentFunctionName = Context->CallStack[Context->CallDepth - 1];
                        Context->FuncCallSumRaw[ParentFunctionName] -= CallDiff;
                    }
                }

                //recursive calls are not supported by named starts
 //               __int64 CallDiff2 = TickNow - Context->FuncCallDepthStart[Context->CallDepth - 1];
//                if (CallDiff2 < CallDiff)
//                    CallDiff = CallDiff2;

                Context->FuncCallSum[namecrc] += CallDiff;
                Context->FuncCallCount[namecrc]++;

#ifndef REMOVE_CALLSTACK_LOGGING
                if (Context->CallDepth > 0)
                    for (int i = 0; i < Context->CallDepth - 1; i++)
                        fprintf(Context->FileDebug, "\t");

                double CallDiffd = double(CallDiff) / PCFreq;
                fprintf(Context->FileDebug, "%f - %f : %s - %s - %d - %s -  %d - %d\n", GetCounter(), CallDiffd, File, Func, line, comment, (int)Context->FuncCallSum[namecrc], (int)Context->FuncCallCount[namecrc]);
#endif
                //                if( PrevFuncCallNameCRC == namecrc || PrevFuncCallNameCRC == 0 )
                {
                    Context->CallDepth--;
//                    Context->PrevFuncCallNameCRC = 0;
                }
                //                if( Context->CallDepth <= 0 )
#ifndef USE_DUMP_STATS_ON_EXIT
                {
                    if (Context->NextSaveStamp <= TickNow)
                    {
                        Context->NextSaveStamp = TickNow + (long long)(PCFreq * 50.0);
                        PrintReport();
                    }
                }
#endif
            }
        }
#ifndef REMOVE_CALLSTACK_LOGGING
        else
        {
            if (Context->CallDepth > 0)
                for (int i = 0; i < Context->CallDepth - 1; i++)
                    fprintf(Context->FileDebug, "\t");
            fprintf(Context->FileDebug, "%f - %f : %s - %s - %d - %s\n", GetCounter(), GetCounterDiff(Context->CounterPrev), File, Func, line, comment);
        }
        fclose(Context->FileDebug);
        Context->FileDebug = 0;
#endif
    }
    Context->CounterPrev = GetTick();    //skip recording time spent in this func;
}

void PrintReport()
{
    if (ThreadsDebugged.empty())
	{
		printf("No threads were detected\n");
        return;
	}
    char FileName[500];

//    sprintf_s(FileName, 500, "d:/temp/perf_report_%d.txt", (int)GetCurrentProcessId());
    sprintf_s(FileName, 500, "perf_report_%d.txt", (int)GetCurrentProcessId());
    FILE* FileDebug = fopen(FileName, "wt");
    if (!FileDebug)
	{
		printf("Could not open destination file for reporting\n");
        return;
	}
    for (std::map<DWORD, ThreadDebugContext*>::iterator citr = ThreadsDebugged.begin(); citr != ThreadsDebugged.end(); citr++)
    {
        ThreadDebugContext* Context = citr->second;

        if (Context->FuncCallNames.empty())
            continue;

        fprintf(FileDebug, "Thread Id : %d\n", citr->first);

        //std::vector<std::pair<int, __int64>> FuncCallCountSorted;
        std::vector<std::pair<int, __int64>> FuncCallSumSorted;
        __int64 SumTicks = 0;
        //for (std::map<int, __int64>::iterator itr = Context->FuncCallCount.begin(); itr != Context->FuncCallCount.end(); itr++)
            //FuncCallCountSorted.push_back(std::make_pair(itr->first, itr->second));
        for (std::map<int, __int64>::iterator itr = Context->FuncCallSum.begin(); itr != Context->FuncCallSum.end(); itr++)
        {
            FuncCallSumSorted.push_back(std::make_pair(itr->first, itr->second));
            SumTicks += itr->second;
        }

        auto ComparePred = [](std::pair<int, __int64> const& a, std::pair<int, __int64> const& b)
        {
            return a.second < b.second;
        };
        //std::sort(FuncCallCountSorted.begin(), FuncCallCountSorted.end(), ComparePred);
        std::sort(FuncCallSumSorted.begin(), FuncCallSumSorted.end(), ComparePred);

        fprintf(FileDebug, "Sum time spent : %lld\n", (long long)((double)SumTicks / PCFreq));
        //order by time spent in func
        fprintf(FileDebug, "=======================================================================================================\n");
        fprintf(FileDebug, "% 80s % 10s % 10s % 20s % 20s\n", "Function name", "call count", "Time share %", "Time MS", "Self Time");
        for (std::vector<std::pair<int, __int64>>::iterator itr = FuncCallSumSorted.begin(); itr != FuncCallSumSorted.end(); itr++)
        {
            unsigned int NameCRC32 = itr->first;
            float TimeShare = Context->FuncCallSum[NameCRC32] * 100.0f / (float)SumTicks;
            float MSTime = (float)(Context->FuncCallSum[NameCRC32] / PCFreq);
            float MSTimeRaw = (float)(Context->FuncCallSumRaw[NameCRC32] / PCFreq);
            char* FuncName = Context->FuncCallNames[NameCRC32];
            fprintf(FileDebug, "% 80s % 10d % 10.2f % 20.2f % 20.2f\n", FuncName, (int)Context->FuncCallCount[NameCRC32], TimeShare, MSTime, MSTimeRaw);
        }
        if(Context->FuncCallMissingBegin.empty() == false)
            fprintf(FileDebug, "Function names that are missing 'begin'\n");
        for (std::map<int, __int64>::iterator itr = Context->FuncCallMissingBegin.begin(); itr != Context->FuncCallMissingBegin.end(); itr++)
            fprintf(FileDebug, "%s\n", Context->FuncCallNames[itr->first]);
        if (Context->FuncCallMissingEnd.empty() == false)
            fprintf(FileDebug, "Function names that are missing 'end'\n");
        for (std::map<int, __int64>::iterator itr = Context->FuncCallMissingEnd.begin(); itr != Context->FuncCallMissingEnd.end(); itr++)
            fprintf(FileDebug, "%s\n", Context->FuncCallNames[itr->first]);
    }
    fclose(FileDebug);
}

#ifdef USE_DUMP_STATS_ON_EXIT
AutoCleanupProfiler::AutoCleanupProfiler()
{
	StartCounter();
}
AutoCleanupProfiler::~AutoCleanupProfiler()
{
	PrintReport();
}
AutoCleanupProfiler tStartProfiler;
#endif

AutoCloseFunctionProfiling::AutoCloseFunctionProfiling(const char* File, const char* Func, int line, const char* comment, FunctionCallState FuncStart = CALL_STATE_NOT_SPECIFIED)
{
	int NewFuncLen=strlen(File)+strlen(Func)+10;
	char *NewFuncName=(char*)malloc(NewFuncLen);
//	sprintf(NewFuncName,"%s:%s:%d",File,Func,line);//not all compilers include class name in func name
	sprintf(NewFuncName,"%s:%d",Func,line);//classes with multiple functions with same name can be tracked separately
	pFile = File;
	pFunc = NewFuncName;
	pLine = line;
	ProfileLine(pFile, pFunc, pLine, comment, FuncStart);
}
AutoCloseFunctionProfiling::~AutoCloseFunctionProfiling()
{
	ProfileLine(pFile, pFunc, pLine, "End", CALL_STATE_FINISHED);
	free(pFunc);
}