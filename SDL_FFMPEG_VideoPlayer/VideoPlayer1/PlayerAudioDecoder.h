#pragma once

/**
 * Struct used to hold data fields used for audio resampling.
 */
typedef struct AudioResamplingState
{
    SwrContext* swr_ctx;
    int64_t in_channel_layout;
    uint64_t out_channel_layout;
    int out_nb_channels;
    int out_linesize;
    int in_nb_samples;
    int64_t out_nb_samples;
    int64_t max_out_nb_samples;
    uint8_t** resampled_data;
    int resampled_data_size;

} AudioResamplingState;

int audio_decode_frame(
    VideoState* videoState,
    uint8_t* audio_buf,
    int buf_size,
    double* pts_ptr
);

int audio_resampling(
    VideoState* videoState,
    AVFrame* decoded_audio_frame,
    enum AVSampleFormat out_sample_fmt,
    uint8_t* out_buf
);

AudioResamplingState* getAudioResampling(uint64_t channel_layout);

void FlushAudioQueue(VideoState* videoState);