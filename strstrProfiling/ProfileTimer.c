#ifndef WINDOWS_BUILD
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <stdint.h>
#include <stddef.h>

double time_passed = -1;
struct timespec CounterStart;
void InitTimer()
{
}

void StartTimer()
{
	time_passed = -1;
	clock_gettime(CLOCK_MONOTONIC, &CounterStart);
}

double EndTimer()
{
	if (time_passed == -1)
	{
		struct timespec CounterEnd;
		clock_gettime(CLOCK_MONOTONIC, &CounterEnd);
		int64_t nsec = (int64_t)(CounterEnd.tv_sec - CounterStart.tv_sec)* (int64_t)1000000000UL + (int64_t)(CounterEnd.tv_nsec - CounterStart.tv_nsec);
		time_passed = (double)(nsec) / (double)1000000000;
	}
	return time_passed;
}
#else
#include <Windows.h>
double freq, time_passed = -1;
LARGE_INTEGER CounterStart;

void InitTimer()
{
	LARGE_INTEGER li;
	QueryPerformanceFrequency(&li);
	freq = (double)(li.QuadPart) / 1000.0;
}

void StartTimer()
{
	time_passed = -1;
	QueryPerformanceCounter(&CounterStart);
}

double EndTimer()
{
	if (time_passed == -1)
	{
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		time_passed = (double)(li.QuadPart - CounterStart.QuadPart) / freq;
	}
	return time_passed;
}
#endif