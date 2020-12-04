#pragma once


extern "C"
{
    #include <libavutil/frame.h>
}

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
           * Custom SDL_Event type.
           * Notifies the next video frame has to be displayed.
           */
#define FF_REFRESH_EVENT (SDL_USEREVENT)

           /**
            * Custom SDL_Event type.
            * Notifies the program needs to quit.
            */
#define FF_QUIT_EVENT (SDL_USEREVENT + 1)

struct AVFrame;
struct AVFormatContext;
struct AVStream;
struct AVCodecContext;

struct DecoderStateStore;
struct PlayerCreateParams;

/**
 * Struct used to hold the format context, the indices of the audio and video stream,
 * the corresponding AVStream objects, the audio and video codec information,
 * the audio and video queues and buffers, the global quit flag and the filename of
 * the movie.
 */
typedef struct VideoState
{
    DecoderStateStore   *ds; // store decoder related info
    SDL_Texture         *texture; // this will hold the last image obtained from decoder to be rendered on the screen
    SDL_Renderer        *renderer; // SDL renderer
    SDL_Thread          *decode_tid; // decoder thread will run in the background to provide us with raw data
    int                 quit; // when set to 1, make all threads exit
    //you need to create the window on main thread to be able to handle events from the main thread
    SDL_Window          *gWindow;
    DecodedAudioFrame   *AudioFrame; //hold the last obtained audio chunk
    int                  AudioBytesConsumed; //there will be times when Audio card does not need all the info we obtained from decoder
    double              Audio_Timestamp; // last used audio timestamp
    double              Video_Timestamp; // last used video timestamp
    int                 currentFrameIndex; // last decoded frame index. Will continue inceasing after seek
    int                 PausePlay;
    PlayerCreateParams  *CreateInfo;
    double              AvgFPS; // can be a wild guess
    double              PlayerSpeed; // fast/slow playback
    int                 MuteAudio;  // when fast or slow playback, we simply erase the content of the audio packets
    int64_t             TimestampAfterPause; // in case audio is not present, we need to compensate rendering time with movie internal time
    int64_t             FramesRenderedSincePause;
} VideoState;


/**
 * Global VideoState reference.
 */
extern VideoState* global_video_state;

int decode_thread(void* arg);
void schedule_refresh(VideoState* videoState, Uint32 delay);
Uint32 sdl_refresh_timer_cb(Uint32 interval, void* param);
struct PlayerCreateParams;
int PlayerCreate(PlayerCreateParams* Params, DecoderStateStore* ds);

void video_refresh_timer(void* userdata);
void audio_callback(void* userdata, Uint8* stream, int len);
void video_display(VideoState* videoState, DecodedVideoFrame* vf);

/**
 * Global SDL_Window reference.
 */
extern SDL_Window* screen;

/**
 * Global SDL_Surface mutex reference.
 */
extern SDL_mutex* screen_mutex;

#ifndef MIN
    #define MIN(a,b) ((a)<(b)?(a):(b))
#endif