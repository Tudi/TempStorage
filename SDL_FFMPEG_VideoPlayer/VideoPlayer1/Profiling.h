#pragma once

enum ProfileLocations
{
	PP_VIDEO_FRAME_DECODE,
	PP_VIDEO_FRAME_RESIZE,
	PP_VIDEO_FRAME_RENDER,
	PP_AUDIO_FRAME_DECODE,
	PP_AUDIO_FRAME_RESAMPLE,
	NUMER_OF_PROFILE_LOCATIONS
};

struct ProfilingStatusStore
{
	Uint64 Startstamp;
	Uint64 TotalSamples;
	Uint64 SampleCount;
	Uint64 LastDiff;	//if we want to print out every timed point value
	ProfileLocations Type;
	const char* Name;
};

enum ProfilePointLocationTypes
{
	START_PROFILING = 1,
	END_PROFILING = 0
};
//start the profiler for a specific point in code
void TriggerProfilePoint(ProfileLocations Type, ProfilePointLocationTypes StartLocation = START_PROFILING);
//get the duration of the code segment
Uint64 GetProfileTimeMS(ProfileLocations Type);
Uint64 GetProfileTimeUS(ProfileLocations Type);
//print out profiling statuses at the end of the code run
void PrintProfilingResults();
