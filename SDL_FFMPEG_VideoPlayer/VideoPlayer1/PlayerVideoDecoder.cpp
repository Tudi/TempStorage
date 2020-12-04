#include "StdAfx.h"
/**
 * This function is used as callback for the SDL_Thread.
 *
 * This thread reads in packets from the video queue, packet_queue_get(), decodes
 * the video packets into a frame, and then calls the queue_picture() function to
 * put the processed frame into the picture queue.
 *
 * @param   arg the data pointer passed to the SDL_Thread callback function.
 *
 * @return
 */
int video_thread(void* arg)
{
    // retrieve global VideoState reference
    VideoState* videoState = (VideoState*)arg;

    // allocate an AVPacket to be used to retrieve data from the videoq.
    AVPacket* packet = av_packet_alloc();
    if (packet == NULL)
    {
        printf("Could not allocate AVPacket.\n");
        return -1;
    }

    // set this when we are done decoding an entire frame
    int frameFinished = 0;

    // allocate a new AVFrame, used to decode video packets
    static AVFrame* pFrame = NULL;
    pFrame = av_frame_alloc();
    if (!pFrame)
    {
        printf("Could not allocate AVFrame.\n");
        return -1;
    }

    // each decoded frame carries its PTS in the VideoPicture queue
    double pts;

    for (;;)
    {
        // get a packet from the video PacketQueue
        int ret = packet_queue_get(&videoState->videoq, packet, 1);
        if (ret < 0)
        {
            // means we quit getting packets
            break;
        }

        if (packet->data == flush_pkt.data)
        {
            avcodec_flush_buffers(videoState->video_ctx);
            packet_queue_flush(&videoState->videoq);
            videoState->FirstVideoFramePTSAfterSeek = -1;
            continue;
        }

        TriggerProfilePoint(PP_VIDEO_FRAME_DECODE);

        // give the decoder raw compressed data in an AVPacket
        ret = avcodec_send_packet(videoState->video_ctx, packet);
        if (ret < 0)
        {
            printf("Error sending video packet for decoding.\n");
            return -1; // this will throw 1 error after seeking, but it's ok
        }

        while (ret >= 0)
        {
            // get decoded output data from decoder
            ret = avcodec_receive_frame(videoState->video_ctx, pFrame);

            // check an entire frame was decoded
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            {
                break;
            }
            else if (ret < 0)
            {
                printf("Error while decoding.\n");
                return -1;
            }
            else
            {
                frameFinished = 1;
            }

            //flushing decoder state might trigger spitting out a frame instantly
            if (videoState->seek_req)
                continue;

            // attempt to guess proper monotonic timestamps for decoded video frames
            pts = (double)guess_correct_pts(videoState->video_ctx, pFrame->pts, pFrame->pkt_dts);
            pts *= av_q2d(videoState->video_st->time_base);


            // did we get an entire video frame?
            if (frameFinished)
            {
                TriggerProfilePoint(PP_VIDEO_FRAME_DECODE, END_PROFILING);

                //                pts = synchronize_video(videoState, pFrame, pts);

                if (videoState->CreateInfo->ShowFrameInfo)
                {
                    double packet_timestamp = (double)packet->pts * av_q2d(videoState->video_st->time_base);
                    printf("Decoder: video frame pts=%lld, packet Timestamp=%f ByteSize=%d PictNum=%d DecodeDuration=%lld\n", packet->pts, (float)packet_timestamp, packet->size, pFrame->coded_picture_number, GetProfileTimeUS(PP_VIDEO_FRAME_DECODE));
                }

                if (queue_picture(videoState, pFrame, pts) < 0)
                {
                    break;
                }

                //after seek, catch first video frame PTS that we can decode
                if (videoState->FirstVideoFramePTSAfterSeek < 0)
                    videoState->FirstVideoFramePTSAfterSeek = pts;
            }
        }

        // wipe the packet
        av_packet_unref(packet);
    }

    // wipe the frame
    av_frame_free(&pFrame);
    av_free(pFrame);

    return 0;
}
