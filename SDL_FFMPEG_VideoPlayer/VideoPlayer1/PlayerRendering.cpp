#include "StdAfx.h"

SDL_Window* screen;

/**
 * Global SDL_Surface mutex reference.
 */
SDL_mutex* screen_mutex;


/**
 * Waits for space in the VideoPicture queue. Allocates a new SDL_Overlay in case
 * it is not already allocated or has a different width/height. Converts the given
 * decoded AVFrame to an AVPicture using specs supported by SDL and writes it in the
 * VideoPicture queue.
 *
 * @param   videoState  the global VideoState reference.
 * @param   pFrame      AVFrame to be inserted in the VideoState->pictq (as an AVPicture).
 *
 * @return              < 0 in case the global quit flag is set, 0 otherwise.
 */
int queue_picture(VideoState* videoState, AVFrame* pFrame, double pts)
{
    // lock VideoState->pictq mutex
    SDL_LockMutex(videoState->pictq_mutex);

    // wait until we have space for a new pic in VideoState->pictq
    while (videoState->pictq_size >= VIDEO_PICTURE_QUEUE_SIZE && !videoState->quit)
    {
        SDL_CondWait(videoState->pictq_cond, videoState->pictq_mutex);
    }

    // unlock VideoState->pictq mutex
    SDL_UnlockMutex(videoState->pictq_mutex);

    // check global quit flag
    if (videoState->quit)
    {
        return -1;
    }

    // retrieve video picture using the queue write index
    VideoPicture* videoPicture;
    videoPicture = &videoState->pictq[videoState->pictq_windex];

    // if the VideoPicture SDL_Overlay is not allocated or has a different width/height
    if (!videoPicture->frame ||
        videoPicture->width != videoState->video_ctx->width ||
        videoPicture->height != videoState->video_ctx->height)
    {
        // set SDL_Overlay not allocated
        videoPicture->allocated = 0;

        // allocate a new SDL_Overlay for the VideoPicture struct
        alloc_picture(videoState);

        // check global quit flag
        if (videoState->quit)
        {
            return -1;
        }
    }

    //check if we should save original image before rescale 
    if (videoState->CreateInfo->DumpImagesToFile != 0)
        SaveAsJPEG(pFrame, videoState->CreateInfo->ImageFileName, videoState->currentFrameIndex);

    // check the new SDL_Overlay was correctly allocated
    if (videoPicture->frame)
    {
        // set pts value for the last decode frame in the VideoPicture queu (pctq)
        videoPicture->pts = pts;

        // set VideoPicture AVFrame info using the last decoded frame
        videoPicture->frame->pict_type = pFrame->pict_type;
        videoPicture->frame->pts = pFrame->pts;
        videoPicture->frame->pkt_dts = pFrame->pkt_dts;
        videoPicture->frame->key_frame = pFrame->key_frame;
        videoPicture->frame->coded_picture_number = pFrame->coded_picture_number;
        videoPicture->frame->display_picture_number = pFrame->display_picture_number;
        videoPicture->frame->width = pFrame->width;
        videoPicture->frame->height = pFrame->height;

        TriggerProfilePoint(PP_VIDEO_FRAME_RESIZE);

        // scale the image in pFrame->data and put the resulting scaled image in pict->data
        sws_scale(
            videoState->sws_ctx,
            (uint8_t const* const*)pFrame->data,
            pFrame->linesize,
            0,
            videoState->video_ctx->height,
            videoPicture->frame->data,
            videoPicture->frame->linesize
        );

        TriggerProfilePoint(PP_VIDEO_FRAME_RESIZE, END_PROFILING);

        // update VideoPicture queue write index
        videoState->pictq_windex = (videoState->pictq_windex + 1 ) % VIDEO_PICTURE_QUEUE_SIZE;

        // lock VideoPicture queue
        SDL_LockMutex(videoState->pictq_mutex);

        // increase VideoPicture queue size
        videoState->pictq_size++;

        // unlock VideoPicture queue
        SDL_UnlockMutex(videoState->pictq_mutex);
    }

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
    if (_DEBUG_)
        fprintf(stderr, "\n!!!VIDEO_REFRESH_TIMER CALLED!!!\n");

    // retrieve global VideoState reference
    VideoState* videoState = (VideoState*)userdata;

    // check the video stream was correctly opened
    if (videoState->video_st)
    {
        // check the VideoPicture queue contains decoded frames
        if (videoState->pictq_size == 0)
        {
            if (_DEBUG_)
                fprintf(stderr, "\n!!!videoState->pictq_size == 0!!!\n");

            schedule_refresh(videoState, 1);
        }
        else
        {

            int RenderNthFrame = (int)(1.0 / global_video_state->PlayerSpeed);
            if (RenderNthFrame > 1 && (videoState->video_ctx->frame_number % RenderNthFrame) != 0)
            {
                // lock VideoPicture queue mutex
                SDL_LockMutex(videoState->pictq_mutex);

                // update read index for the next frame
                videoState->pictq_rindex = (videoState->pictq_rindex + 1) % VIDEO_PICTURE_QUEUE_SIZE;

                // decrease VideoPicture queue size
                videoState->pictq_size--;

                // notify other threads waiting for the VideoPicture queue
                SDL_CondSignal(videoState->pictq_cond);

                // unlock VideoPicture queue mutex
                SDL_UnlockMutex(videoState->pictq_mutex);

                // try to render a new frame as soon as possible
                schedule_refresh(videoState, 1);

                return;
            }

            // get VideoPicture reference using the queue read index
            VideoPicture* videoPicture = &videoState->pictq[videoState->pictq_rindex];

            Uint32 real_delay = CalculateImageShowTime(videoState, videoPicture->pts);

            schedule_refresh(videoState, (Uint32)(real_delay));

            // show the frame on the SDL_Surface (the screen)
            TriggerProfilePoint(PP_VIDEO_FRAME_RENDER);
            video_display(videoState);
            TriggerProfilePoint(PP_VIDEO_FRAME_RENDER, END_PROFILING);

            // lock VideoPicture queue mutex
            SDL_LockMutex(videoState->pictq_mutex);

            // update read index for the next frame
            videoState->pictq_rindex = (videoState->pictq_rindex + 1) % VIDEO_PICTURE_QUEUE_SIZE;

            // decrease VideoPicture queue size
            videoState->pictq_size--;

            // notify other threads waiting for the VideoPicture queue
            SDL_CondSignal(videoState->pictq_cond);

            // unlock VideoPicture queue mutex
            SDL_UnlockMutex(videoState->pictq_mutex);
        }
    }
    else
    {
        schedule_refresh(videoState, 100);
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
void video_display(VideoState* videoState)
{
    // reference for the next VideoPicture to be displayed
    VideoPicture* videoPicture;

    double aspect_ratio;

    int w, h, x, y;

    // get next VideoPicture to be displayed from the VideoPicture queue
    videoPicture = &videoState->pictq[videoState->pictq_rindex];

    if (videoPicture->frame)
    {
        if (videoState->video_ctx->sample_aspect_ratio.num == 0)
        {
            aspect_ratio = 0;
        }
        else
        {
            aspect_ratio = av_q2d(videoState->video_ctx->sample_aspect_ratio) * videoState->video_ctx->width / videoState->video_ctx->height;
        }

        if (aspect_ratio <= 0.0)
        {
            aspect_ratio = (float)videoState->video_ctx->width /
                (float)videoState->video_ctx->height;
        }

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
            if (_DEBUG_ || videoState->CreateInfo->ShowFrameInfo == 1)
            {
                // dump information about the frame being rendered
                int64_t TimeNow = av_gettime() / 1000; // time in milliseconds => 1000 times more precise than PTS
                uint64_t TimeVideoShouldHavePlayed = (int64_t)((videoPicture->pts - videoState->FrameShowTimeFirstFrame) * 1000);
                uint64_t TimeVideoActuallyPlayed = TimeNow - videoState->FrameShowTimeFirstPlayer;
                int64_t TimePassedFromPlayerPoint = videoState->FrameShowTimeFirstPlayer;
/*                printf(
                    "Frame %c (%d) pts %" PRId64 " dts %" PRId64 " key_frame %d PresentationTime %.02f PlayerTime %d MovieTime %d[coded_picture_number %d, display_picture_number %d, %dx%d]\n",
                    av_get_picture_type_char(videoPicture->frame->pict_type),
                    videoState->video_ctx->frame_number,
                    videoPicture->frame->pts,
                    videoPicture->frame->pkt_dts,
                    videoPicture->frame->key_frame,
                    (float)videoState->frame_last_pts,
                    (int)TimeVideoActuallyPlayed,
                    (int)TimeVideoShouldHavePlayed,
                    videoPicture->frame->coded_picture_number,
                    videoPicture->frame->display_picture_number,
                    videoPicture->frame->width,
                    videoPicture->frame->height        
                );*/
                printf(
                    "Display: Frame %c (%d) pts %" PRId64 " dts %" PRId64 " key_frame %d VPTime %.02f APT %.02f PlayerTime %d MovieTime %d\n",
                    av_get_picture_type_char(videoPicture->frame->pict_type),
                    videoState->video_ctx->frame_number,
                    videoPicture->frame->pts,
                    videoPicture->frame->pkt_dts,
                    videoPicture->frame->key_frame,
                    (float)videoState->frame_last_pts,
                    (float)videoState->audio_last_frame_pts,
                    (int)TimeVideoActuallyPlayed,
                    (int)TimeVideoShouldHavePlayed);
            }

            // set blit area x and y coordinates, width and height
            SDL_Rect rect;
            rect.x = x;
            rect.y = y;
            rect.w = w;
            rect.h = h;

            // lock screen mutex
            SDL_LockMutex(screen_mutex);

            // update the texture with the new pixel data
            SDL_UpdateYUVTexture(
                videoState->texture,
                &rect,
                videoPicture->frame->data[0],
                videoPicture->frame->linesize[0],
                videoPicture->frame->data[1],
                videoPicture->frame->linesize[1],
                videoPicture->frame->data[2],
                videoPicture->frame->linesize[2]
            );

            // clear the current rendering target with the drawing color
//            SDL_RenderClear(videoState->renderer);

            // copy a portion of the texture to the current rendering target
            SDL_RenderCopy(videoState->renderer, videoState->texture, NULL, NULL);

            // update the screen with any rendering performed since the previous call
            SDL_RenderPresent(videoState->renderer);

            // unlock screen mutex
            SDL_UnlockMutex(screen_mutex);
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
    }
}

/**
 * Allocates a new SDL_Overlay for the VideoPicture struct referenced by the
 * global VideoState struct reference.
 * The remaining VideoPicture struct fields are also updated.
 *
 * @param   userdata    the global VideoState reference.
 */
void alloc_picture(void* userdata)
{
    // retrieve global VideoState reference.
    VideoState* videoState = (VideoState*)userdata;

    // retrieve the VideoPicture pointed by the queue write index
    VideoPicture* videoPicture;
    videoPicture = &videoState->pictq[videoState->pictq_windex];

    // check if the SDL_Overlay is allocated
    if (videoPicture->frame)
    {
        // we already have an AVFrame allocated, free memory
        av_frame_free(&videoPicture->frame);
        av_free(videoPicture->frame);
    }

    // lock global screen mutex
    SDL_LockMutex(screen_mutex);

    // get the size in bytes required to store an image with the given parameters
    int numBytes;
    numBytes = av_image_get_buffer_size(
        AV_PIX_FMT_YUV420P,
        videoState->video_ctx->width,
        videoState->video_ctx->height,
        32
    );

    // allocate image data buffer
    uint8_t* buffer = NULL;
    buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));

    // alloc the AVFrame later used to contain the scaled frame
    videoPicture->frame = av_frame_alloc();
    if (videoPicture->frame == NULL)
    {
        printf("Could not allocate frame.\n");
        return;
    }

    // The fields of the given image are filled in by using the buffer which points to the image data buffer.
    av_image_fill_arrays(
        videoPicture->frame->data,
        videoPicture->frame->linesize,
        buffer,
        AV_PIX_FMT_YUV420P,
        videoState->video_ctx->width,
        videoState->video_ctx->height,
        32
    );

    // unlock global screen mutex
    SDL_UnlockMutex(screen_mutex);

    // update VideoPicture struct fields
    videoPicture->width = videoState->video_ctx->width;
    videoPicture->height = videoState->video_ctx->height;
    videoPicture->allocated = 1;
}
