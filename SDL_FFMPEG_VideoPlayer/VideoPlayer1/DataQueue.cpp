#include "StdAfx.h"

/**
 * Get the first AVPacket from the given PacketQueue.
 *
 * @param   queue       the PacketQueue to extract from.
 * @param   packet      the first AVPacket extracted from the queue.
 * @param   blocking    0 to avoid waiting for an AVPacket to be inserted in the given
 *                      queue, != 0 otherwise.
 *
 * @return              < 0 if returning because the quit flag is set, 0 if the queue
 *                      is empty, 1 if it is not empty and a packet was extracted.
 */
int packet_queue_get(PacketQueue* queue, AVPacket* packet, int blocking)
{
    int ret;

    AVPacketList* avPacketList;

    // lock mutex
    SDL_LockMutex(queue->mutex);

    for (;;)
    {
        // check quit flag
        if (global_video_state == NULL || global_video_state->quit)
        {
            ret = -1;
            break;
        }

        // point to the first AVPacketList in the queue
        avPacketList = queue->first_pkt;

        // if the first packet is not NULL, the queue is not empty
        if (avPacketList)
        {
            // place the second packet in the queue at first position
            queue->first_pkt = avPacketList->next;

            // check if queue is empty after removal
            if (!queue->first_pkt)
            {
                // first_pkt = last_pkt = NULL = empty queue
                queue->last_pkt = NULL;
            }

            // decrease the number of packets in the queue
            queue->nb_packets--;

            // decrease the size of the packets in the queue
            queue->size -= avPacketList->pkt.size;

            // point packet to the extracted packet, this will return to the calling function
            *packet = avPacketList->pkt;

            // free memory
            av_free(avPacketList);

            ret = 1;
            break;
        }
        else if (!blocking)
        {
            ret = 0;
            break;
        }
        else
        {
            // unlock mutex and wait for cond signal, then lock mutex again
            SDL_CondWait(queue->cond, queue->mutex);
        }
    }

    // unlock mutex
    SDL_UnlockMutex(queue->mutex);

    return ret;
}

/**
 *
 * @param queue
 */
void packet_queue_flush(PacketQueue* queue)
{
    AVPacketList* pkt, * pkt1;

    SDL_LockMutex(queue->mutex);

    pkt = queue->first_pkt;
    for (; pkt != NULL; pkt = pkt1)
    {
        pkt1 = pkt->next;
        //        av_free_packet(&pkt->pkt);
        av_packet_unref(&pkt->pkt);
        av_freep(&pkt);
    }

    queue->last_pkt = NULL;
    queue->first_pkt = NULL;
    queue->nb_packets = 0;
    queue->size = 0;

    SDL_UnlockMutex(queue->mutex);
}

/**
 * Initialize the given PacketQueue.
 *
 * @param q the PacketQueue to be initialized.
 */
void packet_queue_init(PacketQueue* q)
{
    // alloc memory for the audio queue
    memset(
        q,
        0,
        sizeof(PacketQueue)
    );

    // Returns the initialized and unlocked mutex or NULL on failure
    q->mutex = SDL_CreateMutex();
    if (!q->mutex)
    {
        // could not create mutex
        printf("SDL_CreateMutex Error: %s.\n", SDL_GetError());
        return;
    }

    // Returns a new condition variable or NULL on failure
    q->cond = SDL_CreateCond();
    if (!q->cond)
    {
        // could not create condition variable
        printf("SDL_CreateCond Error: %s.\n", SDL_GetError());
        return;
    }
}

/**
 * Put the given AVPacket in the given PacketQueue.
 *
 * @param  queue    the queue to be used for the insert
 * @param  packet   the AVPacket to be inserted in the queue
 *
 * @return          0 if the AVPacket is correctly inserted in the given PacketQueue.
 */
int packet_queue_put(PacketQueue* queue, AVPacket* packet, int PutInFront)
{
    // alloc the new AVPacketList to be inserted in the audio PacketQueue
    AVPacketList* avPacketList;
    avPacketList = (AVPacketList*)av_malloc(sizeof(AVPacketList));

    // check the AVPacketList was allocated
    if (!avPacketList)
    {
        return -1;
    }

    // add reference to the given AVPacket
    avPacketList->pkt = *packet;

    // the new AVPacketList will be inserted at the end of the queue
    avPacketList->next = NULL;

    // lock mutex
    SDL_LockMutex(queue->mutex);

    if (PutInFront)
    {
        avPacketList->next = queue->first_pkt;
        queue->first_pkt = avPacketList;
    }
    else
    {
        // check the queue is empty
        if (!queue->last_pkt)
        {
            // if it is, insert as first
            queue->first_pkt = avPacketList;
        }
        else
        {
            // if not, insert as last
            queue->last_pkt->next = avPacketList;
        }

        // point the last AVPacketList in the queue to the newly created AVPacketList
        queue->last_pkt = avPacketList;
    }
    // increase by 1 the number of AVPackets in the queue
    queue->nb_packets++;

    // increase queue size by adding the size of the newly inserted AVPacket
    queue->size += avPacketList->pkt.size;

    // notify packet_queue_get which is waiting that a new packet is available
    SDL_CondSignal(queue->cond);

    // unlock mutex
    SDL_UnlockMutex(queue->mutex);

    return 0;
}
