#pragma once

/**
 * Global SDL_Window reference.
 */
extern SDL_Window* screen;

/**
 * Global SDL_Surface mutex reference.
 */
extern SDL_mutex* screen_mutex;

int queue_picture(
    VideoState* videoState,
    AVFrame* pFrame,
    double pts
);


void video_refresh_timer(void* userdata);

void video_display(VideoState* videoState);

void alloc_picture(void* userdata);
