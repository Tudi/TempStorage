#include "StdAfx.h"

ProfilingStatusStore ProfilePoints[NUMER_OF_PROFILE_LOCATIONS];
static int ProfilingInitialized = 0;

void InitializeProfiling()
{
	memset(ProfilePoints, 0, sizeof(ProfilePoints));

	ProfilePoints[PP_VIDEO_FRAME_DECODE].Type = PP_VIDEO_FRAME_DECODE;
	ProfilePoints[PP_VIDEO_FRAME_DECODE].Name = "Time to decode 1 video frame";

	ProfilePoints[PP_VIDEO_FRAME_RESIZE].Type = PP_VIDEO_FRAME_DECODE;
	ProfilePoints[PP_VIDEO_FRAME_RESIZE].Name = "Time to resize 1 video frame";

	ProfilePoints[PP_VIDEO_FRAME_RENDER].Type = PP_VIDEO_FRAME_RENDER;
	ProfilePoints[PP_VIDEO_FRAME_RENDER].Name = "Time to render 1 video frame";

	ProfilePoints[PP_AUDIO_FRAME_DECODE].Type = PP_AUDIO_FRAME_DECODE;
	ProfilePoints[PP_AUDIO_FRAME_DECODE].Name = "Time to decode 1 audio frame";

	ProfilePoints[PP_AUDIO_FRAME_RESAMPLE].Type = PP_AUDIO_FRAME_RESAMPLE;
	ProfilePoints[PP_AUDIO_FRAME_RESAMPLE].Name = "Time to resample 1 audio frame";

	ProfilingInitialized = 1;
}

void TriggerProfilePoint(ProfileLocations Type, ProfilePointLocationTypes StartLocation)
{
	if (ProfilingInitialized == 0)
		InitializeProfiling();

	if (Type >= NUMER_OF_PROFILE_LOCATIONS)
		return;

	Uint64 TimeNow = av_gettime();

	if (StartLocation == START_PROFILING)
	{
		ProfilePoints[Type].Startstamp = TimeNow;
		return;
	}

	if (ProfilePoints[Type].Startstamp <= TimeNow)
	{
		Uint64 Diff = (TimeNow - ProfilePoints[Type].Startstamp);
		ProfilePoints[Type].TotalSamples += Diff;
		ProfilePoints[Type].SampleCount++;
		ProfilePoints[Type].LastDiff = Diff;
	}
}

Uint64 GetProfileTimeMS(ProfileLocations Type)
{
	return GetProfileTimeUS(Type) / 1000;
}

Uint64 GetProfileTimeUS(ProfileLocations Type)
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
	printf("Printing average runtimes in micro seconds for profiled locations : \n");
	for (int i = 0; i < NUMER_OF_PROFILE_LOCATIONS; i++)
	{
		if (ProfilePoints[i].SampleCount == 0)
			continue;
		Uint64 Avg = ProfilePoints[i].TotalSamples / ProfilePoints[i].SampleCount;
		printf("\t%s:%lld\n", ProfilePoints[i].Name, Avg);
	}
}