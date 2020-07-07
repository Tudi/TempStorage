#include <stdio.h>
extern "C" {
    #include <libswresample/swresample.h>
    #include <libavutil/opt.h>
    #include <libavformat/avformat.h>
}

#pragma pack(push,1)
typedef struct wavfile_header_s
{
    char    ChunkID[4];     /*  4   */
    int     ChunkSize;      /*  4   */
    char    Format[4];      /*  4   */

    char    Subchunk1ID[4]; /*  4   */
    int     Subchunk1Size;  /*  4   */
    short   AudioFormat;    /*  2   */
    short   NumChannels;    /*  2   */
    int     SampleRate;     /*  4   */
    int     ByteRate;       /*  4   */
    short   BlockAlign;     /*  2   */
    short   BitsPerSample;  /*  2   */

    char    Subchunk2ID[4];
    int     Subchunk2Size;
} wavfile_header_t;
#pragma pack(pop,1)

#define SUBCHUNK1SIZE   (16)
#define FORCE_OUTPUT_CHANNEL_COUNT 1

int WriteWavHeader(FILE* file_p, int AudioFormat, int channels, int bitsPerSample, int SampleRate, int FrameCount)
{
    int ret;

    wavfile_header_t wav_header;
    int subchunk2_size;
    int chunk_size;

    size_t write_count;
 //   channels = FORCE_OUTPUT_CHANNEL_COUNT;

    subchunk2_size = FrameCount * channels * bitsPerSample / 8;
    chunk_size = 4 + (8 + SUBCHUNK1SIZE) + (8 + subchunk2_size);

    wav_header.ChunkID[0] = 'R';
    wav_header.ChunkID[1] = 'I';
    wav_header.ChunkID[2] = 'F';
    wav_header.ChunkID[3] = 'F';

    wav_header.ChunkSize = chunk_size;

    wav_header.Format[0] = 'W';
    wav_header.Format[1] = 'A';
    wav_header.Format[2] = 'V';
    wav_header.Format[3] = 'E';

    wav_header.Subchunk1ID[0] = 'f';
    wav_header.Subchunk1ID[1] = 'm';
    wav_header.Subchunk1ID[2] = 't';
    wav_header.Subchunk1ID[3] = ' ';

    wav_header.Subchunk1Size = SUBCHUNK1SIZE;
    wav_header.AudioFormat = 1; // 1 for PCM 
    wav_header.NumChannels = channels;
    wav_header.SampleRate = SampleRate;
    wav_header.ByteRate = SampleRate * channels * bitsPerSample / 8;
    wav_header.BlockAlign = channels * bitsPerSample / 8;
    wav_header.BitsPerSample = bitsPerSample;

    wav_header.Subchunk2ID[0] = 'd';
    wav_header.Subchunk2ID[1] = 'a';
    wav_header.Subchunk2ID[2] = 't';
    wav_header.Subchunk2ID[3] = 'a';
    wav_header.Subchunk2Size = subchunk2_size;

    write_count = fwrite(&wav_header, sizeof(wavfile_header_t), 1, file_p);

    ret = (1 != write_count) ? -1 : 0;

    return ret;
}

int ResampleAndWriteFrame(AVFrame* frame_, AVCodecContext* pCodecCtx, FILE *f)
{
    SwrContext* swrCtx_ = 0;
 
    // Initializing the sample rate convert. We only really use it to convert float output into int.
    int64_t wanted_channel_layout = AV_CH_LAYOUT_STEREO;
    int want_sample_rate = pCodecCtx->sample_rate;

    uint8_t* outbuf = NULL;
    av_samples_alloc((uint8_t**)&outbuf, NULL, pCodecCtx->channels, frame_->nb_samples, AV_SAMPLE_FMT_S16, 0);

#define USER_SAMPLEING
#ifdef USER_SAMPLEING
    if (pCodecCtx->sample_fmt == AV_SAMPLE_FMT_FLTP)
    {
        int nb_samples = frame_->nb_samples;
        int channels = frame_->channels;
        auto outputBuffer = (int16_t*)outbuf;

        int c = 0;
        for (int i = 0; i < nb_samples; i++) 
//            for (int c = 0; c < channels; c++)
            {
                float* extended_data = (float*)frame_->extended_data[c];
                float sample = extended_data[i];
                if (sample < -1.0f) sample = -1.0f;
                else if (sample > 1.0f) sample = 1.0f;
                outputBuffer[i] = (int16_t)round(sample * 32767.0f);
        }
        
        fwrite(outbuf, 1, nb_samples * sizeof(unsigned short), f);
        return 0;
    }
#endif
#if 0
    swrCtx_ = swr_alloc_set_opts(
        NULL, //swrCtx_,
        wanted_channel_layout,
        AV_SAMPLE_FMT_S16,
        want_sample_rate / 2,
        pCodecCtx->channel_layout,
        pCodecCtx->sample_fmt,
        pCodecCtx->sample_rate,
        0,
        NULL);

    if (!swrCtx_ || swr_init(swrCtx_) < 0) {
        printf("swr_init: Failed to initialize the resampling context");
        return -1;
    }

    // convert audio to AV_SAMPLE_FMT_S16
    int frame_count = swr_convert(swrCtx_, &outbuf, frame_->nb_samples, (const uint8_t**)frame_->extended_data, frame_->nb_samples);
    if (frame_count < 0) {
        printf("swr_convert: Error while converting %d", frame_count);
        return frame_count;
    }
    fwrite(outbuf, 1, frame_count * sizeof(unsigned short) * pCodecCtx->channels, f);

    swr_free(&swrCtx_);
    av_freep(&outbuf);
#endif

    return 0;
}