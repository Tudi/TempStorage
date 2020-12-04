#pragma once

enum ProfileLocations
{
	PP_READ_PACKETS,
	PP_DECODE_AUDIO,
	PP_DECODE_VIDEO,
	PP_VIDEO_COPY,
	NUMER_OF_PROFILE_LOCATIONS
};

struct ProfilingStatusStore
{
	int64_t Startstamp;
	int64_t TotalSamples;
	int64_t SampleCount;
	int64_t LastDiff;	//if we want to print out every timed point value
	ProfileLocations Type;
	const char* Name;
};

enum ProfilePointLocationTypes
{
	START_PROFILING = 1,
	END_PROFILING = 0
};

#ifdef _SPEED_PROFILE_BUILD
//start the profiler for a specific point in code
void TriggerProfilePoint(ProfileLocations Type, ProfilePointLocationTypes StartLocation = START_PROFILING);
//get the duration of the code segment
int64_t GetProfileTimeMS(ProfileLocations Type);
int64_t GetProfileTimeUS(ProfileLocations Type);
int64_t GetProfileTimeAvgUS(ProfileLocations Type);
//print out profiling statuses at the end of the code run
void PrintProfilingResults();
#else
	#define TriggerProfilePoint(a,b) {}
#endif