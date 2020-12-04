#if defined(_CONSOLE) && !defined(_SPEED_PROFILE_BUILD)
#include <stdio.h>
#include "DecoderInterface.h"
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

int main()
{
	//check if we can open the file for reading data
	DecoderStateStore* ds = CreateDecoder("sample_02.mp4");
	if (ds == NULL)
	{
		printf("Could not open file for decoding. Exiting\n");
		return -1;
	}
	//based on file info, print on console what we know about the codecs so far
	DumpStreamInfo(ds);
	int64_t StartTimeTotal = av_gettime();
	int FramesDecoded = 0;
	//fill our raw output buffer queue. We can later fetch frames from it
	while (1)
	{
		int64_t StartTime = av_gettime();
		FramesDecoded++;
		//read a few packets from the stream file, decode them, format them into raw buffers
		//this could run it's own thread
		GenerateRawOutput(ds);

		//fet a buffer from the queue of raw data
		DecodedVideoFrame* vf = GetVideoBuffer(ds);
		DecodedAudioFrame* af = GetAudioBuffer(ds);
		if (vf == NULL && af == NULL)
			break;

		//there are multiple small audio buffers compared to 1 big video frame
		int AudioBuffersPopped = 0;
		while (af != NULL && vf != NULL && af->TimeStamp < vf->TimeStamp)
		{
			DecodedAudioFrame* af2 = PeekAudioBuffer(ds, 0);
			if (af2 != NULL && af2->TimeStamp < vf->TimeStamp)
			{
				af = GetAudioBuffer(ds);
				AudioBuffersPopped++;
			}
			else
				break;
		}

		//there are multiple small audio buffers compared to 1 big video frame
		int VideoBuffersPopped = 0;
		while (af != NULL && vf != NULL && af->TimeStamp > vf->TimeStamp)
		{
			DecodedVideoFrame* vf2 = PeekVideoBuffer(ds, 0);
			if (vf2 != NULL && vf2->TimeStamp > vf->TimeStamp)
			{
				vf = GetVideoBuffer(ds);
				VideoBuffersPopped++;
			}
			else
				break;
		}

		if (vf == NULL || af == NULL)
			continue;

		int FrameNr = vf->frame_number;
		float vts = (float)vf->TimeStamp;
		float ats = (float)af->TimeStamp;

		//we are no longer using these buffers, throw them away
		FreeAudioBuffer(&af);
		FreeVideoBuffer(&vf);

		int64_t EndTime = av_gettime();
		//we popped enough audio and video buffers so they are in sync
		//if queues get congested, the stream reader will block
		printf("Ready to display frame number=%d %d %d video timestamp=%f audio timestamp=%f Duration(us)=%d\n", FrameNr, VideoBuffersPopped, AudioBuffersPopped, vts, ats, (int)(EndTime - StartTime));
	}
	int64_t EndTimeTotal = av_gettime();
	int64_t TimeDiff = EndTimeTotal - StartTimeTotal;
	printf("Time required to decode the movie without render : %lld ms  %lld us/frame\n", TimeDiff / 1000, TimeDiff / FramesDecoded);
	//test seeking
	printf("Starting seek test\n");
	SeekInputStream(ds, 30);
	DecodedVideoFrame* vf = GetVideoBuffer(ds);
	DecodedAudioFrame* af = GetAudioBuffer(ds);
	if (vf != NULL && af != NULL)
	{
		printf("After seek : frame number=%d video timestamp=%f audio timestamp=%f\n", vf->frame_number, (float)vf->TimeStamp, (float)af->TimeStamp);
		//we are no longer using these buffers, throw them away
		FreeAudioBuffer(&af);
		FreeVideoBuffer(&vf);
	}
	//all done. Kill the decoder
	DeleteDecoder(&ds);
	return 0;
}
#endif