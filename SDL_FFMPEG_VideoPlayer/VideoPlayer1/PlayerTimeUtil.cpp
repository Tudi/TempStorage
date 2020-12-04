#include "StdAfx.h"


/**
 * Attempts to guess proper monotonic timestamps for decoded video frames which
 * might have incorrect times.
 *
 * Input timestamps may wrap around, in which case the output will as well.
 *
 * @param   ctx             the video AVCodecContext.
 * @param   reordered_pts   the pts field of the decoded AVPacket, as passed
 *                          through AVFrame.pts.
 * @param   dts             the pkt_dts field of the decoded AVPacket.
 *
 * @return                  one of the input values, may be AV_NOPTS_VALUE.
 */
int64_t guess_correct_pts(AVCodecContext* ctx, int64_t reordered_pts, int64_t dts)
{
    int64_t pts;

    if (dts != AV_NOPTS_VALUE)
    {
        ctx->pts_correction_num_faulty_dts += dts <= ctx->pts_correction_last_dts;
        ctx->pts_correction_last_dts = dts;
    }
    else if (reordered_pts != AV_NOPTS_VALUE)
    {
        ctx->pts_correction_last_dts = reordered_pts;
    }

    if (reordered_pts != AV_NOPTS_VALUE)
    {
        ctx->pts_correction_num_faulty_pts += reordered_pts <= ctx->pts_correction_last_pts;
        ctx->pts_correction_last_pts = reordered_pts;
    }
    else if (dts != AV_NOPTS_VALUE)
    {
        ctx->pts_correction_last_pts = dts;
    }

    if ((ctx->pts_correction_num_faulty_pts <= ctx->pts_correction_num_faulty_dts || dts == AV_NOPTS_VALUE) && reordered_pts != AV_NOPTS_VALUE)
    {
        pts = reordered_pts;
    }
    else if (dts != AV_NOPTS_VALUE)
    {
        pts = dts;
    }
    else
        pts = 0;

    return pts;
}

/**
 * So we're going to use a fractional coefficient, say c, and So now let's say
 * we've gotten N audio sample sets that have been out of sync. The amount we are
 * out of sync can also vary a good deal, so we're going to take an average of how
 * far each of those have been out of sync. So for example, the first call might
 * have shown we were out of sync by 40ms, the next by 50ms, and so on. But we're
 * not going to take a simple average because the most recent values are more
 * important than the previous ones. So we're going to use a fractional coefficient,
 * say c, and sum the differences like this: diff_sum = new_diff + diff_sum*c.
 * When we are ready to find the average difference, we simply calculate
 * avg_diff = diff_sum * (1-c).
 *
 * @param   videoState      the global VideoState reference.
 * @param   samples         global VideoState reference audio buffer.
 * @param   samples_size    last decoded audio AVFrame size after resampling.
 *
 * @return
 */
int synchronize_audio(VideoState* videoState, short* samples, int samples_size)
{
    int n;
    double ref_clock;

    n = 2 * videoState->audio_ctx->channels;

    // check if
    if (videoState->av_sync_type != AV_SYNC_AUDIO_MASTER)
    {
        double diff, avg_diff;
        int wanted_size, min_size, max_size /*, nb_samples */;

        ref_clock = get_master_clock(videoState);
        diff = get_audio_clock(videoState) - ref_clock;

        if (diff < AV_NOSYNC_THRESHOLD)
        {
            // accumulate the diffs
            videoState->audio_diff_cum = diff + videoState->audio_diff_avg_coef * videoState->audio_diff_cum;

            if (videoState->audio_diff_avg_count < AUDIO_DIFF_AVG_NB)
            {
                videoState->audio_diff_avg_count++;
            }
            else
            {
                avg_diff = videoState->audio_diff_cum * (1.0 - videoState->audio_diff_avg_coef);

                /**
                 * So we're doing pretty well; we know approximately how off the audio
                 * is from the video or whatever we're using for a clock. So let's now
                 * calculate how many samples we need to add or lop off by putting this
                 * code where the "Shrinking/expanding buffer code" section is:
                 */
                if (fabs(avg_diff) >= videoState->audio_diff_threshold)
                {
                    wanted_size = samples_size + ((int)(diff * videoState->audio_ctx->sample_rate) * n);
                    min_size = samples_size * ((100 - SAMPLE_CORRECTION_PERCENT_MAX) / 100);
                    max_size = samples_size * ((100 + SAMPLE_CORRECTION_PERCENT_MAX) / 100);

                    if (wanted_size < min_size)
                    {
                        wanted_size = min_size;
                    }
                    else if (wanted_size > max_size)
                    {
                        wanted_size = max_size;
                    }

                    /**
                     * Now we have to actually correct the audio. You may have noticed that our
                     * synchronize_audio function returns a sample size, which will then tell us
                     * how many bytes to send to the stream. So we just have to adjust the sample
                     * size to the wanted_size. This works for making the sample size smaller.
                     * But if we want to make it bigger, we can't just make the sample size larger
                     * because there's no more data in the buffer! So we have to add it. But what
                     * should we add? It would be foolish to try and extrapolate audio, so let's
                     * just use the audio we already have by padding out the buffer with the
                     * value of the last sample.
                     */
                    if (wanted_size < samples_size)
                    {
                        /* remove samples */
                        samples_size = wanted_size;
                    }
                    else if (wanted_size > samples_size)
                    {
                        uint8_t* samples_end, * q;
                        int nb;

                        /* add samples by copying final sample*/
                        nb = (samples_size - wanted_size);
                        samples_end = (uint8_t*)samples + samples_size - n;
                        q = samples_end + n;

                        while (nb > 0)
                        {
                            memcpy(q, samples_end, n);
                            q += n;
                            nb -= n;
                        }

                        samples_size = wanted_size;
                    }
                }
            }
        }
        else
        {
            /* difference is TOO big; reset diff stuff */
            videoState->audio_diff_avg_count = 0;
            videoState->audio_diff_cum = 0;
        }
    }

    return samples_size;
}

/**
 * Calculates and returns the current audio clock reference value.
 *
 * @param   videoState  the global VideoState reference.
 *
 * @return              the current audio clock reference value.
 */
double get_audio_clock(VideoState* videoState)
{
    double pts = videoState->audio_clock;

    int hw_buf_size = videoState->audio_buf_size - videoState->audio_buf_index;

    int bytes_per_sec = 0;

    int n = 2 * videoState->audio_ctx->channels;

    if (videoState->audio_st)
    {
        bytes_per_sec = videoState->audio_ctx->sample_rate * n;
    }

    if (bytes_per_sec)
    {
        pts -= (double)hw_buf_size / bytes_per_sec;
    }

    return pts;
}

/**
 * Calculates and returns the current video clock reference value.
 *
 * @param   videoState  the global VideoState reference.
 *
 * @return              the current video clock reference value.
 */
double get_video_clock(VideoState* videoState)
{
    double delta = (av_gettime() - videoState->video_current_pts_time) / 1000000.0;

    return videoState->video_current_pts + delta;
}

/**
 * Calculates and returns the current external clock reference value: the computer
 * clock.
 *
 * @return  the current external clock reference value.
 */
double get_external_clock(VideoState* videoState)
{
    videoState->external_clock_time = av_gettime();
    videoState->external_clock = videoState->external_clock_time / 1000000.0;

    return videoState->external_clock;
}

/**
 * Checks the VideoState global reference av_sync_type variable and then calls
 * get_audio_clock, get_video_clock, or get_external_clock accordingly.
 *
 * @param   videoState  the global VideoState reference.
 *
 * @return              the reference clock according to the chosen AV sync type.
 */
double get_master_clock(VideoState* videoState)
{
    if (videoState->av_sync_type == AV_SYNC_VIDEO_MASTER)
    {
        return get_video_clock(videoState);
    }
    else if (videoState->av_sync_type == AV_SYNC_AUDIO_MASTER)
    {
        return get_audio_clock(videoState);
    }
    else if (videoState->av_sync_type == AV_SYNC_EXTERNAL_MASTER)
    {
        return get_external_clock(videoState);
    }
    else
    {
        fprintf(stderr, "Error: Undefined A/V sync type.");
        return -1;
    }
}

/**
 * Updates the PTS of the last decoded video frame to be in sync with everything.
 * This function will also deal with cases where we don't get a PTS value
 * for our frame. At the same time we need to keep track of when the next frame
 * is expected so we can set our refresh rate properly. We can accomplish this by
 * using the VideoState internal video_clock value which keeps track of how much
 * time has passed according to the video.
 *
 * You'll notice we account for repeated frames in this function, too.
 *
 * @param   videoState  the global VideoState reference.
 * @param   src_frame   last decoded video AVFrame, not yet queued in the
 *                      VideoPicture queue.
 * @param   pts         the pts of the last decoded video AVFrame obtained using
 *                      FFmpeg guess_correct_pts.
 *
 * @return              the updated (synchronized) pts for the given video AVFrame.
 */
double synchronize_video(VideoState* videoState, AVFrame* src_frame, double pts)
{
    double frame_delay;

    if (pts != 0)
    {
        // if we have pts, set video clock to it
        videoState->video_clock = pts;
    }
    else
    {
        // if we aren't given a pts, set it to the clock
        pts = videoState->video_clock;
    }

    // update the video clock
    frame_delay = av_q2d(videoState->video_ctx->time_base);

    // if we are repeating a frame, adjust clock accordingly
    frame_delay += src_frame->repeat_pict * (frame_delay * 0.5);

    // increase video clock to match the delay required for repeaing frames
    videoState->video_clock += frame_delay;

    return pts;
}

//how muh time should we show this image while playing the movie ?
Uint32 CalculateImageShowTime(VideoState* videoState, double PicturePTS)
{
/*
    if (_DEBUG_)
    {
        printf("Current Frame PTS:\t\t%f\n", videoPicture->pts);
        printf("Last Frame PTS:\t\t\t%f\n", videoState->frame_last_pts);
    }*/

    // used for video frames display delay and audio video sync
    double pts_delay;
    //#define Theoretical_PTS_CALCULATION
#ifdef Theoretical_PTS_CALCULATION
    double audio_ref_clock;
    double sync_threshold;
    double audio_video_delay;

    // get last frame pts
    pts_delay = videoPicture->pts - videoState->frame_last_pts;

    if (_DEBUG_)
        printf("PTS Delay:\t\t\t\t%f\n", pts_delay);

    // if the obtained delay is incorrect
    if (pts_delay <= 0 || pts_delay >= 1.0)
    {
        // use the previously calculated delay
        pts_delay = videoState->frame_last_delay;
    }

    if (_DEBUG_)
        printf("Corrected PTS Delay:\t%f\n", pts_delay);

    // save delay information for the next time
    videoState->frame_last_delay = pts_delay;
    videoState->frame_last_pts = videoPicture->pts;

    // in case the external clock is not used
    if (videoState->av_sync_type != AV_SYNC_VIDEO_MASTER)
    {
        // update delay to stay in sync with the master clock: audio or video
        audio_ref_clock = get_master_clock(videoState);

        if (_DEBUG_)
            printf("Ref Clock:\t\t\t\t%f\n", audio_ref_clock);

        // calculate audio video delay accordingly to the master clock
        audio_video_delay = videoPicture->pts - audio_ref_clock;

        if (_DEBUG_)
            printf("Audio Video Delay:\t\t%f\n", audio_video_delay);

        // skip or repeat the frame taking into account the delay
        sync_threshold = (pts_delay > AV_SYNC_THRESHOLD) ? pts_delay : AV_SYNC_THRESHOLD;

        if (_DEBUG_)
            printf("Sync Threshold:\t\t\t%f\n", sync_threshold);

        // check audio video delay absolute value is below sync threshold
        if (fabs(audio_video_delay) < AV_NOSYNC_THRESHOLD)
        {
            if (audio_video_delay <= -sync_threshold)
            {
                pts_delay = 0;
            }
            else if (audio_video_delay >= sync_threshold)
            {
                pts_delay = 2 * pts_delay;
            }
        }
    }

    if (_DEBUG_)
        printf("Corrected PTS delay:\t%f\n", pts_delay);

    videoState->frame_timer += pts_delay;

    // compute the real delay
    real_delay = videoState->frame_timer - (av_gettime() / 1000000.0);

    if (_DEBUG_)
        printf("Real Delay:\t\t\t\t%f\n", real_delay);

    if (real_delay < 0.010)
    {
        real_delay = 0.010;
    }

    if (_DEBUG_)
        printf("Corrected Real Delay:\t%f\n", real_delay);


    if (_DEBUG_)
        printf("Next Scheduled Refresh:\t%f\n\n", real_delay);
    //            schedule_refresh(videoState, (Uint32)(real_delay * 1000 + 0.5));
#else
    //cumulative rendering delays should be corrected
    {
        //try to guess movie FPS
        int expected_FPS = videoState->video_ctx->framerate.num;
        if (expected_FPS < 1)
            expected_FPS = 25;
        if(expected_FPS > 120)
            expected_FPS = 25;
        double ExpectedFrameDisplayDuration = 1.0 / expected_FPS;
        if (videoState->FramesUsedForSum < 100 
            && PicturePTS - videoState->frame_last_pts > ExpectedFrameDisplayDuration / 2
            && PicturePTS - videoState->frame_last_pts < ExpectedFrameDisplayDuration * 2)
        {
            videoState->FrameShowSumTime += (PicturePTS - videoState->frame_last_pts);
            videoState->FramesUsedForSum++;
        }
        if (videoState->FramesUsedForSum == 100)
            ExpectedFrameDisplayDuration = videoState->FrameShowSumTime / 100;

        //should initialize this every time pause/seek... happens
        pts_delay = PicturePTS - videoState->frame_last_pts;
        videoState->frame_last_pts = PicturePTS;

        int64_t TimeNow = av_gettime() / 1000; // time in milliseconds => 1000 times more precise than PTS
        //should reinitialize when you push play after seek or pause
        if (videoState->FrameShowTimeFirstPlayer <= 0 
            || pts_delay < ExpectedFrameDisplayDuration / 2 || pts_delay > ExpectedFrameDisplayDuration * 2
            )
        {
            videoState->FrameShowTimeFirstPlayer = TimeNow;
            videoState->FrameShowTimeFirstFrame = (int64_t)(PicturePTS);
            pts_delay = ExpectedFrameDisplayDuration;
        }
        double real_delay = pts_delay * 1000;
        double TimeVideoShouldHavePlayed = (PicturePTS - videoState->FrameShowTimeFirstFrame) * 1000.0;
        double TimeVideoActuallyPlayed = (double)(TimeNow - videoState->FrameShowTimeFirstPlayer);
        double PlayerSpeedup = 1.0;
        if (TimeVideoActuallyPlayed != 0)
        {
            PlayerSpeedup = (double)TimeVideoShouldHavePlayed / (double)TimeVideoActuallyPlayed;
            if (PlayerSpeedup < 0.5)
                PlayerSpeedup = 0.5;
            if (PlayerSpeedup > 2)
                PlayerSpeedup = 2;
        }
        real_delay *= PlayerSpeedup;
        
        //try to syncronize to audio. Audio play buffer should be less than 1 second
        //if video is playing faster than audio, try to slow down
        if (videoState->MuteAudio == 0)
        {
            if (videoState->audio_last_frame_pts + 0.1 < videoState->frame_last_pts)
                real_delay += 5; //5 microseconds per frame should be enough to avoid extra latency
            else if (videoState->audio_last_frame_pts - 0.1 > videoState->frame_last_pts)
                real_delay -= 5; //5 microseconds per frame should be enough to avoid extra latency
        }

        //adjust play speed
        if(global_video_state->PlayerSpeed < 1)
            real_delay *= global_video_state->PlayerSpeed;
        else if (global_video_state->PlayerSpeed > 1)
        {
            double FractionalPart = global_video_state->PlayerSpeed - (int)global_video_state->PlayerSpeed + 1.0;
            real_delay *= FractionalPart;
        }

        if (_DEBUG_)
            printf("Next Scheduled Refresh:\t%f Speed coeff %0.03f\n\n", real_delay, PlayerSpeedup);
        return (Uint32)real_delay;
    }
#endif
}