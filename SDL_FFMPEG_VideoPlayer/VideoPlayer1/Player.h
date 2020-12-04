#pragma once


extern "C"
{
    #include <libavutil/frame.h>
}
#include "DataQueue.h"

/**
 * Prevents SDL from overriding main().
 */
#ifdef __MINGW32__
#undef main
#endif

 /**
  * Debug flag.
  */
#define _DEBUG_ 0

  /**
   * SDL audio buffer size in samples.
   */
#define SDL_AUDIO_BUFFER_SIZE 1024

   /**
    * Maximum number of samples per channel in an audio frame.
    */
#define MAX_AUDIO_FRAME_SIZE 192000

    /**
     * Audio packets queue maximum size.
     */
#define MAX_AUDIOQ_SIZE (5 * 16 * 1024)

     /**
      * Video packets queue maximum size.
      */
#define MAX_VIDEOQ_SIZE (5 * 256 * 1024)

      /**
       * AV sync correction threshold.
       */
#define AV_SYNC_THRESHOLD 0.01

       /**
        * No AV sync correction threshold.
        */
#define AV_NOSYNC_THRESHOLD 1.0

        /**
         *
         */
#define SAMPLE_CORRECTION_PERCENT_MAX 10

         /**
          *
          */
#define AUDIO_DIFF_AVG_NB 20

          /**
           * Custom SDL_Event type.
           * Notifies the next video frame has to be displayed.
           */
#define FF_REFRESH_EVENT (SDL_USEREVENT)

           /**
            * Custom SDL_Event type.
            * Notifies the program needs to quit.
            */
#define FF_QUIT_EVENT (SDL_USEREVENT + 1)

            /**
             * Video Frame queue size.
             */
#define VIDEO_PICTURE_QUEUE_SIZE 1

             /**
              * Default audio video sync type.
              */
#define DEFAULT_AV_SYNC_TYPE AV_SYNC_AUDIO_MASTER

struct AVFrame;
struct AVFormatContext;
struct AVStream;
struct AVCodecContext;

/**
 * Queue structure used to store processed video frames.
 */
typedef struct VideoPicture
{
    AVFrame* frame;
    int         width;
    int         height;
    int         allocated;
    double      pts;
} VideoPicture;

struct PlayerCreateParams;

/**
 * Struct used to hold the format context, the indices of the audio and video stream,
 * the corresponding AVStream objects, the audio and video codec information,
 * the audio and video queues and buffers, the global quit flag and the filename of
 * the movie.
 */
typedef struct VideoState
{
    /**
     * File I/O Context.
     */
    AVFormatContext* pFormatCtx;

    /**
     * Audio Stream.
     */
    int                 audioStream;
    AVStream* audio_st;
    AVCodecContext* audio_ctx;
    PacketQueue         audioq;
    uint8_t             audio_buf[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
    unsigned int        audio_buf_size;
    unsigned int        audio_buf_index;
    AVFrame             audio_frame;
    AVPacket            audio_pkt;
    uint8_t* audio_pkt_data;
    int                 audio_pkt_size;
    double              audio_clock;

    /**
     * Video Stream.
     */
    int                 videoStream;
    AVStream* video_st;
    AVCodecContext* video_ctx;
    SDL_Texture* texture;
    SDL_Renderer* renderer;
    PacketQueue         videoq;
    struct SwsContext* sws_ctx;
    double              frame_timer;
    double              frame_last_pts;
    double              frame_last_delay;
    int64_t             FrameShowTimeFirstPlayer; //should reset it after seek/pause...
    int64_t             FrameShowTimeFirstFrame; //should reset it after seek/pause...
    double              video_clock;
    double              video_current_pts;
    int64_t             video_current_pts_time;
    double              audio_diff_cum;
    double              audio_diff_avg_coef;
    double              audio_diff_threshold;
    int                 audio_diff_avg_count;
    double              audio_last_frame_pts;

    /**
     * VideoPicture Queue.
     */
    VideoPicture        pictq[VIDEO_PICTURE_QUEUE_SIZE];
    int                 pictq_size;
    int                 pictq_rindex;
    int                 pictq_windex;
    SDL_mutex* pictq_mutex;
    SDL_cond* pictq_cond;

    /**
     * AV Sync.
     */
    int     av_sync_type;
    double  external_clock;
    int64_t external_clock_time;

    /**
     * Seeking.
     */
    int     seek_req;
    int     seek_flags;
    int64_t seek_pos;
    double  FirstVideoFramePTSAfterSeek; // First key frame might not be first frame after seek

    /**
     * Threads.
     */
    SDL_Thread* decode_tid;
    SDL_Thread* video_tid;

    /**
     * Input file name.
     */
    char filename[1024];

    /**
     * Global quit flag.
     */
    int quit;

    /**
     * Maximum number of frames to be decoded.
     */
    int     currentFrameIndex;
    //you need to create the window on main thread to be able to handle events from the main thread
    SDL_Window* gWindow;
    int     PausePlay;
    PlayerCreateParams *CreateInfo;
    double  FrameShowSumTime; // should be 1/(average FPS)
    int     FramesUsedForSum; // we need to calculate an everage over at least 1 second
    double  PlayerSpeed; // fast/slow playback
    int     MuteAudio;  // when fast or slow playback, we simply erase the content of the audio packets
} VideoState;


/**
 * Global VideoState reference.
 */
extern VideoState* global_video_state;


int decode_thread(void* arg);

int stream_component_open(
    VideoState* videoState,
    int stream_index
);

void schedule_refresh(
    VideoState* videoState,
    Uint32 delay
);

Uint32 sdl_refresh_timer_cb(
    Uint32 interval,
    void* param
);

void stream_seek(VideoState* videoState, int64_t pos, int rel);

/**
 *
 */
extern AVPacket flush_pkt;

struct PlayerCreateParams;
int PlayerCreate(PlayerCreateParams* Params);

#define HasFlag(val,flag) (val & flag)
#define RemoveFlag(val,flag) (val & (~flag))