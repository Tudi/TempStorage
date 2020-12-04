#include "StdAfx.h"

void FlushAudioQueue(VideoState* videoState)
{
    avcodec_flush_buffers(videoState->audio_ctx);
    packet_queue_flush(&videoState->audioq);
    videoState->audio_buf_index = 0;
    videoState->audio_buf_size = 0;
}
/**
 * Get a packet from the queue if available. Decode the extracted packet. Once
 * we have the frame, resample it and simply copy it to our audio buffer, making
 * sure the data_size is smaller than our audio buffer.
 *
 * @param   aCodecCtx   the audio AVCodecContext used for decoding
 * @param   audio_buf   the audio buffer to write into
 * @param   buf_size    the size of the audio buffer, 1.5 larger than the one
 *                      provided by FFmpeg
 * @param   pts_ptr     a pointer to the pts of the decoded audio frame.
 *
 * @return              0 if everything goes well, -1 in case of error or quit
 */
int audio_decode_frame(VideoState* videoState, uint8_t* audio_buf, int buf_size, double* pts_ptr)
{
    // allocate AVPacket to read from the audio PacketQueue (audioq)
    AVPacket* avPacket = av_packet_alloc();
    if (avPacket == NULL)
    {
        printf("Could not allocate AVPacket.\n");
        return -1;
    }

    double pts;
    int n;

    // allocate a new frame, used to decode audio packets
    static AVFrame* avFrame = NULL;
    avFrame = av_frame_alloc();
    if (!avFrame)
    {
        printf("Could not allocate AVFrame.\n");
        return -1;
    }

    // infinite loop: read AVPackets from the audio PacketQueue, decode them into
    // audio frames, resample the obtained frame and update the audio buffer
    for (;;)
    {
        // check global quit flag
        if (videoState->quit)
            return -1;

        // get more audio AVPacket
        int ret = packet_queue_get(&videoState->audioq, avPacket, 1);

        // if packet_queue_get returns < 0, the global quit flag was set
        // audio queue is empty, wait for some packet to be read from the file
        if (ret < 0)
            return -1;

        if (avPacket->data == flush_pkt.data)
        {
            avcodec_flush_buffers(videoState->audio_ctx);
            packet_queue_flush(&videoState->audioq);
            //we performed a seek, flushed queue and codec buffers. Need to read new packets from file
            return -1;
        }

        //happens after seek. Make audio silent until we get a video frame that we can decode
        if (videoState->FirstVideoFramePTSAfterSeek < 0)
            return -1;

        //try to catch up audio with video. 
        double PacketPTS = av_q2d(videoState->audio_st->time_base) * avPacket->pts;
        if (PacketPTS + 1 < videoState->FirstVideoFramePTSAfterSeek 
//            || PacketPTS + 3 < videoState->frame_last_pts
            )
        {
            videoState->FirstVideoFramePTSAfterSeek = videoState->frame_last_pts;
            printf("Audio packet PTS is %f, too far behind %f / %f to decode, skip it\n", (float)PacketPTS, (float)videoState->FirstVideoFramePTSAfterSeek, (float)videoState->frame_last_pts);
            continue;
        }
        else
            // Catch up should only happen once after seek
            videoState->FirstVideoFramePTSAfterSeek = 0;

        // keep audio_clock up-to-date
        if (avPacket->pts != AV_NOPTS_VALUE)
            videoState->audio_clock = av_q2d(videoState->audio_st->time_base) * avPacket->pts;

        TriggerProfilePoint(PP_AUDIO_FRAME_DECODE);

        // give the decoder raw compressed data in an AVPacket
        ret = avcodec_send_packet(videoState->audio_ctx, avPacket);

        // check if we obtained an AVPacket from the audio PacketQueue
        int data_size = 0;
        while (ret >= 0)
        {
            // get decoded output data from decoder
            int ret = avcodec_receive_frame(videoState->audio_ctx, avFrame);

            // check the decoder needs more AVPackets to be sent
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                break;
            else if (ret < 0)
            {
                printf("avcodec_receive_frame decoding error.\n");
                return -1;
            }

            TriggerProfilePoint(PP_AUDIO_FRAME_DECODE, END_PROFILING);
            // if we decoded an entire audio frame
            // apply audio resampling to the decoded frame
            TriggerProfilePoint(PP_AUDIO_FRAME_RESAMPLE);
            data_size = audio_resampling(videoState, avFrame, AV_SAMPLE_FMT_S16, audio_buf);
            TriggerProfilePoint(PP_AUDIO_FRAME_RESAMPLE, END_PROFILING);
            assert(data_size <= buf_size);

            // no data yet, get more frames
            if (data_size <= 0)
                continue;

            // keep audio_clock up-to-date
            pts = videoState->audio_clock;
            *pts_ptr = pts;
            n = 2 * videoState->audio_ctx->channels;
            videoState->audio_clock += (double)data_size / (double)(n * videoState->audio_ctx->sample_rate);
            videoState->audio_last_frame_pts = pts;
            if (videoState->CreateInfo->ShowFrameInfo)
            {
                double packet_timestamp = (double)avPacket->pts * av_q2d(videoState->audio_st->time_base);
                double frame_timestamp = (double)avFrame->pts * av_q2d(videoState->audio_st->time_base);
                double duration = (double)avFrame->nb_samples / (double)videoState->audio_ctx->sample_rate;
                printf("Decoder : audio frame pts=%lld, packet Timestamp=%f Bytes=%d Duration=%f DecodeDuration=%d rawsize=%d\n", avPacket->pts, (float)packet_timestamp, avPacket->size, (float)duration,(int)GetProfileTimeUS(PP_AUDIO_FRAME_DECODE), data_size);
            }

            // wipe the packet
            if (avPacket->data)
                av_packet_unref(avPacket);

            //if output format is wav, dump it into the wav file
            if (videoState->CreateInfo->DumpAudioToFile)
                WriteWavBuffer(videoState->CreateInfo->AudioFileName, videoState->audio_buf, data_size);

            // we have the data, return it and come back for more later
            return data_size;
        }

        //we only get here if we failed to decode the packet
        if (avPacket->data)
            av_packet_unref(avPacket);
    }

    return 0;
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
static int audio_resampling(VideoState* videoState, AVFrame* decoded_audio_frame, enum AVSampleFormat out_sample_fmt, uint8_t* out_buf)
{
    // get an instance of the AudioResamplingState struct
    AudioResamplingState* arState = getAudioResampling(videoState->audio_ctx->channel_layout);

    if (!arState->swr_ctx)
    {
        printf("swr_alloc error.\n");
        return -1;
    }

    // get input audio channels
    arState->in_channel_layout = (videoState->audio_ctx->channels ==
        av_get_channel_layout_nb_channels(videoState->audio_ctx->channel_layout)) ?
        videoState->audio_ctx->channel_layout :
        av_get_default_channel_layout(videoState->audio_ctx->channels);

    // check input audio channels correctly retrieved
    if (arState->in_channel_layout <= 0)
    {
        printf("in_channel_layout error.\n");
        return -1;
    }

    // set output audio channels based on the input audio channels
    if (videoState->audio_ctx->channels == 1)
        arState->out_channel_layout = AV_CH_LAYOUT_MONO;
    else if (videoState->audio_ctx->channels == 2)
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
    av_opt_set_int(arState->swr_ctx, "in_sample_rate", videoState->audio_ctx->sample_rate, 0);

    // Set SwrContext parameters for resampling
    av_opt_set_sample_fmt(arState->swr_ctx, "in_sample_fmt", videoState->audio_ctx->sample_fmt, 0);

    // Set SwrContext parameters for resampling
    av_opt_set_int(arState->swr_ctx, "out_channel_layout", arState->out_channel_layout, 0);

    // Set SwrContext parameters for resampling
    av_opt_set_int(arState->swr_ctx, "out_sample_rate", videoState->audio_ctx->sample_rate, 0);

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
        videoState->audio_ctx->sample_rate,
        videoState->audio_ctx->sample_rate,
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
        swr_get_delay(arState->swr_ctx, videoState->audio_ctx->sample_rate) + arState->in_nb_samples,
        videoState->audio_ctx->sample_rate,
        videoState->audio_ctx->sample_rate,
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