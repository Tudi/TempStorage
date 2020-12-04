#include "Stdafx.h"

VideoState* global_video_state;
SDL_Window* screen;
SDL_mutex* screen_mutex;

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
#define RUN_DECODING_IN_BACKGROUND_THREAD
int decode_thread(void* arg)
{
    // retrieve global VideoState reference
    VideoState* videoState = (VideoState*)arg;

    // fill the decoder buffers
    GenerateRawOutput(videoState->ds);
    // wait for the rest of the program to end
    int TriedFetchMoreData = 0;
    while (!videoState->quit)
    {
        DecodedAudioFrame* af2 = PeekAudioBuffer(videoState->ds, 2);
        DecodedVideoFrame* vf2 = PeekVideoBuffer(videoState->ds, 2);
        if (af2 == NULL && vf2 == NULL)
        {
            if (TriedFetchMoreData < 5)
            {
	            TriggerProfilePoint(PP_STREAM_DECODE);
                GenerateRawOutput(videoState->ds);
	            TriggerProfilePoint(PP_STREAM_DECODE, END_PROFILING);
                TriedFetchMoreData++;
            }
            else
            {
                videoState->quit = 1;
                break;
            }
        }
        else if (af2 == NULL || vf2 == NULL)
        {
            TriggerProfilePoint(PP_STREAM_DECODE);
            GenerateRawOutput(videoState->ds);
            TriggerProfilePoint(PP_STREAM_DECODE, END_PROFILING);
            TriedFetchMoreData = 0;
        }
//        else
//            SDL_Delay(1);
    }

    // exiting with good result
    return 0;
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
int stream_component_open(VideoState* videoState, AVCodecContext* codecCtx, int stream_index)
{
    // retrieve file I/O context
    AVFormatContext* pFormatCtx = GetFormatContext(videoState->ds);
    int ret;

    if (codecCtx == NULL || pFormatCtx == NULL)
        return -1;

    // set up the global VideoState based on the type of the codec obtained for
    // the given stream index
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

        // start playing audio on the first audio device
        SDL_PauseAudio(0);
    }
    else if(codecCtx->codec_type == AVMEDIA_TYPE_VIDEO)
    {
        AVCodecContext * video_ctx = GetVideoContext(videoState->ds);
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
        videoState->texture = SDL_CreateTexture( videoState->renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, video_ctx->width, video_ctx->height );
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

int PlayerCreate(PlayerCreateParams* Params, DecoderStateStore* ds)
{
    // the global VideoState reference will be set in decode_thread() to this pointer
    VideoState* videoState = NULL;

    // allocate memory for the VideoState and zero it out
    videoState = (VideoState*)av_mallocz(sizeof(VideoState));

    // set global VideoState reference
    global_video_state = videoState;

    videoState->CreateInfo = Params;
    videoState->ds = ds;

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

    videoState->gWindow = SDL_CreateWindow("Video Player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 100, 100, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_SHOWN);

    stream_component_open(videoState, GetVideoContext(videoState->ds), 0);
    stream_component_open(videoState, GetAudioContext(videoState->ds), 1);

    // launch our threads by pushing an SDL_event of type FF_REFRESH_EVENT
    schedule_refresh(videoState, 100);

    // start the decoding thread to read data from the AVFormatContext
#ifdef RUN_DECODING_IN_BACKGROUND_THREAD
    videoState->decode_tid = SDL_CreateThread(decode_thread, "Decoding Thread", videoState);

    // check the decode thread was correctly started
    if (!videoState->decode_tid)
    {
        printf("Could not start decoding SDL_Thread: %s.\n", SDL_GetError());

        // free allocated memory before exiting
        av_free(videoState);

        return -1;
    }
#endif
    videoState->MuteAudio = 0;
    videoState->PlayerSpeed = 1.0;
    videoState->AudioFrame = NULL;
    videoState->TimestampAfterPause = 0;
    videoState->FramesRenderedSincePause = 0;
    // fill the decoder buffers
    GenerateRawOutput(videoState->ds);
    //try to have a vague idea what the movie FPS is
    videoState->AvgFPS = GetFPS(videoState->ds);
    if (videoState->AvgFPS <= 0)
        videoState->AvgFPS = 25;
    return 0;
}

/**
 * Pulls from the VideoPicture queue when we have something, sets our timer for
 * when the next video frame should be shown, calls the video_display() method to
 * actually show the video on the screen, then decrements the counter on the queue,
 * and decreases its size.
 *
 * @param   userdata    SDL_UserEvent->data1;   User defined data pointer.
 */
void video_refresh_timer(void* userdata)
{
    // retrieve global VideoState reference
    VideoState* videoState = (VideoState*)userdata;

    //seems like there is no decoder anymore. No point keeping this refresh alive ?
    if (videoState->ds == NULL)
    {
        schedule_refresh(videoState, 100);
        return;
    }

    // check the video stream was correctly opened
#ifndef RUN_DECODING_IN_BACKGROUND_THREAD
//    GenerateRawOutput(videoState->ds);
#endif
    DecodedVideoFrame* vf = GetVideoBuffer(videoState->ds);
    // check the VideoPicture queue contains decoded frames
    if (vf == NULL)
    {
        if (videoState->CreateInfo->ShowFrameInfo)
        {
            static int c = 0;
            printf("Could not obtain video data to render - %d\n", c++);
        }
        schedule_refresh(videoState, 1);
        return;
    }

    //eeeeuuugh. Missing frame timestamp. Crossing fingers it's good as it gets
    if (vf->TimeStamp < 0)
        vf->TimeStamp = videoState->Video_Timestamp + 1.0 / videoState->AvgFPS;

//    assert(videoState->Video_Timestamp <= vf->TimeStamp);
    int SkipSyncronize = 0;
    if (videoState->Video_Timestamp > vf->TimeStamp)
        SkipSyncronize = 1;

    videoState->Video_Timestamp = vf->TimeStamp;
    videoState->currentFrameIndex = vf->frame_number;

//    printf("Video render : %f\n", (float)videoState->Video_Timestamp);
    int RenderNthFrame = (int)(1.0 / global_video_state->PlayerSpeed);
    if (RenderNthFrame > 1 && (vf->frame_number % RenderNthFrame) != 0)
    {
        // try to render a new frame as soon as possible
        schedule_refresh(videoState, 1);
        return;
    }

    // get VideoPicture reference using the queue read index
//            Uint32 real_delay = CalculateImageShowTime(videoState, vf->TimeStamp);
    Uint32 real_delay = (Uint32)(1000 / videoState->AvgFPS);
    //todo : remove this once proper syncronization is made
    if (SkipSyncronize == 0)
    {
        //if possible, we trust our harware audio rendering to provide a good clock
        if (videoState->Audio_Timestamp != 0)
        {
            if (videoState->Video_Timestamp > videoState->Audio_Timestamp)
            {
                double SleepMS = 1000 * (videoState->Video_Timestamp - videoState->Audio_Timestamp);
                if (SleepMS > real_delay * 2)
                    SleepMS = real_delay * 2;
                real_delay = (Uint32)SleepMS;
            }
            else if (videoState->Video_Timestamp < videoState->Audio_Timestamp)
            {
                double SleepMS = real_delay / (1000 * (videoState->Audio_Timestamp - videoState->Video_Timestamp));
                real_delay = (Uint32)SleepMS;
            }
        }
        //if audio stream is not available, we will use PC clock to syncronize packet timestamps
        else if(videoState->TimestampAfterPause > 0)
        {
            int64_t PCTimePassedSincePause = av_gettime() - videoState->TimestampAfterPause;
            int64_t AvgFrameSwapTime = PCTimePassedSincePause / videoState->FramesRenderedSincePause / 1000;
            if (AvgFrameSwapTime > real_delay)
            {
                double SpeedupCoeff = (double)real_delay / (double)AvgFrameSwapTime;
                real_delay = (Uint32)(real_delay * SpeedupCoeff);
            }
        }
    }
    if (videoState->TimestampAfterPause == 0)
        videoState->TimestampAfterPause = av_gettime();
    videoState->FramesRenderedSincePause++;

    real_delay = (Uint32)(real_delay * videoState->PlayerSpeed);

    schedule_refresh(videoState, (Uint32)(real_delay));

    // show the frame on the SDL_Surface (the screen)
    video_display(videoState, vf);
    FreeVideoBuffer(&vf);
}

/**
 * Pull in data from audio_decode_frame(), store the result in an intermediary
 * buffer, attempt to write as many bytes as the amount defined by len to
 * stream, and get more data if we don't have enough yet, or save it for later
 * if we have some left over.
 *
 * @param   userdata    the pointer we gave to SDL.
 * @param   stream      the buffer we will be writing audio data to.
 * @param   len         the size of that buffer.
 */
void audio_callback(void* userdata, Uint8* stream, int len)
{
    // retrieve the VideoState
    VideoState* videoState = (VideoState*)userdata;

    // while the length of the audio data buffer is > 0
    while (len > 0)
    {
        // check global quit flag
        if (global_video_state == NULL || global_video_state->quit)
        {
            return;
        }
#ifndef RUN_DECODING_IN_BACKGROUND_THREAD
        //make sure we have something to fetch
        GenerateRawOutput(videoState->ds);
#endif
        //fetch a decoded audio buffer
        if (videoState->AudioFrame == NULL)
        {
            videoState->AudioFrame = GetAudioBuffer(videoState->ds);
            videoState->AudioBytesConsumed = 0;

            if (videoState->CreateInfo->DumpAudioToFile && videoState->AudioFrame != NULL)
                WriteWavBuffer(videoState->CreateInfo->AudioFileName, videoState->AudioFrame->audio_data, videoState->AudioFrame->audio_size);
        }
        //if input stream was not ready to provide an audio buffer than wait for more input data
        if (videoState->AudioFrame == NULL)
        {
            memset(stream, 0, len);
            if (videoState->CreateInfo->ShowFrameInfo)
            {
                static int c = 0;
                printf("Could not obtain audio data to render - %d\n", c++);
            }
            break;
        }
//        assert(videoState->Audio_Timestamp <= videoState->AudioFrame->TimeStamp);
        if(videoState->AudioFrame->TimeStamp > 0) // on error, this might go berserk
            videoState->Audio_Timestamp = videoState->AudioFrame->TimeStamp;
        if (global_video_state->MuteAudio)
        {
            memset(stream, 0, len);
            if (videoState->AudioFrame->TimeStamp < videoState->Video_Timestamp)
            {
                FreeAudioBuffer(&videoState->AudioFrame);
                continue;
            }
            else
                break;
        }
        //dump audio render related info to console
        if (videoState->CreateInfo->ShowFrameInfo)
        {
            static int FrameCounter = 0;
            printf("Audio render : Frame=%d TS=%f Bytes=%d\n", FrameCounter++, (float)videoState->Audio_Timestamp, videoState->AudioFrame->audio_size);
        }

        int len1 = videoState->AudioFrame->audio_size - videoState->AudioBytesConsumed;
        //if we have decoded data to be sent to the "audio card buffer". Else audio will make pop sound ?
        if (len1 > 0)
        {
            if (len1 > len)
                len1 = len;

            // copy data from audio buffer to the SDL stream
            memcpy(stream, &videoState->AudioFrame->audio_data[videoState->AudioBytesConsumed], len1);

            len -= len1;
            stream += len1;

            // update global VideoState audio buffer index
            videoState->AudioBytesConsumed += len1;
        }
        if (videoState->AudioBytesConsumed >= videoState->AudioFrame->audio_size)
        {
            FreeAudioBuffer(&videoState->AudioFrame);
            videoState->AudioBytesConsumed = 0;
        }
    }
}

/**
 * Retrieves the video aspect ratio first, which is just the width divided by the
 * height. Then it scales the movie to fit as big as possible in our screen
 * (SDL_Surface). Then it centers the movie, and calls SDL_DisplayYUVOverlay()
 * to update the surface, making sure we use the screen mutex to access it.
 *
 * @param   videoState  the global VideoState reference.
 */
void video_display(VideoState* videoState, DecodedVideoFrame* vf)
{
    int status;

    status = SDL_TryLockMutex(screen_mutex);
    if (status == SDL_MUTEX_TIMEDOUT)
        return;

    if (videoState->CreateInfo->DumpImagesToFile != 0)
        SaveAsJPEG(vf->video_frame, global_video_state->CreateInfo->ImageFileName, global_video_state->currentFrameIndex);

    // reference for the next VideoPicture to be displayed
    double aspect_ratio;

    int w, h, x, y;

    aspect_ratio = (float)vf->width / (float)vf->height;

    // get the size of a window's client area
    int screen_width;
    int screen_height;
    SDL_GetWindowSize(screen, &screen_width, &screen_height);

    // global SDL_Surface height
    h = screen_height;

    // retrieve width using the calculated aspect ratio and the screen height
    w = ((int)rint(h * aspect_ratio)) & -3;

    // if the new width is bigger than the screen width
    if (w > screen_width)
    {
        // set the width to the screen width
        w = screen_width;

        // recalculate height using the calculated aspect ratio and the screen width
        h = ((int)rint(w / aspect_ratio)) & -3;
    }

    // TODO: Add full screen support
    x = (screen_width - w);
    y = (screen_height - h);

    // check the number of frames to decode was not exceeded
    if (++videoState->currentFrameIndex < videoState->CreateInfo->MaxFramesToDecode)
    {

        if (videoState->CreateInfo->ShowFrameInfo)
        {
            static int FrameCounter = 0;
            printf("Video render : Frame=%d TS=%f FrameType=%c KeyFrame=%d RenderTime=%lld\n", FrameCounter++, (float)vf->TimeStamp, av_get_picture_type_char((AVPictureType) vf->pic_type), vf->key_frame, GetProfileTimeUS(PP_VIDEO_FRAME_RENDER));
        }

        // set blit area x and y coordinates, width and height
        SDL_Rect rect;
        rect.x = x;
        rect.y = y;
        rect.w = w;
        rect.h = h;

        Uint8* Y = vf->video_frame->data[0];
        Uint8* U = vf->video_frame->data[1];
        Uint8* V = vf->video_frame->data[2];
        TriggerProfilePoint(PP_RENDER_TEXTURE_UPDATE);
        // update the texture with the new pixel data
        SDL_UpdateYUVTexture( videoState->texture, &rect, Y, vf->video_frame->linesize[0], U, vf->video_frame->linesize[1], V, vf->video_frame->linesize[2]);

        // clear the current rendering target with the drawing color
        SDL_RenderClear(videoState->renderer);

        // copy a portion of the texture to the current rendering target
        SDL_RenderCopy(videoState->renderer, videoState->texture, NULL, NULL);
        TriggerProfilePoint(PP_RENDER_TEXTURE_UPDATE, END_PROFILING);

        // update the screen with any rendering performed since the previous call
        TriggerProfilePoint(PP_VIDEO_FRAME_RENDER);
        SDL_RenderPresent(videoState->renderer);
        TriggerProfilePoint(PP_VIDEO_FRAME_RENDER, END_PROFILING);
    }
    else
    {
        // create an SDLEvent of type FF_QUIT_EVENT
        SDL_Event event;
        event.type = FF_QUIT_EVENT;
        event.user.data1 = videoState;

        // push the event
        SDL_PushEvent(&event);
    }

    // unlock screen mutex
    SDL_UnlockMutex(screen_mutex);
}
