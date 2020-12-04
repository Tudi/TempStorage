#pragma once

extern "C"
{
    #include <libavformat/avformat.h>
    #include <SDL.h>
}

/**
 * Queue structure used to store AVPackets.
 */
typedef struct PacketQueue
{
    AVPacketList* first_pkt;
    AVPacketList* last_pkt;
    int             nb_packets;
    int             size;
    SDL_mutex* mutex;
    SDL_cond* cond;
} PacketQueue;


void packet_queue_init(PacketQueue* q);

int packet_queue_put(
    PacketQueue* queue,
    AVPacket* packet,
    int PutInFront = 0
);

int packet_queue_get(
    PacketQueue* queue,
    AVPacket* packet,
    int blocking
);

void packet_queue_flush(PacketQueue* queue);
