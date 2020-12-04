#include "StdAfx.h"


/**
 * Entry point.
 *
 * @param   argc    command line arguments counter.
 * @param   argv    command line arguments.
 *
 * @return          execution exit code.
 */

int main(int argc, char* argv[])
{
    // if the given number of command line arguments is wrong
    PlayerCreateParams* Params = ParseCommandLineParams(argc, argv);
    int ret;
    DecoderStateStore* ds = CreateDecoder(Params->InputFileName);
    if (ds == NULL)
    {
        printf("Could not open input file\n");
        return -1;
    }

    //show stream related info on the console
    if (Params->ShowFrameInfo)
        DumpStreamInfo(ds);

    ret = PlayerCreate(Params, ds);
    if (ret < 0)
    {
        DeleteDecoder(&ds);
        return -1;
    }

    //if we want to dump audio to wav. Create the header for it
    //todo : move this to a function later
    if (Params->DumpAudioToFile)
    {
        AVFormatContext* pFormatCtx = GetFormatContext(ds);
        AVCodecContext* audio_dec_ctx = GetAudioContext(ds);
        if (pFormatCtx && audio_dec_ctx)
        {
            int n_channels = audio_dec_ctx->channels;
            long long FrameCount = pFormatCtx->duration;
            WriteWavHeader(global_video_state->CreateInfo->AudioFileName, 1, n_channels, 16, audio_dec_ctx->sample_rate, (int)FrameCount);
        }
    }

    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

    // infinite loop waiting for fired events
    SDL_Event event;
    for (;;)
    {
        // wait indefinitely for the next available event
        int ret = SDL_WaitEvent(&event);
        if (ret == 0)
        {
            printf("SDL_WaitEvent failed: %s.\n", SDL_GetError());
        }

        // switch on the retrieved event type
        switch (event.type)
        {
            case SDL_KEYDOWN:
            {
                HandleKey(&event);
            }
            break;
            case SDL_MOUSEBUTTONDOWN:
            {
                int mouseX, mouseY;
                int ButtonStates = SDL_GetMouseState(&mouseX, &mouseY);
                int screen_width;
                int screen_height;
                SDL_GetWindowSize(screen, &screen_width, &screen_height);
                AVFormatContext* pFormatCtx = GetFormatContext(global_video_state->ds);
                if (pFormatCtx == NULL)
                    break;
                double MovieDuration = (double)pFormatCtx->duration;
                float SeekPercent = (float)mouseX / (float)screen_width;
                double JumpTo = MovieDuration * mouseX / screen_width / AV_TIME_BASE;
                if (global_video_state->CreateInfo->ShowFrameInfo)
                    printf("Seek to timestamp : %f\n", JumpTo);
                global_video_state->Video_Timestamp = 0;
                global_video_state->Audio_Timestamp = 0;
                SDL_PauseAudio(1);
                FreeAudioBuffer(&global_video_state->AudioFrame);
                SeekPreciseInputStream(global_video_state->ds, JumpTo);
                SDL_PauseAudio(global_video_state->PausePlay);
                global_video_state->TimestampAfterPause = 0;
                global_video_state->FramesRenderedSincePause = 0;
            }break;
            case SDL_SYSWMEVENT:
            {
    /*            const auto& winMessage = event.syswm.msg->msg.win;
                if (winMessage.msg == WM_ENTERSIZEMOVE) 
                {
                    global_video_state->PausePlay = 1;
                    SDL_PauseAudio(global_video_state->PausePlay);
                }
                if (winMessage.msg == WM_EXITSIZEMOVE)
                {
                    global_video_state->PausePlay = 0;
                    SDL_PauseAudio(global_video_state->PausePlay);
                }*/
            }break;

            case FF_QUIT_EVENT:
            case SDL_QUIT:
            {
                if (global_video_state->CreateInfo->ShowFrameInfo == 1)
                    printf("Quit event received. Exiting\n");

                global_video_state->quit = 1;

                SDL_Quit();
            }break;

            case FF_REFRESH_EVENT:
            {
                if(global_video_state == NULL || global_video_state->PausePlay == 0)
                    video_refresh_timer(event.user.data1);
            }break;

            default:
            {
                // nothing to do
            }break;
        }

        // check global quit flag
        if (global_video_state != NULL && global_video_state->quit)
        {
            // exit for loop
            break;
        }
    }

    //allow threads to exit
    SDL_Delay(100);

    //show profiling status
    if( global_video_state && global_video_state->CreateInfo->ShowFrameInfo)
        PrintProfilingResults();

    // clean up memory
    if (global_video_state != NULL)
    {
        av_free(global_video_state);
        global_video_state = NULL;
    }

    //all done. Kill the decoder
    DeleteDecoder(&ds);

    return 0;
}
