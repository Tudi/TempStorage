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
            global_video_state->FrameShowTimeFirstPlayer = 0;
            global_video_state->PausePlay = 1 - global_video_state->PausePlay;
            //resume showing the video
            SDL_PauseAudio(global_video_state->PausePlay);
            if (global_video_state->PausePlay == 0)
                video_refresh_timer(global_video_state);
        }break;
        case SDLK_s:
        {
            if (global_video_state->PausePlay == 0)
                printf("Pause the video to save image as picture\n");
            else if(global_video_state->pictq_size != 0)
            {
                VideoPicture* videoPicture;
                // get next VideoPicture to be displayed from the VideoPicture queue
                videoPicture = &global_video_state->pictq[global_video_state->pictq_rindex];
//                SaveAsBMP(videoPicture->frame, global_video_state->video_ctx->width, global_video_state->video_ctx->height, global_video_state->currentFrameIndex);
//                SaveAsJPEG(videoPicture->frame, global_video_state->video_ctx->width, global_video_state->video_ctx->height, global_video_state->currentFrameIndex);
                SaveAsJPEG(videoPicture->frame, global_video_state->CreateInfo->ImageFileName, global_video_state->currentFrameIndex);
            }
        }break;
        case SDLK_a:
        {
            if(strlen(global_video_state->CreateInfo->AudioFileName) == 0)
                strcpy_s(global_video_state->CreateInfo->AudioFileName, sizeof(global_video_state->CreateInfo->AudioFileName), "AudioOut.Wav");

            AVCodecContext* audio_dec_ctx = global_video_state->audio_ctx;            
            int n_channels = audio_dec_ctx->channels;
            long long FrameCount = global_video_state->pFormatCtx->duration;

            //force PCM - 16 bit audio for wav file
            WriteWavHeader(global_video_state->CreateInfo->AudioFileName, 1, n_channels, 16, audio_dec_ctx->sample_rate, (int)FrameCount);
            global_video_state->CreateInfo->DumpAudioToFile = 1;
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
            {
                SDL_PauseAudio(1);
                global_video_state->MuteAudio = 1;
                FlushAudioQueue(global_video_state);
            }
            else
                SDL_PauseAudio(global_video_state->PausePlay);
        }break;
        case SDLK_MINUS:
        case SDLK_KP_MINUS:
        {
            global_video_state->PlayerSpeed *= 1.1; // decrease play speed by 10%
            printf("Player playback speed decreased to %f\n", 1/global_video_state->PlayerSpeed);
            if (global_video_state->PlayerSpeed != 1.0)
            {
                SDL_PauseAudio(1);
                global_video_state->MuteAudio = 1;
                FlushAudioQueue(global_video_state);
            }
            else
                SDL_PauseAudio(global_video_state->PausePlay);
        }break;
        case SDLK_EQUALS:
        {
            global_video_state->PlayerSpeed = 1.0; // reset play speed to normal speed
            printf("Player playback speed reset to 1\n");
            SDL_PauseAudio(global_video_state->PausePlay);
            global_video_state->MuteAudio = 0;
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
                pos = get_master_clock(global_video_state);
                pos += incr;
                stream_seek(global_video_state, (int64_t)(pos * AV_TIME_BASE), (int)incr);
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