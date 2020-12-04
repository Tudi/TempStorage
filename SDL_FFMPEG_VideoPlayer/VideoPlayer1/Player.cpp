#include "Stdafx.h"

VideoState* global_video_state;
AVPacket flush_pkt;

/**
 * This function is used as callback for the SDL_Thread.
 *
 * Opens Audio and Video Streams. If all codecs are retrieved correctly, starts
 * an infinite loop to read AVPackets from the global VideoState AVFormatContext.
 * Based on their stream index, each packet is placed in the appropriate queue.
 *
 * @param   arg the data pointer passed to the SDL_Thread callback function.
 *
 * @return      < 0 in case of error, 0 otherwise.
 */
int decode_thread(void* arg)
{
    // retrieve global VideoState reference
    VideoState* videoState = (VideoState*)arg;

    // set global VideoState reference
    global_video_state = videoState;

    int ret = 0;
    // video and audio stream indexes
    int videoStream = -1;
    int audioStream = -1;
    AVFormatContext* pFormatCtx = NULL;
    // alloc the AVPacket used to read the media file
    AVPacket* packet = av_packet_alloc();
    if (packet == NULL)
    {
        printf("Could not allocate AVPacket.\n");
        goto fail;
    }

    // file I/O context: demuxers read a media file and split it into chunks of data (packets)
    ret = avformat_open_input(&pFormatCtx, videoState->filename, NULL, NULL);
    if (ret < 0)
    {
        printf("Could not open file %s.\n", videoState->filename);
        videoState->quit = 1;
        return -1;
    }

    // reset stream indexes
    videoState->videoStream = -1;
    videoState->audioStream = -1;

    // set the AVFormatContext for the global VideoState reference
    videoState->pFormatCtx = pFormatCtx;

    // read packets of the media file to get stream information
    ret = avformat_find_stream_info(pFormatCtx, NULL);
    if (ret < 0)
    {
        printf("Could not find stream information: %s.\n", videoState->filename);
        return -1;
    }

    // dump information about file onto standard error
    if (_DEBUG_ || videoState->CreateInfo->ShowFrameInfo == 1)
        av_dump_format(pFormatCtx, 0, videoState->filename, 0);

    // loop through the streams that have been found
    for (unsigned int i = 0; i < pFormatCtx->nb_streams; i++)
    {
        // look for the video stream
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && videoStream < 0)
        {
            videoStream = i;
        }

        // look for the audio stream
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && audioStream < 0)
        {
            audioStream = i;
        }
    }

    // return with error in case no video stream was found
    if (videoStream == -1)
    {
        printf("Could not find video stream.\n");
        goto fail;
    }
    else
    {
        // open video stream component codec
        ret = stream_component_open(videoState, videoStream);

        // check video codec was opened correctly
        if (ret < 0)
        {
            printf("Could not open video codec.\n");
            goto fail;
        }
    }

    // return with error in case no audio stream was found
    if (audioStream == -1)
    {
        printf("Could not find audio stream.\n");
        goto fail;
    }
    else
    {
        // open audio stream component codec
        ret = stream_component_open(videoState, audioStream);

        // check audio codec was opened correctly
        if (ret < 0)
        {
            printf("Could not open audio codec.\n");
            goto fail;
        }
    }

    // check both the audio and video codecs were correctly retrieved
    if (videoState->videoStream < 0 || videoState->audioStream < 0)
    {
        printf("Could not open codecs: %s.\n", videoState->filename);
        goto fail;
    }

    if (videoState->CreateInfo->DumpAudioToFile && videoState->audio_ctx)
    {
        AVCodecContext* audio_dec_ctx = videoState->audio_ctx;
        int n_channels = audio_dec_ctx->channels;
        long long FrameCount = videoState->pFormatCtx->duration;
        WriteWavHeader(global_video_state->CreateInfo->AudioFileName, 1, n_channels, 16, audio_dec_ctx->sample_rate, (int)FrameCount);
    }

    // main decode loop: read in a packet and put it on the right queue
    for (;;)
    {
        // check global quit flag
        if (videoState->quit)
        {
            break;
        }

        // seek stuff goes here
        if (videoState->seek_req || videoState->seek_pos)
        {
            if (videoState->CreateInfo->ShowFrameInfo)
            {
                printf("===========================\n");
                printf("Seek in data stream started\n");
                printf("===========================\n");
            }

            int64_t seek_target;
            if (videoState->seek_req != 0) //relative seek
            {
                seek_target = videoState->seek_req;
                //add seconds to the current frame PTS. We presume the jump is relative to current time
                if(videoState->frame_last_pts>0)
                    seek_target += (int64_t)videoState->frame_last_pts;
                else if(videoState->audio_last_frame_pts > 0)
                    seek_target += (int64_t)videoState->audio_last_frame_pts;
            }
            else  // abolute position seek
                seek_target = videoState->seek_pos;

            if (videoState->CreateInfo->ShowFrameInfo)
                printf("Seek : absolute position : %lld seconds\n", seek_target);

            int SeekFlags = 0;
            if (seek_target < videoState->audio_last_frame_pts || seek_target < videoState->frame_last_pts)
                SeekFlags = AVSEEK_FLAG_BACKWARD;

            if (videoState->videoStream >= 0)
            {
                int video_stream_index = videoState->videoStream;
                //scale human seconds to file stored time units
                int64_t seek_target_v = av_rescale(seek_target, pFormatCtx->streams[video_stream_index]->time_base.den, pFormatCtx->streams[video_stream_index]->time_base.num);
                //do the actual seeking
                ret = av_seek_frame(videoState->pFormatCtx, video_stream_index, seek_target_v, SeekFlags);
                //dump remaining buffers from the queue
                if (ret < 0)
                    fprintf(stderr, "%s: error while seeking video to %d\n", videoState->filename, (int)seek_target);
                else if (videoState->videoStream >= 0)
                    // signal our decoder thread to flush it's internal state as soon as possible
                    packet_queue_put(&videoState->videoq, &flush_pkt, 1);
            }

           if (videoState->audioStream >= 0)
            {
               int audio_stream_index = videoState->audioStream;
                int64_t seek_target_a = av_rescale(seek_target, pFormatCtx->streams[audio_stream_index]->time_base.den, pFormatCtx->streams[audio_stream_index]->time_base.num);
                //do the actual seeking
                ret = av_seek_frame(videoState->pFormatCtx, audio_stream_index, seek_target_a, SeekFlags);
                //dump remaining buffers from the queue
                if (ret < 0)
                    fprintf(stderr, "%s: error while seeking audio to %d\n", videoState->filename, (int)seek_target);
                else if (videoState->videoStream >= 0)
                {
                    // signal our decoder thread to flush it's internal state as soon as possible
                    packet_queue_put(&videoState->audioq, &flush_pkt, 1);

                    SDL_CondSignal(videoState->audioq.cond);
                    //this should automatically flush the DECODER->WAVREADER queue
                    videoState->audio_buf_index = 0;
                    videoState->audio_buf_size = 0;
                }
            }

            videoState->seek_pos = 0;
            videoState->seek_req = 0;
            videoState->frame_last_pts = 0; //signal this needs a reinitialization
            videoState->FrameShowTimeFirstPlayer = -1;
            videoState->FirstVideoFramePTSAfterSeek = -1; //delay decoding audio until we get a decodable video frame
        }

        // check audio and video packets queues size
        if (videoState->audioq.size > MAX_AUDIOQ_SIZE || videoState->videoq.size > MAX_VIDEOQ_SIZE)
        {
            // wait for audio and video queues to decrease size
            SDL_Delay(10);

            continue;
        }

        // read data from the AVFormatContext by repeatedly calling av_read_frame()
        ret = av_read_frame(videoState->pFormatCtx, packet);
        if (ret < 0)
        {
            if (ret == AVERROR_EOF)
            {
                if (videoState->videoq.size == 0 && videoState->audioq.size == 0)
                {
                    // media EOF reached, quit
                    if (global_video_state->CreateInfo->ShowFrameInfo == 1)
                        printf("End of file reached. Exiting\n");
                    videoState->quit = 1;
                    break;
                }
                else
                    SDL_Delay(100);
            }
            else if (videoState->pFormatCtx->pb->error == 0)
            {
                // no read error; wait for user input
                SDL_Delay(10);

                continue;
            }
            else
            {
                // exit for loop in case of error
                break;
            }
        }

        // put the packet in the appropriate queue
        if (packet->stream_index == videoState->videoStream)
        {
//            if (videoState->CreateInfo->ShowFrameInfo)
//                printf("DataStream: queue video packet to decoder\n");
            packet_queue_put(&videoState->videoq, packet);
        }
        else if (packet->stream_index == videoState->audioStream)
        {
//            if (videoState->CreateInfo->ShowFrameInfo)
 //               printf("DataStream: queue audio packet to decoder\n");
            if(global_video_state->MuteAudio == 0)
                packet_queue_put(&videoState->audioq, packet);
        }
        else
        {
            // otherwise free the memory
            av_packet_unref(packet);
        }
    }

    // wait for the rest of the program to end
    while (!videoState->quit)
    {
        SDL_Delay(100);
    }

    // close the opened input AVFormatContext
    avformat_close_input(&pFormatCtx);

    // in case of failure, push the FF_QUIT_EVENT and return
fail:
    {
        // create an SDL_Event of type FF_QUIT_EVENT
        SDL_Event event;
        event.type = FF_QUIT_EVENT;
        event.user.data1 = videoState;

        // push the event to the events queue
        SDL_PushEvent(&event);

        // return with error
        return -1;
    };
}

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
int stream_component_open(VideoState* videoState, int stream_index)
{
    // retrieve file I/O context
    AVFormatContext* pFormatCtx = videoState->pFormatCtx;

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

    // in case of Audio codec, set up and open the audio device
    if (codecCtx->codec_type == AVMEDIA_TYPE_AUDIO)
    {
        // desired and obtained audio specs references
        SDL_AudioSpec wanted_specs;
        SDL_AudioSpec specs;

        // Set audio settings from codec info
        wanted_specs.freq = codecCtx->sample_rate;
        wanted_specs.format = AUDIO_S16SYS;
        wanted_specs.channels = codecCtx->channels;
        wanted_specs.silence = 0;
        wanted_specs.samples = SDL_AUDIO_BUFFER_SIZE;
        wanted_specs.callback = audio_callback;
        wanted_specs.userdata = videoState;

        /* Deprecated, please refer to tutorial04-resampled.c for the new API */
        // open audio device
        ret = SDL_OpenAudio(&wanted_specs, &specs);

        // check audio device was correctly opened
        if (ret < 0)
        {
            printf("SDL_OpenAudio: %s.\n", SDL_GetError());
            return -1;
        }
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
        videoState->audioStream = stream_index;
        videoState->audio_st = pFormatCtx->streams[stream_index];
        videoState->audio_ctx = codecCtx;
        videoState->audio_buf_size = 0;
        videoState->audio_buf_index = 0;

        // zero out the block of memory pointed by videoState->audio_pkt
        memset(&videoState->audio_pkt, 0, sizeof(videoState->audio_pkt));

        // init audio packet queue
        packet_queue_init(&videoState->audioq);

        // start playing audio on the first audio device
        SDL_PauseAudio(0);
    }
    break;

    case AVMEDIA_TYPE_VIDEO:
    {
        // set VideoState video related fields
        videoState->videoStream = stream_index;
        videoState->video_st = pFormatCtx->streams[stream_index];
        videoState->video_ctx = codecCtx;

        // Don't forget to initialize the frame timer and the initial
        // previous frame delay: 1ms = 1e-6s
        videoState->frame_timer = (double)av_gettime() / 1000000.0;
        videoState->frame_last_delay = 40e-3;
        videoState->video_current_pts_time = av_gettime();

        // init video packet queue
        packet_queue_init(&videoState->videoq);

        // start video thread
        videoState->video_tid = SDL_CreateThread(video_thread, "Video Thread", videoState);

        // set up the VideoState SWSContext to convert the image data to YUV420
        videoState->sws_ctx = sws_getContext(videoState->video_ctx->width,
            videoState->video_ctx->height,
            videoState->video_ctx->pix_fmt,
            videoState->video_ctx->width,
            videoState->video_ctx->height,
            AV_PIX_FMT_YUV420P,
            SWS_BILINEAR,
            NULL,
            NULL,
            NULL
        );

        // create a window with the specified position, dimensions, and flags.
 /*       screen = SDL_CreateWindow(
            "FFmpeg SDL Video Player",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            codecCtx->width,
            codecCtx->height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI
        );*/
        screen = videoState->gWindow;
        SDL_SetWindowSize(videoState->gWindow, codecCtx->width, codecCtx->height);


        // check window was correctly created
        if (!screen)
        {
            printf("SDL: could not create window - exiting.\n");
            return -1;
        }

        //
        SDL_GL_SetSwapInterval(1);

        // initialize global SDL_Surface mutex reference
        screen_mutex = SDL_CreateMutex();

        // create a 2D rendering context for the SDL_Window
        videoState->renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);

        // create a texture for a rendering context
        videoState->texture = SDL_CreateTexture(
            videoState->renderer,
            SDL_PIXELFORMAT_YV12,
            SDL_TEXTUREACCESS_STREAMING,
            videoState->video_ctx->width,
            videoState->video_ctx->height
        );
    }
    break;

    default:
    {
        // nothing to do
    }
    break;
    }

    return 0;
}

/**
 * Schedules video updates - every time we call this function, it will set the
 * timer, which will trigger an event, which will have our main() function in turn
 * call a function that pulls a frame from our picture queue and displays it.
 *
 * @param   videoState  the global VideoState reference.
 * @param   delay       the delay, expressed in milliseconds, before displaying
 *                      the next video frame on the screen.
 */
void schedule_refresh(VideoState* videoState, Uint32 delay)
{
    // schedule an SDL timer
    int ret = SDL_AddTimer(delay, sdl_refresh_timer_cb, videoState);

    // check the timer was correctly scheduled
    if (ret == 0)
    {
        printf("Could not schedule refresh callback: %s.\n.", SDL_GetError());
    }
}

/**
 * This is the callback function for the SDL Timer.
 *
 * Pushes an SDL_Event of type FF_REFRESH_EVENT to the events queue.
 *
 * @param   interval    the timer delay in milliseconds.
 * @param   param       user defined data passed to the callback function when
 *                      scheduling the timer. In our case the global VideoState
 *                      reference.
 *
 * @return              if the returned value from the callback is 0, the timer
 *                      is canceled.
 */
Uint32 sdl_refresh_timer_cb(Uint32 interval, void* param)
{
    // create an SDL_Event of type FF_REFRESH_EVENT
    SDL_Event event;
    event.type = FF_REFRESH_EVENT;
    event.user.data1 = param;

    // push the event to the events queue
    SDL_PushEvent(&event);

    // return 0 to cancel the timer
    return 0;
}

/**
 *
 * @param videoState
 * @param pos
 * @param rel
 */
void stream_seek(VideoState* videoState, int64_t pos, int rel)
{
    if (!videoState->seek_req)
    {
        videoState->FrameShowTimeFirstPlayer = 0;
        videoState->seek_pos = pos;
        videoState->seek_flags = rel < 0 ? AVSEEK_FLAG_BACKWARD : 0;
        videoState->seek_req = rel;
    }
}

int PlayerCreate(PlayerCreateParams* Params)
{

    /**
     * Initialize SDL.
     * New API: this implementation does not use deprecated SDL functionalities.
     */
    int ret = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
    if (ret != 0)
    {
        printf("Could not initialize SDL - %s\n.", SDL_GetError());
        return -1;
    }

#ifdef _WIN32
    int AudioInitRet = SDL_AudioInit("directsound");
    if (AudioInitRet != 0)
    {
        printf("Could not initialize SDL directsound Audio- %s\n.", SDL_GetError());
        return -1;
    }
#endif
    // the global VideoState reference will be set in decode_thread() to this pointer
    VideoState* videoState = NULL;

    // allocate memory for the VideoState and zero it out
    videoState = (VideoState*)av_mallocz(sizeof(VideoState));

    videoState->CreateInfo = Params;

    videoState->gWindow = SDL_CreateWindow("Video Player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 100, 100, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_SHOWN);

    // copy the file name input by the user to the VideoState structure
    av_strlcpy(videoState->filename, Params->InputFileName, sizeof(videoState->filename));

    // initialize locks for the display buffer (pictq)
    videoState->pictq_mutex = SDL_CreateMutex();
    videoState->pictq_cond = SDL_CreateCond();

    // launch our threads by pushing an SDL_event of type FF_REFRESH_EVENT
    schedule_refresh(videoState, 100);

    videoState->av_sync_type = DEFAULT_AV_SYNC_TYPE;

    // start the decoding thread to read data from the AVFormatContext
    videoState->decode_tid = SDL_CreateThread(decode_thread, "Decoding Thread", videoState);

    // check the decode thread was correctly started
    if (!videoState->decode_tid)
    {
        printf("Could not start decoding SDL_Thread: %s.\n", SDL_GetError());

        // free allocated memory before exiting
        av_free(videoState);

        return -1;
    }

    av_init_packet(&flush_pkt);
    flush_pkt.data = (uint8_t*)"FLUSH";

    videoState->FirstVideoFramePTSAfterSeek = 0;
    videoState->MuteAudio = 0;
    videoState->PlayerSpeed = 1.0;

    return 0;
}