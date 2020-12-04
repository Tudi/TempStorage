#ifdef _SPEED_PROFILE_BUILD
#include <stdio.h>
extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavutil/imgutils.h>
	#include <libavutil/avstring.h>
	#include <libavutil/time.h>
	#include <libavutil/opt.h>
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
	#include <libswresample/swresample.h>
}
#include "DecoderInterface.h"
#include "Profiling.h"

int main()
{
	//check if we can open the file for reading data
	DecoderStateStore* ds = CreateDecoder("../TestSamples/Halo.Nightfall.2014.1080p.mHD.x264.HUN.READ.NFO-666.mkv");
	if (ds == NULL)
	{
		printf("Could not open file for decoding. Exiting\n");
		return -1;
	}
	//based on file info, print on console what we know about the codecs so far
	DumpStreamInfo(ds);
	int64_t StartTimeTotal = av_gettime();
	int LoopsRemainingToBreak = 500;
	int FramesDecoded = 0;
	//fill our raw output buffer queue. We can later fetch frames from it
	while (1)
	{
		if (LoopsRemainingToBreak <= 0)
			break;
		LoopsRemainingToBreak--;

		int64_t StartTime = av_gettime();
		//read a few packets from the stream file, decode them, format them into raw buffers
		//this could run it's own thread
		GenerateRawOutput(ds);

		//fet a buffer from the queue of raw data
		DecodedVideoFrame* vf = GetVideoBuffer(ds);
		DecodedAudioFrame* af = GetAudioBuffer(ds);

		if (vf == NULL && af == NULL)
			break;

		while (vf)
		{
			FramesDecoded++;
			FreeVideoBuffer(&vf);
			vf = GetVideoBuffer(ds);
		}
		while (af)
		{
			FreeAudioBuffer(&af);
			af = GetAudioBuffer(ds);
		}

		int64_t EndTime = av_gettime();
	}
	int64_t EndTimeTotal = av_gettime();
	int64_t TimeDiff = EndTimeTotal - StartTimeTotal;
	printf("Time required to decode the movie without render : %lld  /frame\n", TimeDiff / 1000);
	printf("Average FPS using null renderer : %f\n", (float)FramesDecoded / ((float)TimeDiff / (float)1000000));
	//all done. Kill the decoder
	DeleteDecoder(&ds);
	PrintProfilingResults();
	return 0;
}
#endif