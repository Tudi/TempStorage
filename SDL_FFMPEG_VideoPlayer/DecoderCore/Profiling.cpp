#ifdef _SPEED_PROFILE_BUILD
#include <stdio.h>
#include <string.h>
extern "C"
{
	#include <libavutil/time.h>
}
#include "Profiling.h"

ProfilingStatusStore ProfilePoints[NUMER_OF_PROFILE_LOCATIONS];
static int ProfilingInitialized = 0;
int64_t StartedProfilingTimeStamp;

void InitializeProfiling()
{
	memset(ProfilePoints, 0, sizeof(ProfilePoints));

	ProfilePoints[PP_READ_PACKETS].Type = PP_READ_PACKETS;
	ProfilePoints[PP_READ_PACKETS].Name = "Read packets from stream";

	ProfilePoints[PP_DECODE_AUDIO].Type = PP_DECODE_AUDIO;
	ProfilePoints[PP_DECODE_AUDIO].Name = "Decode and resample audio";

	ProfilePoints[PP_DECODE_VIDEO].Type = PP_DECODE_VIDEO;
	ProfilePoints[PP_DECODE_VIDEO].Name = "Decode video";

	ProfilePoints[PP_VIDEO_COPY].Type = PP_VIDEO_COPY;
	ProfilePoints[PP_VIDEO_COPY].Name = "Convert from frame to buffer";

	StartedProfilingTimeStamp = av_gettime();

	ProfilingInitialized = 1;
}

void TriggerProfilePoint(ProfileLocations Type, ProfilePointLocationTypes StartLocation)
{
	if (ProfilingInitialized == 0)
		InitializeProfiling();

	if (Type >= NUMER_OF_PROFILE_LOCATIONS)
		return;

	int64_t TimeNow = av_gettime();

	if (StartLocation == START_PROFILING)
	{
		ProfilePoints[Type].Startstamp = TimeNow;
		return;
	}
	else if (ProfilePoints[Type].Startstamp <= TimeNow)
	{
		int64_t Diff = (TimeNow - ProfilePoints[Type].Startstamp);
		ProfilePoints[Type].TotalSamples += Diff;
		ProfilePoints[Type].SampleCount++;
		ProfilePoints[Type].LastDiff = Diff;
	}
}

int64_t GetProfileTimeMS(ProfileLocations Type)
{
	return GetProfileTimeUS(Type) / 1000;
}

int64_t GetProfileTimeUS(ProfileLocations Type)
{
	if (ProfilingInitialized == 0)
	{
		InitializeProfiling();
		return 0;
	}

	if (Type >= NUMER_OF_PROFILE_LOCATIONS)
		return 0;

	return ProfilePoints[Type].LastDiff;
}

void PrintProfilingResults()
{
	//get total time we used for profiling. !Not full run time!
	int64_t TimeNow = av_gettime();
	int64_t TotalTime = TimeNow - StartedProfilingTimeStamp;
	printf("Printing average runtimes in micro seconds for profiled locations : \n");
	for (int i = 0; i < NUMER_OF_PROFILE_LOCATIONS; i++)
	{
		if (ProfilePoints[i].SampleCount == 0)
			continue;
		uint64_t Avg = ProfilePoints[i].TotalSamples / ProfilePoints[i].SampleCount;
		float TimeSharePCT = (float)ProfilePoints[i].TotalSamples * (float)100.0 / (float)TotalTime;
		printf("\t% 40s:% 10lld % 10lld % 5.2f%%\n", ProfilePoints[i].Name, Avg, ProfilePoints[i].TotalSamples, TimeSharePCT);
	}
}

int64_t GetProfileTimeAvgUS(ProfileLocations Type)
{
	if (ProfilingInitialized == 0)
	{
		InitializeProfiling();
		return 0;
	}

	if (Type >= NUMER_OF_PROFILE_LOCATIONS)
		return 0;

	return ProfilePoints[Type].TotalSamples / ProfilePoints[Type].SampleCount;
}
#endif