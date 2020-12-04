#include <stdlib.h>
#include <string.h>
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
#ifdef LINUX
#include <unistd.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif
#include "Profiling.h"

void mySleep(int sleepMs)
{
#ifdef LINUX
    usleep(sleepMs * 1000);   // usleep takes sleep time in us (1 millionth of a second)
#endif
#ifdef _WIN32
    Sleep(sleepMs);
#endif
}
#include "DecoderInterface.h"
#include "CircularBuffer.h"


struct DecoderStateStore
{
    AVFormatContext* pFormatCtx; // store format parameters
    char* FileName;
    /**
     * Audio Stream.
     */
    int                 audioStream;
    AVStream* audio_st;
    AVCodecContext* audio_ctx;

    CircularBuffer* AudioPackets;
    CircularBuffer* AudioFrames;

    /**
     * Video Stream.
     */
    int                 videoStream;
    AVStream* video_st;
    AVCodecContext* video_ctx;

    CircularBuffer* VideoPackets;
    CircularBuffer* VideoFrames;
    int             ContentStartedSeek;
    int             DecoderRunningInParallel;
    int             DecoderWaitingSeek;
    int             DecoderIsSeeking; // should use a mutex here
    int             EndOfFileReached;

    // for a precise seeking, it would be great, where the previous key_frame was. We will generate statistics to seek before the next key frame and read until we reach the desired frame
    int64_t         PrevKeyFrameStamp;
    int64_t         SumKeyFrameDist;
    int64_t         NumberOfKeyFrameDistances;
    int64_t         MaxKeyFrameDistance;

    AVFrame* aFrame;
};

/**
 * Retrieves the AVCodec and initializes the AVCodecContext for the given AVStream
 * index. In case of AVMEDIA_TYPE_AUDIO codec type, it sets the desired audio specs,
 * opens the audio device and starts playing.
 *
 * @param   videoState      the global VideoState reference used to save info
 *                          related to the media being played.
 * @param   stream_index    the stream index obtained from the AVFormatContext.
 *
 * @return                  < 0 in case of error, 0 otherwise.
 */
int stream_component_open(DecoderStateStore* dss, int stream_index)
{
    // retrieve file I/O context
    AVFormatContext* pFormatCtx = dss->pFormatCtx;

    // check the given stream index is valid
    if (stream_index < 0 || stream_index >= (int)pFormatCtx->nb_streams)
    {
        printf("Invalid stream index.");
        return -1;
    }

    // retrieve codec for the given stream index
    AVCodec* codec = NULL;
    codec = avcodec_find_decoder(pFormatCtx->streams[stream_index]->codecpar->codec_id);
    if (codec == NULL)
    {
        printf("Unsupported codec.\n");
        return -1;
    }

    // retrieve codec context
    AVCodecContext* codecCtx = NULL;
    codecCtx = avcodec_alloc_context3(codec);
    int ret = avcodec_parameters_to_context(codecCtx, pFormatCtx->streams[stream_index]->codecpar);
    if (ret != 0)
    {
        printf("Could not copy codec context.\n");
        return -1;
    }

    // initialize the AVCodecContext to use the given AVCodec
    if (avcodec_open2(codecCtx, codec, NULL) < 0)
    {
        printf("Unsupported codec.\n");
        return -1;
    }

    // set up the global VideoState based on the type of the codec obtained for
    // the given stream index
    switch (codecCtx->codec_type)
    {
    case AVMEDIA_TYPE_AUDIO:
    {
        // set VideoState audio related fields
        dss->audioStream = stream_index;
        dss->audio_st = pFormatCtx->streams[stream_index];
        dss->audio_ctx = codecCtx;
    }break;

    case AVMEDIA_TYPE_VIDEO:
    {
        // set VideoState video related fields
        dss->videoStream = stream_index;
        dss->video_st = pFormatCtx->streams[stream_index];
        dss->video_ctx = codecCtx;
    }break;

    default:
    {
        // nothing to do
    }break;
    }

    return 0;
}

DecoderStateStore* CreateDecoder(const char* FileName)
{
    DecoderStateStore* dss = (DecoderStateStore*)malloc(sizeof(DecoderStateStore));
    memset(dss, 0, sizeof(DecoderStateStore));
    int ret = 0;
    AVFormatContext* pFormatCtx = NULL;

    // file I/O context: demuxers read a media file and split it into chunks of data (packets)
    ret = avformat_open_input(&pFormatCtx, FileName, NULL, NULL);
    if (ret < 0)
    {
        printf("Could not open file %s.\n", FileName);
        goto fail;
    }

    // reset stream indexes
    dss->videoStream = -1;
    dss->audioStream = -1;

    // set the AVFormatContext for the global VideoState reference
    dss->pFormatCtx = pFormatCtx;

    // read packets of the media file to get stream information
    ret = avformat_find_stream_info(pFormatCtx, NULL);
    if (ret < 0)
    {
        printf("Could not find stream information: %s.\n", FileName);
        goto fail;
    }

    // loop through the streams that have been found
    for (unsigned int i = 0; i < pFormatCtx->nb_streams; i++)
    {
        // look for the video stream
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && dss->videoStream < 0)
            dss->videoStream = i;

        // look for the audio stream
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && dss->audioStream < 0)
            dss->audioStream = i;
    }

    // return with error in case no video stream was found
/*    if (dss->videoStream == -1)
    {
        printf("Could not find video stream.\n");
        goto fail;
    }*/
    if (dss->videoStream >= 0)
    {
        // open video stream component codec
        ret = stream_component_open(dss, dss->videoStream);

        // check video codec was opened correctly
        if (ret < 0)
        {
            printf("Could not open video codec.\n");
            goto fail;
        }
    }

    // return with error in case no audio stream was found
/*    if (dss->audioStream == -1)
    {
        printf("Could not find audio stream.\n");
        goto fail;
    }*/
    if (dss->audioStream >= 0)
    {
        // open audio stream component codec
        ret = stream_component_open(dss, dss->audioStream);

        // check audio codec was opened correctly
        if (ret < 0)
        {
            printf("Could not open audio codec.\n");
            goto fail;
        }
    }

    // check both the audio and video codecs were correctly retrieved
    if (dss->videoStream < 0 && dss->audioStream < 0)
    {
        printf("Could not open codecs: %s.\n", FileName);
        goto fail;
    }

    //should have around 10 seconds of buffering here. Based on youtube caching
    dss->AudioPackets = CreateCircularBuffer(1000); // at least 750 frames
    dss->VideoPackets = CreateCircularBuffer(500); // 25 fps, at least 250 frames

    dss->AudioFrames = CreateCircularBuffer(8);
    dss->VideoFrames = CreateCircularBuffer(4);

    dss->ContentStartedSeek = 0;
    dss->DecoderRunningInParallel = 0;
    dss->DecoderWaitingSeek = 0;
    dss->DecoderIsSeeking = 0;
    dss->EndOfFileReached = 0;

    dss->PrevKeyFrameStamp = -1;
    dss->SumKeyFrameDist = 0;
    dss->NumberOfKeyFrameDistances = 0;
    dss->MaxKeyFrameDistance = -1;

    dss->aFrame = av_frame_alloc();
    return dss;

fail:
    if (pFormatCtx)
        avformat_close_input(&pFormatCtx);
    free(dss);
    return NULL;
}

void FlushCircularBufferContents(DecoderStateStore* dss)
{
    //flush packet queue
    while (1)
    {
        AVPacket* packet = NULL;
        int ret = GetBuffer(dss->VideoPackets, (void**)&packet, NULL);
        if (packet != NULL)
        {
            av_packet_unref(packet);
            av_packet_free(&packet);
        }
        else
            break;
    }
    //flush packet queue
    while (1)
    {
        AVPacket* packet = NULL;
        int ret = GetBuffer(dss->AudioPackets, (void**)&packet, NULL);
        if (packet != NULL)
        {
            av_packet_unref(packet);
            av_packet_free(&packet);
        }
        else
            break;
    }
    //flush raw data also
    while (1)
    {
        DecodedVideoFrame* vf = GetVideoBuffer(dss);
        if (vf != NULL)
            FreeVideoBuffer(&vf);
        else
            break;
    }
    while (1)
    {
        DecodedAudioFrame* af = GetAudioBuffer(dss);
        if (af != NULL)
            FreeAudioBuffer(&af);
        else
            break;
    }
}

void DeleteDecoder(DecoderStateStore** dss)
{
    if (dss == NULL)
        return;
    if ((*dss)->audio_ctx)
    {
        avcodec_flush_buffers((*dss)->audio_ctx);
        avcodec_free_context(&(*dss)->audio_ctx);
    }
    if ((*dss)->video_ctx)
    {
        avcodec_flush_buffers((*dss)->video_ctx);
        avcodec_free_context(&(*dss)->video_ctx);
    }
    avformat_close_input(&(*dss)->pFormatCtx);
    FlushCircularBufferContents(*dss);
    av_frame_free(&(*dss)->aFrame);

    free(*dss);
    *dss = NULL;
}

typedef struct AudioResamplingState
{
    SwrContext* swr_ctx;
    int64_t in_channel_layout;
    uint64_t out_channel_layout;
    int out_nb_channels;
    int out_linesize;
    int in_nb_samples;
    int64_t out_nb_samples;
    int64_t max_out_nb_samples;
    uint8_t** resampled_data;
    int resampled_data_size;

} AudioResamplingState;

/**
 * Initializes an instance of the AudioResamplingState Struct with the given
 * parameters.
 *
 * @param   channel_layout  the audio codec context channel layout to be used.
 *
 * @return                  the allocated and initialized AudioResamplingState
 *                          struct instance.
 */
AudioResamplingState* getAudioResampling(uint64_t channel_layout)
{
    AudioResamplingState* audioResampling = (AudioResamplingState*)av_mallocz(sizeof(AudioResamplingState));

    audioResampling->swr_ctx = swr_alloc();
    audioResampling->in_channel_layout = channel_layout;
    audioResampling->out_channel_layout = AV_CH_LAYOUT_STEREO;
    audioResampling->out_nb_channels = 0;
    audioResampling->out_linesize = 0;
    audioResampling->in_nb_samples = 0;
    audioResampling->out_nb_samples = 0;
    audioResampling->max_out_nb_samples = 0;
    audioResampling->resampled_data = NULL;
    audioResampling->resampled_data_size = 0;

    return audioResampling;
}

/**
 * Resamples the audio data retrieved using FFmpeg before playing it.
 *
 * @param   videoState          the global VideoState reference.
 * @param   decoded_audio_frame the decoded audio frame.
 * @param   out_sample_fmt      audio output sample format (e.g. AV_SAMPLE_FMT_S16).
 * @param   out_buf             audio output buffer.
 *
 * @return                      the size of the resampled audio data.
 */
static int audio_resampling(DecoderStateStore* dss, AVFrame* decoded_audio_frame, enum AVSampleFormat out_sample_fmt, uint8_t* out_buf)
{
    // get an instance of the AudioResamplingState struct
    AudioResamplingState* arState = getAudioResampling(dss->audio_ctx->channel_layout);

    if (!arState->swr_ctx)
    {
        printf("swr_alloc error.\n");
        return -1;
    }

    // get input audio channels
    arState->in_channel_layout = (dss->audio_ctx->channels ==
        av_get_channel_layout_nb_channels(dss->audio_ctx->channel_layout)) ?
        dss->audio_ctx->channel_layout :
        av_get_default_channel_layout(dss->audio_ctx->channels);

    // check input audio channels correctly retrieved
    if (arState->in_channel_layout <= 0)
    {
        printf("in_channel_layout error.\n");
        return -1;
    }

    // set output audio channels based on the input audio channels
    if (dss->audio_ctx->channels == 1)
        arState->out_channel_layout = AV_CH_LAYOUT_MONO;
    else if (dss->audio_ctx->channels == 2)
        arState->out_channel_layout = AV_CH_LAYOUT_STEREO;
    else
        arState->out_channel_layout = AV_CH_LAYOUT_SURROUND;

    // retrieve number of audio samples (per channel)
    arState->in_nb_samples = decoded_audio_frame->nb_samples;
    if (arState->in_nb_samples <= 0)
    {
        printf("in_nb_samples error.\n");
        return -1;
    }

    // Set SwrContext parameters for resampling
    av_opt_set_int(arState->swr_ctx, "in_channel_layout", arState->in_channel_layout, 0);

    // Set SwrContext parameters for resampling
    av_opt_set_int(arState->swr_ctx, "in_sample_rate", dss->audio_ctx->sample_rate, 0);

    // Set SwrContext parameters for resampling
    av_opt_set_sample_fmt(arState->swr_ctx, "in_sample_fmt", dss->audio_ctx->sample_fmt, 0);

    // Set SwrContext parameters for resampling
    av_opt_set_int(arState->swr_ctx, "out_channel_layout", arState->out_channel_layout, 0);

    // Set SwrContext parameters for resampling
    av_opt_set_int(arState->swr_ctx, "out_sample_rate", dss->audio_ctx->sample_rate, 0);

    // Set SwrContext parameters for resampling
    av_opt_set_sample_fmt(arState->swr_ctx, "out_sample_fmt", out_sample_fmt, 0);

    // initialize SWR context after user parameters have been set
    int ret = swr_init(arState->swr_ctx);;
    if (ret < 0)
    {
        printf("Failed to initialize the resampling context.\n");
        return -1;
    }

    arState->max_out_nb_samples = arState->out_nb_samples = av_rescale_rnd(
        arState->in_nb_samples,
        dss->audio_ctx->sample_rate,
        dss->audio_ctx->sample_rate,
        AV_ROUND_UP
    );

    // check rescaling was successful
    if (arState->max_out_nb_samples <= 0)
    {
        printf("av_rescale_rnd error.\n");
        return -1;
    }

    // get number of output audio channels
    arState->out_nb_channels = av_get_channel_layout_nb_channels(arState->out_channel_layout);

    // allocate data pointers array for arState->resampled_data and fill data
    // pointers and linesize accordingly
    ret = av_samples_alloc_array_and_samples(
        &arState->resampled_data,
        &arState->out_linesize,
        arState->out_nb_channels,
        (int)arState->out_nb_samples,
        out_sample_fmt,
        0
    );

    // check memory allocation for the resampled data was successful
    if (ret < 0)
    {
        printf("av_samples_alloc_array_and_samples() error: Could not allocate destination samples.\n");
        return -1;
    }

    // retrieve output samples number taking into account the progressive delay
    arState->out_nb_samples = av_rescale_rnd(
        swr_get_delay(arState->swr_ctx, dss->audio_ctx->sample_rate) + arState->in_nb_samples,
        dss->audio_ctx->sample_rate,
        dss->audio_ctx->sample_rate,
        AV_ROUND_UP
    );

    // check output samples number was correctly rescaled
    if (arState->out_nb_samples <= 0)
    {
        printf("av_rescale_rnd error\n");
        return -1;
    }

    if (arState->out_nb_samples > arState->max_out_nb_samples)
    {
        // free memory block and set pointer to NULL
        av_free(arState->resampled_data[0]);

        // Allocate a samples buffer for out_nb_samples samples
        ret = av_samples_alloc(
            arState->resampled_data,
            &arState->out_linesize,
            arState->out_nb_channels,
            (int)arState->out_nb_samples,
            out_sample_fmt,
            1
        );

        // check samples buffer correctly allocated
        if (ret < 0)
        {
            printf("av_samples_alloc failed.\n");
            return -1;
        }

        arState->max_out_nb_samples = arState->out_nb_samples;
    }

    if (arState->swr_ctx)
    {
        // do the actual audio data resampling
        ret = swr_convert(
            arState->swr_ctx,
            arState->resampled_data,
            (int)arState->out_nb_samples,
            (const uint8_t**)decoded_audio_frame->data,
            decoded_audio_frame->nb_samples
        );

        // check audio conversion was successful
        if (ret < 0)
        {
            printf("swr_convert_error.\n");
            return -1;
        }

        // get the required buffer size for the given audio parameters
        arState->resampled_data_size = av_samples_get_buffer_size(
            &arState->out_linesize,
            arState->out_nb_channels,
            ret,
            out_sample_fmt,
            1
        );

        // check audio buffer size
        if (arState->resampled_data_size < 0)
        {
            printf("av_samples_get_buffer_size error.\n");
            return -1;
        }
    }
    else
    {
        printf("swr_ctx null error.\n");
        return -1;
    }

    // copy the resampled data to the output buffer
    memcpy(out_buf, arState->resampled_data[0], arState->resampled_data_size);

    /*
     * Memory Cleanup.
     */
    if (arState->resampled_data)
    {
        // free memory block and set pointer to NULL
        av_freep(&arState->resampled_data[0]);
    }

    av_freep(&arState->resampled_data);
    arState->resampled_data = NULL;

    if (arState->swr_ctx)
    {
        // free the allocated SwrContext and set the pointer to NULL
        swr_free(&arState->swr_ctx);
    }

    return arState->resampled_data_size;
}

void ThreadDecodeAudioPackets(DecoderStateStore* dss)
{
    if (CanStoreMore(dss->AudioFrames) == 0)
        return;
    // allocate AVPacket to read from the audio PacketQueue (audioq)
    AVPacket* avPacket = NULL;

    // infinite loop: read AVPackets from the audio PacketQueue, decode them into
    // audio frames, resample the obtained frame and update the audio buffer
    for (;;)
    {
        //if we can not store more raw outputs, there is no point to loop anymore
        if (CanStoreMore(dss->AudioFrames) == 0)
            break;

        // get more audio AVPacket
        int ret = GetBuffer(dss->AudioPackets, (void**)&avPacket, NULL);
        //was unable to obtain a buffer
        if (ret < 0 || avPacket == NULL)
            break;

        //        TriggerProfilePoint(PP_AUDIO_FRAME_DECODE);

                // give the decoder raw compressed data in an AVPacket
        ret = avcodec_send_packet(dss->audio_ctx, avPacket);

        // check if we obtained an AVPacket from the audio PacketQueue
        while (ret >= 0)
        {
            // get decoded output data from decoder
            int ret = avcodec_receive_frame(dss->audio_ctx, dss->aFrame);

            // check the decoder needs more AVPackets to be sent
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                break;
            else if (ret < 0)
            {
                printf("avcodec_receive_frame decoding error.\n");
                return;
            }

            if (CanStoreMore(dss->AudioFrames) == 0)
                continue;

            //            TriggerProfilePoint(PP_AUDIO_FRAME_DECODE, END_PROFILING);
                        // if we decoded an entire audio frame
                        // apply audio resampling to the decoded frame
            //            TriggerProfilePoint(PP_AUDIO_FRAME_RESAMPLE);
            int data_size = 0;
            unsigned char* audio_buf = (unsigned char*)malloc(dss->aFrame->nb_samples * 2 * 2 * 2);
            data_size = audio_resampling(dss, dss->aFrame, AV_SAMPLE_FMT_S16, audio_buf);
            //            TriggerProfilePoint(PP_AUDIO_FRAME_RESAMPLE, END_PROFILING);
            //            assert(data_size <= buf_size);

                        // no data yet, get more frames
            if (data_size <= 0)
            {
                free(audio_buf);
                continue;
            }
            DecodedAudioFrame* af = (DecodedAudioFrame*)malloc(sizeof(DecodedAudioFrame));
            memset(af, 0, sizeof(DecodedAudioFrame));
            af->audio_data = audio_buf;
            af->audio_size = data_size;
            af->TimeStamp = (double)avPacket->pts * av_q2d(dss->audio_st->time_base);
            ret = AddBuffer(dss->AudioFrames, af, NULL);
            if (ret < 0)
            {
                free(audio_buf);
                free(af);
            }
            av_frame_unref(dss->aFrame);
        }
        //we only get here if we failed to decode the packet
        av_packet_unref(avPacket);
        av_packet_free(&avPacket);
    }
}

void ThreadDecodeVideoPackets(DecoderStateStore* dss)
{
    if (CanStoreMore(dss->VideoFrames) == 0)
        return;

    // allocate an AVPacket to be used to retrieve data from the videoq.
    AVPacket* packet = NULL;

    for (;;)
    {
        //if we can not store more raw outputs, there is no point to loop anymore
        if (CanStoreMore(dss->VideoFrames) == 0)
            break;

        // get a packet from the video PacketQueue
        int ret = GetBuffer(dss->VideoPackets, (void**)&packet, NULL);
        //was unable to obtain a buffer
        if (ret < 0 || packet == NULL)
            break;

        // give the decoder raw compressed data in an AVPacket
        ret = avcodec_send_packet(dss->video_ctx, packet);
        if (ret < 0)
        {
            printf("Error sending video packet for decoding.\n");
            return; // this will throw 1 error after seeking, but it's ok
        }

        while (ret >= 0)
        {
            // get decoded output data from decoder
            AVFrame* vFrame = av_frame_alloc();
            ret = avcodec_receive_frame(dss->video_ctx, vFrame);

            // check an entire frame was decoded
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            {
                break;
            }
            else if (ret < 0)
            {
                printf("Error while decoding.\n");
                return;
            }
            //statistics regarding key frame intervals
            int64_t PacketPTS = packet->pts;
            if (PacketPTS < 0 || PacketPTS == AV_NOPTS_VALUE)
                PacketPTS = packet->dts;
            if (vFrame->key_frame)
            {
                if (dss->PrevKeyFrameStamp >= 0)
                {
                    int64_t PTSDiff = PacketPTS - dss->PrevKeyFrameStamp;
                    dss->SumKeyFrameDist += PTSDiff;
                    dss->NumberOfKeyFrameDistances++;
                    if(PTSDiff > dss->MaxKeyFrameDistance)
                        dss->MaxKeyFrameDistance = PTSDiff;
                }
                dss->PrevKeyFrameStamp = PacketPTS;
            }
            TriggerProfilePoint(PP_VIDEO_COPY, START_PROFILING);
            DecodedVideoFrame* af = (DecodedVideoFrame*)malloc(sizeof(DecodedVideoFrame));
            memset(af, 0, sizeof(DecodedVideoFrame));
            af->video_frame = vFrame;
            af->width = dss->video_ctx->width;
            af->height = dss->video_ctx->height;
            af->TimeStamp = (double)PacketPTS * av_q2d(dss->video_st->time_base);
            af->frame_number = vFrame->coded_picture_number;
            af->pic_type = vFrame->pict_type;
            af->key_frame = vFrame->key_frame;
            int ret = AddBuffer(dss->VideoFrames, af, NULL);
            if (ret < 0)
                free(af);
        }

        // wipe the packet
        av_packet_unref(packet);
        av_packet_free(&packet);
    }
}

void ThreadReadPackets(DecoderStateStore* dss, int ExitOnFullQueue = 1)
{
    TriggerProfilePoint(PP_READ_PACKETS, START_PROFILING);
    //you should not try to run this in more than one thread. In case you did, need to protect you from doing bad stuff
    //todo : swap this to a mutex later
    while (dss->DecoderRunningInParallel)
        mySleep(1);
    dss->DecoderRunningInParallel = 1;
    // main decode loop: read in a packet and put it on the right queue
    for (;;)
    {
        if (dss->ContentStartedSeek)
        {
            dss->DecoderWaitingSeek = 1;
            mySleep(1);
            continue;
        }
        int CanStoreMorePackets = 0;
        if (dss->videoStream >= 0)
            CanStoreMorePackets += CanStoreMore(dss->AudioPackets);
        else
            CanStoreMorePackets += 1;
        if (dss->audioStream >= 0)
            CanStoreMorePackets += CanStoreMore(dss->VideoPackets);
        else
            CanStoreMorePackets += 1;
        // check audio and video packets queues size
        if (CanStoreMorePackets != 2)
        {
            if (ExitOnFullQueue == 1)
                break;
            // wait for audio and video queues to decrease size
            mySleep(1);
            continue;
        }

        // read data from the AVFormatContext by repeatedly calling av_read_frame()
        AVPacket* packet = av_packet_alloc();
        if (packet == NULL)
            break;
        av_init_packet(packet);
        packet->data = NULL;
        packet->size = 0;

        int ret = av_read_frame(dss->pFormatCtx, packet);
        if (ret < 0)
        {
            av_packet_unref(packet);
            av_packet_free(&packet);
            if (ret == AVERROR_EOF)
            {
                dss->EndOfFileReached = 1;
                break;
            }
            else if (dss->pFormatCtx->pb->error == 0)
            {
                // no read error; wait for user input
                mySleep(1);
                continue;
            }
            else
                // exit for loop in case of error
                break;
        }

        // put the packet in the appropriate queue
        if (packet->stream_index == dss->videoStream)
        {
            ret = AddBuffer(dss->VideoPackets, packet, NULL);
            if (ret != 0)
                printf("Packet thrown away! Should never happen!!\n");
            //            ThreadDecodeVideoPackets(dss);
        }
        else if (packet->stream_index == dss->audioStream)
        {
            ret = AddBuffer(dss->AudioPackets, packet, NULL);
            if (ret != 0)
                printf("Packet thrown away! Should never happen!!\n");
            //            ThreadDecodeAudioPackets(dss);
        }
        else
        {
            // otherwise free the memory
            av_packet_unref(packet);
            av_packet_free(&packet);
        }
    }
    TriggerProfilePoint(PP_READ_PACKETS, END_PROFILING);
    //maybe we are full of unprocessed packets
    TriggerProfilePoint(PP_DECODE_VIDEO, START_PROFILING);
    ThreadDecodeVideoPackets(dss);
    TriggerProfilePoint(PP_DECODE_VIDEO, END_PROFILING);
    TriggerProfilePoint(PP_DECODE_AUDIO, START_PROFILING);
    ThreadDecodeAudioPackets(dss);
    TriggerProfilePoint(PP_DECODE_AUDIO, END_PROFILING);
    //we are done with this function. Feel free to call again
    dss->DecoderRunningInParallel = 0;
}

void GenerateRawOutput(DecoderStateStore* dss)
{
    //fill the packet queue
    ThreadReadPackets(dss);
}

void DumpStreamInfo(DecoderStateStore* dss)
{
    av_dump_format(dss->pFormatCtx, 0, dss->FileName, 0);
}

void FetchMoreDecodedData(DecoderStateStore* dss)
{
    if (dss->DecoderRunningInParallel == 0)
        GenerateRawOutput(dss);
    else
    {
        //check if we have some raw data
        while (dss->audioStream >= 0 && PeekAudioBuffer(dss, 1) == NULL && dss->EndOfFileReached == 0)
            mySleep(1);
        while (dss->videoStream >= 0 && PeekVideoBuffer(dss, 1) == NULL && dss->EndOfFileReached == 0)
            mySleep(1);
    }
}
/*
int SeekInputStream(DecoderStateStore* dss, double TimeStamp)
{
    //do not seek while seeking
    while (dss->DecoderIsSeeking)
        mySleep(1);
    dss->DecoderIsSeeking = 1;

    dss->ContentStartedSeek = 1;

    //wait for decoder thread to pause it's execution
    while (dss->DecoderRunningInParallel != 0 && dss->DecoderWaitingSeek == 0)
        mySleep(1);

    if (TimeStamp < 0)
        TimeStamp = 0;

    int retv = 0, reta = 0;
    int64_t seek_target = (int64_t)TimeStamp;
    DecodedVideoFrame* vf2 = PeekVideoBuffer(dss, 0);
    DecodedAudioFrame* af2 = PeekAudioBuffer(dss, 0);
    int SeekFlags = 0;
    if ((vf2 != NULL && TimeStamp < vf2->TimeStamp) || (af2 != NULL && TimeStamp < af2->TimeStamp))
        SeekFlags = AVSEEK_FLAG_BACKWARD;

    if (dss->videoStream >= 0)
    {
        int video_stream_index = dss->videoStream;
        //scale human seconds to file stored time units
        int64_t seek_target_v = av_rescale(seek_target, dss->pFormatCtx->streams[video_stream_index]->time_base.den, dss->pFormatCtx->streams[video_stream_index]->time_base.num);
        //do the actual seeking
        retv = av_seek_frame(dss->pFormatCtx, video_stream_index, seek_target_v, SeekFlags);
    }

    if (dss->audioStream >= 0)
    {
        int audio_stream_index = dss->audioStream;
        int64_t seek_target_a = av_rescale(seek_target, dss->pFormatCtx->streams[audio_stream_index]->time_base.den, dss->pFormatCtx->streams[audio_stream_index]->time_base.num);
        //do the actual seeking
        reta = av_seek_frame(dss->pFormatCtx, audio_stream_index, seek_target_a, SeekFlags);
    }

    //seek failed, nothing we can do
    if (retv < 0 && reta < 0)
    {
        dss->ContentStartedSeek = 0;
        dss->DecoderWaitingSeek = 0;
        return -1;
    }

    //dump old content
    FlushCircularBufferContents(dss);

    dss->ContentStartedSeek = 0;
    dss->DecoderWaitingSeek = 0;

    //this will execute in a blocking way
    FetchMoreDecodedData(dss);

    //if we have data, we should sync audio and video packets
    DecodedAudioFrame* af = PeekAudioBuffer(dss, 0);
    DecodedVideoFrame* vf = PeekVideoBuffer(dss, 0);
    //if audio is lagging behind the video
    if (af != NULL && vf != NULL && af->TimeStamp < vf->TimeStamp)
    {
        while (af != NULL && vf != NULL && af->TimeStamp < vf->TimeStamp)
        {
            af = GetAudioBuffer(dss);
            if (dss->DecoderRunningInParallel == 0)
                GenerateRawOutput(dss);
            af = PeekAudioBuffer(dss, 0);
        }
    }
    else
    {
        //video is lagging behind audio
        while (af != NULL && vf != NULL && af->TimeStamp > vf->TimeStamp)
        {
            vf = GetVideoBuffer(dss);
            if (dss->DecoderRunningInParallel == 0)
                GenerateRawOutput(dss);
            vf = PeekVideoBuffer(dss, 0);
        }
    }
    dss->DecoderIsSeeking = 0;
    //seek went ok
    return 0;
}*/

void FreeAudioBuffer(DecodedAudioFrame** buff)
{
    if (*buff == NULL)
        return;
    if ((*buff)->audio_data)
    {
        free((*buff)->audio_data);
        (*buff)->audio_data = NULL;
    }
    free(*buff);
    *buff = NULL;
}

void FreeVideoBuffer(DecodedVideoFrame** buff)
{
    if (*buff == NULL)
        return;
    if ((*buff)->video_frame)
    {
        av_frame_unref((*buff)->video_frame);
        av_frame_free(&(*buff)->video_frame);
        (*buff)->video_frame = NULL;
    }
    free(*buff);
    *buff = NULL;
}

//fetch a decoded raw wav buffer from the decoder
DecodedAudioFrame* GetAudioBuffer(DecoderStateStore* dss)
{
    if (dss == NULL || dss->AudioFrames == NULL)
        return NULL;
    DecodedAudioFrame* ret;
    int err = GetBuffer(dss->AudioFrames, (void**)&ret, NULL);
    return ret;
}

//fetch a decoded raw YUV buffer from the decoder
DecodedVideoFrame* GetVideoBuffer(DecoderStateStore* dss)
{
    if (dss == NULL || dss->VideoFrames == NULL)
        return NULL;
    DecodedVideoFrame* ret;
    int err = GetBuffer(dss->VideoFrames, (void**)&ret, NULL);
    return ret;
}
//fetch a decoded raw wav buffer from the decoder
DecodedAudioFrame* PeekAudioBuffer(DecoderStateStore* dss, int index)
{
    if (dss == NULL || dss->AudioFrames == NULL)
        return NULL;
    DecodedAudioFrame* ret;
    int err = PeekBuffer(dss->AudioFrames, (void**)&ret, index, NULL);
    return ret;
}

//fetch a decoded raw YUV buffer from the decoder
DecodedVideoFrame* PeekVideoBuffer(DecoderStateStore* dss, int index)
{
    if (dss == NULL || dss->VideoFrames == NULL)
        return NULL;
    DecodedVideoFrame* ret;
    int err = PeekBuffer(dss->VideoFrames, (void**)&ret, index, NULL);
    return ret;
}

AVFormatContext* GetFormatContext(DecoderStateStore* dss)
{
    if (dss == NULL)
        return NULL;
    return dss->pFormatCtx;
}

AVCodecContext* GetAudioContext(DecoderStateStore* dss)
{
    if (dss == NULL)
        return NULL;
    return dss->audio_ctx;
}

AVCodecContext* GetVideoContext(DecoderStateStore* dss)
{
    if (dss == NULL)
        return NULL;
    return dss->video_ctx;
}

double GetFPS(DecoderStateStore* dss)
{
    if (dss == NULL)
        return 0;

    AVPacket* FirstPacket = NULL;
    PeekBuffer(dss->VideoPackets, (void**)&FirstPacket, 0, NULL);
    if (FirstPacket == NULL)
        return 0;
    //try to get a packet that is far away
    AVPacket* NextPacket = NULL;
    int NextIdex = 1;
    while (NextIdex < 10 && PeekBuffer(dss->VideoPackets, (void**)&NextPacket, NextIdex, NULL) == 0)
        NextIdex++;
    PeekBuffer(dss->VideoPackets, (void**)&NextPacket, NextIdex, NULL);
    if (NextPacket == NULL)
        return 0;
    int64_t Diff = 0;
    if(FirstPacket->pts >= 0 && FirstPacket->pts >= 0)
        Diff = NextPacket->pts - FirstPacket->pts;
    else if (FirstPacket->dts >= 0 && FirstPacket->dts >= 0)
        Diff = NextPacket->dts - FirstPacket->dts;
    double ret = NextIdex / (Diff * av_q2d(dss->video_st->time_base));
    return ret;
}

int _SeekPreciseInputStream(DecoderStateStore* dss, double TimeStamp)
{
    //do not seek while seeking
    while (dss->DecoderIsSeeking)
        mySleep(1);
    dss->DecoderIsSeeking = 1;

    dss->ContentStartedSeek = 1;

    //wait for decoder thread to pause it's execution
    while (dss->DecoderRunningInParallel != 0 && dss->DecoderWaitingSeek == 0)
        mySleep(1);

    if (TimeStamp < 0)
        TimeStamp = 0;

    //break statistic generation
    dss->PrevKeyFrameStamp = -1;

    int retv = 0, reta = 0;
    DecodedVideoFrame* vf2 = PeekVideoBuffer(dss, 0);
    DecodedAudioFrame* af2 = PeekAudioBuffer(dss, 0);
    int SeekFlags = 0;
    if ((vf2 != NULL && TimeStamp < vf2->TimeStamp) || (af2 != NULL && TimeStamp < af2->TimeStamp))
        SeekFlags = AVSEEK_FLAG_BACKWARD;

    int64_t seek_target = (int64_t)TimeStamp;

    if (dss->videoStream >= 0)
    {
        int video_stream_index = dss->videoStream;
        //scale human seconds to file stored time units
        int64_t seek_target_v = av_rescale(seek_target, dss->pFormatCtx->streams[video_stream_index]->time_base.den, dss->pFormatCtx->streams[video_stream_index]->time_base.num);
        //do the actual seeking
        retv = av_seek_frame(dss->pFormatCtx, video_stream_index, seek_target_v, SeekFlags);
    }

    if (dss->audioStream >= 0)
    {
        int audio_stream_index = dss->audioStream;
        int64_t seek_target_a = av_rescale(seek_target, dss->pFormatCtx->streams[audio_stream_index]->time_base.den, dss->pFormatCtx->streams[audio_stream_index]->time_base.num);
        //do the actual seeking
        reta = av_seek_frame(dss->pFormatCtx, audio_stream_index, seek_target_a, SeekFlags);
    }

    //seek failed, nothing we can do
    if (retv < 0 && reta < 0)
    {
        dss->ContentStartedSeek = 0;
        dss->DecoderWaitingSeek = 0;
        return -1;
    }

    //maybe we seeked backwards
    dss->EndOfFileReached = 0;

    //dump old content
    FlushCircularBufferContents(dss);

    //need to clear these flags or else more data will not be fetched
    dss->ContentStartedSeek = 0;
    dss->DecoderWaitingSeek = 0;

    //keep decoding until we find a keyframe + the timestamp we were looking for
    int CanBreak = 1;
    do {
        //this will execute in a blocking way
        FetchMoreDecodedData(dss);

        CanBreak = 1;
        DecodedAudioFrame* af = PeekAudioBuffer(dss, 1);
        if (af != NULL && af->TimeStamp < TimeStamp)
        {
            af = GetAudioBuffer(dss);
            FreeAudioBuffer(&af);
            CanBreak = 0;
        }
        DecodedVideoFrame* vf = PeekVideoBuffer(dss, 1);
        if (vf != NULL && vf->TimeStamp < TimeStamp)
        {
            vf = GetVideoBuffer(dss);
            FreeVideoBuffer(&vf);
            CanBreak = 0;
        }
    } while (CanBreak == 0);
    dss->DecoderIsSeeking = 0;
    //let's double check we indeed seeked correctly. There is about 10% chance we did not jump correctly
    if (TimeStamp > 0)
    {
        int JumpIsFine = 0;
        DecodedAudioFrame* af = PeekAudioBuffer(dss, 0);
        if ((af != NULL && af->TimeStamp <= TimeStamp) || dss->audioStream < 0)
            JumpIsFine++;
        DecodedVideoFrame* vf = PeekVideoBuffer(dss, 0);
        if ((vf != NULL && vf->TimeStamp <= TimeStamp) || dss->videoStream < 0)
            JumpIsFine++;
        //if we failed to find a key frame, try to jump even more back in time and redo the seeking
        if (JumpIsFine != 2)
        {
//            printf("!!!! direct jump failed\n");
            //we need to seek a bit "before" the target time, to be able to find a key frame
            double SafetyDist = 10;
            if (dss->MaxKeyFrameDistance > -1)
                SafetyDist = dss->MaxKeyFrameDistance * av_q2d(dss->video_st->time_base);
            return SeekPreciseInputStream(dss, TimeStamp - SafetyDist);
        }
    }
    //seek went ok
    return 0;
}

int SeekPreciseInputStream(DecoderStateStore* dss, double TimeStamp)
{
    return _SeekPreciseInputStream(dss, TimeStamp);
}
