#ifndef MT_QUEUE_H
#define MT_QUEUE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Types

typedef
struct
{
    void* q;
} MtQueue_t;

typedef void (*MtQueueFreeFunction_t)(void*);

// Constants

#define MtQueue_NULL ((MtQueue_t) { .q = NULL })

// Functions

MtQueue_t mtQueue_init(uint16_t numEntries, MtQueueFreeFunction_t freeFunction);
void mtQueue_free(MtQueue_t queue);
bool mtQueue_isNull(MtQueue_t queue);

void mtQueue_push(MtQueue_t queue, void* item);
void* mtQueue_pop(MtQueue_t queue);
size_t mtQueue_size(MtQueue_t queue);

void MtQueueThreadCleanup(MtQueue_t queue);

#endif // MT_QUEUE_H
