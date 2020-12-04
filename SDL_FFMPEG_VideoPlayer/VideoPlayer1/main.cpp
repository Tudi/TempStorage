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
    ret = PlayerCreate(Params);
    if (ret < 0)
    {
        return -1;
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
            if (global_video_state == NULL || global_video_state->pFormatCtx == NULL)
                break;
            int64_t MovieDuration = global_video_state->pFormatCtx->duration;
            float SeekPercent = (float)mouseX / (float)screen_width;
            int64_t JumpTo = MovieDuration * mouseX / screen_width / AV_TIME_BASE;
            stream_seek(global_video_state, JumpTo, 0);
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

            /**
             * If the video has finished playing, then both the picture and audio
             * queues are waiting for more data.  Make them stop waiting and
             * terminate normally.
             */
            SDL_CondSignal(global_video_state->audioq.cond);
            SDL_CondSignal(global_video_state->videoq.cond);

            SDL_Quit();
        }
        break;

        case FF_REFRESH_EVENT:
        {
            if(global_video_state == NULL || global_video_state->PausePlay == 0)
                video_refresh_timer(event.user.data1);
        }
        break;

        default:
        {
            // nothing to do
        }
        break;
        }

        // check global quit flag
        if (global_video_state != NULL && global_video_state->quit)
        {
            // exit for loop
            break;
        }
    }

    //show profiling status
    if( global_video_state && global_video_state->CreateInfo->ShowFrameInfo)
        PrintProfilingResults();

    // clean up memory
    if (global_video_state != NULL)
    {
        av_free(global_video_state);
        global_video_state = NULL;
    }

    return 0;
}
