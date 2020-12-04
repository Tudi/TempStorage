#pragma once

/**
 * Audio Video Sync Types.
 */
enum
{
    /**
     * Sync to audio clock.
     */
    AV_SYNC_AUDIO_MASTER,

    /**
     * Sync to video clock.
     */
     AV_SYNC_VIDEO_MASTER,

     /**
      * Sync to external clock: the computer clock
      */
      AV_SYNC_EXTERNAL_MASTER,
};

double get_audio_clock(VideoState* videoState);

double get_video_clock(VideoState* videoState);

double get_external_clock(VideoState* videoState);

double get_master_clock(VideoState* videoState);

double synchronize_video(
    VideoState* videoState,
    AVFrame* src_frame,
    double pts
);

int synchronize_audio(
    VideoState* videoState,
    short* samples,
    int samples_size
);

int64_t guess_correct_pts(
    AVCodecContext* ctx,
    int64_t reordered_pts,
    int64_t dts
);

//how muh time should we show this image while playing the movie ?
Uint32 CalculateImageShowTime(VideoState* videoState, double PicturePTS);