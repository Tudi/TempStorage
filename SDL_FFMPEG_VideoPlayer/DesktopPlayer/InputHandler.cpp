#include "StdAfx.h"

void HandleKey(SDL_Event* event)
{
    double incr, pos;

    if (global_video_state == NULL)
        return;

    switch (event->key.keysym.sym)
    {
        case SDLK_SPACE:
        {
            //pause / resume video play
            global_video_state->PausePlay = 1 - global_video_state->PausePlay;
            //resume showing the video
            SDL_PauseAudio(global_video_state->PausePlay);
            if (global_video_state->PausePlay == 0)
            {
                video_refresh_timer(global_video_state);
                global_video_state->TimestampAfterPause = 0;
                global_video_state->FramesRenderedSincePause = 0;
            }

        }break;
        case SDLK_s:
        {
            if (global_video_state->PausePlay == 0)
            {
                printf("Pause the video to save image as picture\n");
                break;
            }
            DecodedVideoFrame* vf2 = PeekVideoBuffer(global_video_state->ds, 0);
            if(vf2 != NULL)
            {
                printf("Image saved\n");
                SaveAsJPEG(vf2->video_frame, global_video_state->CreateInfo->ImageFileName, global_video_state->currentFrameIndex);
            }
        }break;
        case SDLK_a:
        {
            AVFormatContext* pFormatCtx = GetFormatContext(global_video_state->ds);
            AVCodecContext* audio_dec_ctx = GetAudioContext(global_video_state->ds);
            if (pFormatCtx && audio_dec_ctx)
            {
                if (strlen(global_video_state->CreateInfo->AudioFileName) == 0)
                    strcpy_s(global_video_state->CreateInfo->AudioFileName, sizeof(global_video_state->CreateInfo->AudioFileName), "AudioOut.Wav");
                int n_channels = audio_dec_ctx->channels;
                long long FrameCount = pFormatCtx->duration;
                WriteWavHeader(global_video_state->CreateInfo->AudioFileName, 1, n_channels, 16, audio_dec_ctx->sample_rate, (int)FrameCount);
                global_video_state->CreateInfo->DumpAudioToFile = 1;
            }
        }break;
        case SDLK_i:
        {
            global_video_state->CreateInfo->ShowFrameInfo = 1 - global_video_state->CreateInfo->ShowFrameInfo;
        }break;
        case SDLK_PLUS:
        case SDLK_KP_PLUS:
        {
            global_video_state->PlayerSpeed *= 0.9; // increase play speed by 10%
            printf("Player playback speed increased to %f\n", 1/global_video_state->PlayerSpeed);
            if(global_video_state->PlayerSpeed != 1.0)
                global_video_state->MuteAudio = 1;
        }break;
        case SDLK_MINUS:
        case SDLK_KP_MINUS:
        {
            global_video_state->PlayerSpeed *= 1.1; // decrease play speed by 10%
            printf("Player playback speed decreased to %f\n", 1/global_video_state->PlayerSpeed);
            if (global_video_state->PlayerSpeed != 1.0)
                global_video_state->MuteAudio = 1;
        }break;
        case SDLK_EQUALS:
        {
            global_video_state->PlayerSpeed = 1.0; // reset play speed to normal speed
            printf("Player playback speed reset to 1\n");
            global_video_state->MuteAudio = 0;
        }break;
        case SDLK_m:
        {
            global_video_state->MuteAudio = 1 - global_video_state->MuteAudio;
        }break;
        case SDLK_LEFT:
        {
            incr = -10.0;
            goto do_seek;
        }
        case SDLK_RIGHT:
        {
            incr = 10.0;
            goto do_seek;
        }
        case SDLK_DOWN:
        {
            incr = -60.0;
            goto do_seek;
        }
        case SDLK_UP:
        {
            incr = 60.0;
            goto do_seek;
        }

 do_seek:
        {
            if (global_video_state)
            {
                pos = global_video_state->Video_Timestamp + incr;
                global_video_state->Video_Timestamp = 0;
                global_video_state->Audio_Timestamp = 0;
                if (global_video_state->CreateInfo->ShowFrameInfo)
                    printf("Seek to timestamp : %f\n", pos);
                SDL_PauseAudio(1);
                FreeAudioBuffer(&global_video_state->AudioFrame);
                SeekPreciseInputStream(global_video_state->ds, pos);
                SDL_PauseAudio(global_video_state->PausePlay);
                global_video_state->TimestampAfterPause = 0;
                global_video_state->FramesRenderedSincePause = 0;
            }
            break;
        };

        default:
        {
            // nothing to do
        }
        break;
    }

}