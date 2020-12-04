#pragma once

/*
Open a file to be used by a player UI
Should provide :
	- info about audio stream
	- info about video stream
	- ability to seek in stream
	- fetch next decoded audio block
	- fetch next decoded video block
	- dump all video packets and only decode audio on request
	- dump all audio packets and only decode video on request
	- force audio / video to sync to less than 1 PTS difference
Known issues :
	- audio / video sync : sometimes more video / audio packets come and decoded content starts to lag behind
				need to cache a few packets until we get some data for the "other" stream
	- seek : after seek, next packets might not be decodable. There should be a few frames thrown away until
				we manage to decode a video / audio frame. After that we should throw away a few frames to
				syncronize audio and video
How it works :
	- open a file for reading
	- intialize internal states
	- create thread that reads packets from file and pushes to stream queues. Role of the queues is to be able to create work for both decoders
	- create audio / video decoder thread that will read packet queues and produce raw buffer queues
*/

struct AVFrame;
struct AVFormatContext;
struct AVStream;
struct AVCodecContext;
struct CircularBuffer;
struct DecoderStateStore;

struct DecodedAudioFrame
{
	unsigned char* audio_data;
	int             audio_size;
	double          TimeStamp;
};

struct DecodedVideoFrame
{
	//    unsigned char* video_data;
	AVFrame* video_frame;
	int     width;
	int     height;
	int     frame_number;
	int     pic_type;
	int     key_frame;
	double  TimeStamp;
};

/***************************************************

		Core Decoder functions

***************************************************/
//create a decoder based on the File Name. Can return NULL if File could not be opened
DecoderStateStore* CreateDecoder(const char* FileName);
//should run in a separate thread to continually produce raw outputs
void GenerateRawOutput(DecoderStateStore* dss);
//destroy the decoder store and release allocated memory
void DeleteDecoder(DecoderStateStore** dss);
//we hand out buffers that the player will use. After usage we expect player to give us back the buffer
void FreeAudioBuffer(DecodedAudioFrame** buff);
//we hand out buffers that the player will use. After usage we expect player to give us back the buffer
void FreeVideoBuffer(DecodedVideoFrame** buff);
//seek in input stream to a specific absolute position given in seconds
//int SeekInputStream(DecoderStateStore* dss, double TimeStamp);
// when you need a precise seek. Note that this will be slower than "good enough" see
int SeekPreciseInputStream(DecoderStateStore* dss, double TimeStamp);
//fetch a decoded raw wav buffer from the decoder
DecodedAudioFrame* GetAudioBuffer(DecoderStateStore* dss);
//fetch a decoded raw YUV buffer from the decoder
DecodedVideoFrame* GetVideoBuffer(DecoderStateStore* dss);
//fetch a decoded raw wav buffer from the decoder
DecodedAudioFrame* PeekAudioBuffer(DecoderStateStore* dss, int index);
//fetch a decoded raw YUV buffer from the decoder
DecodedVideoFrame* PeekVideoBuffer(DecoderStateStore* dss, int index);

/***************************************************
	Helper functions for a UI player
***************************************************/

//tell ffmpeg to dump the stream related info
void DumpStreamInfo(DecoderStateStore* dss);
//get the format context
AVFormatContext* GetFormatContext(DecoderStateStore* dss);
AVCodecContext* GetAudioContext(DecoderStateStore* dss);
AVCodecContext* GetVideoContext(DecoderStateStore* dss);
double GetFPS(DecoderStateStore* dss);