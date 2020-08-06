#include <windows.h>

static double               PCFreq = 0.0;         // CPU speed bazed timer. Must be divided to be the same on all PC
__int64                     CounterStart;

void StartCounter()
{
    LARGE_INTEGER li;
    if(!QueryPerformanceFrequency(&li))
	      return;

    PCFreq = double(li.QuadPart)/1000.0;

    QueryPerformanceCounter(&li);
    CounterStart = li.QuadPart;
}

double GetCounter()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return double(li.QuadPart-CounterStart)/PCFreq;
}

double GetCounterDiff( __int64 &CounterPrev )
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    double ret = (li.QuadPart-CounterPrev)/PCFreq;
    CounterPrev = li.QuadPart;
    return ret;
}

__int64 GetTick()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return li.QuadPart;
}

class DummyConstructor
{
public:
    DummyConstructor()
    {
        StartCounter();
    }
};

DummyConstructor StartTheCounter;