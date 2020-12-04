#include "StdAfx.h"

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

    double pts;

    // while the length of the audio data buffer is > 0
    while (len > 0)
    {
        // check global quit flag
        if (global_video_state->quit)
        {
            return;
        }

        // check how much audio is left to writes
        if (videoState->audio_buf_index >= videoState->audio_buf_size)
        {
            // we have already sent all avaialble data; get more
            int audio_size = audio_decode_frame(videoState, videoState->audio_buf, sizeof(videoState->audio_buf), &pts);

            // if error
            if (audio_size < 0)
            {
                // output silence
                videoState->audio_buf_size = 1024;
                // clear memory
                memset(videoState->audio_buf, 0, videoState->audio_buf_size);
                printf("audio_decode_frame() failed.\n");
                //no valid data in the buffer
//                videoState->audio_buf_size = 0;
            }
            else
            {
                //                audio_size = synchronize_audio(videoState, (int16_t*)videoState->audio_buf, audio_size);

                // cast to usigned just to get rid of annoying warning messages
                videoState->audio_buf_size = (unsigned)audio_size;
            }

            videoState->audio_buf_index = 0;
        }

        int len1 = videoState->audio_buf_size - videoState->audio_buf_index;
        //if we have decoded data to be sent to the "audio card buffer". Else audio will make pop sound ?
        if (len1 > 0)
        {
            if (len1 > len)
                len1 = len;

            // copy data from audio buffer to the SDL stream
            memcpy(stream, (uint8_t*)videoState->audio_buf + videoState->audio_buf_index, len1);

            len -= len1;
            stream += len1;

            // update global VideoState audio buffer index
            videoState->audio_buf_index += len1;
        }
    }
}

