#include "StdAfx.h"

ProfilingStatusStore ProfilePoints[NUMER_OF_PROFILE_LOCATIONS];
static int ProfilingInitialized = 0;
int64_t StartedProfilingTimeStamp;

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

	ProfilePoints[PP_STREAM_DECODE].Type = PP_STREAM_DECODE;
	ProfilePoints[PP_STREAM_DECODE].Name = "Time to read/decode 1 audio+video frame";

	ProfilePoints[PP_RENDER_TEXTURE_UPDATE].Type = PP_RENDER_TEXTURE_UPDATE;
	ProfilePoints[PP_RENDER_TEXTURE_UPDATE].Name = "Update texture for rendering";

	StartedProfilingTimeStamp = av_gettime();

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
	else if (ProfilePoints[Type].Startstamp <= TimeNow)
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
	int64_t TimeNow = av_gettime();
	int64_t TotalTime = TimeNow - StartedProfilingTimeStamp;
	int64_t TotalProfiledTime = 0;
	for (int i = 0; i < NUMER_OF_PROFILE_LOCATIONS; i++)
		TotalProfiledTime += ProfilePoints[i].TotalSamples;
	printf("Printing average runtimes in micro seconds for profiled locations : \n");
	for (int i = 0; i < NUMER_OF_PROFILE_LOCATIONS; i++)
	{
		if (ProfilePoints[i].SampleCount == 0)
			continue;
		Uint64 Avg = ProfilePoints[i].TotalSamples / ProfilePoints[i].SampleCount;
		float TimeSharePCT = (float)ProfilePoints[i].TotalSamples * (float)100.0 / (float)TotalTime;
		float TimeShareProfiledPCT = (float)ProfilePoints[i].TotalSamples * (float)100.0 / (float)TotalProfiledTime;
		printf("\t% 40s:% 10lld % 10lld % 5.2f%% % 5.2f%%\n", ProfilePoints[i].Name, Avg, ProfilePoints[i].TotalSamples, TimeSharePCT, TimeShareProfiledPCT);
	}
}

Uint64 GetProfileTimeAvgUS(ProfileLocations Type)
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