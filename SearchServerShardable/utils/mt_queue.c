#include <mt_queue.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

//
// Types
//

typedef
struct
{
    uint16_t capacity;
    uint16_t count;
    void** queue;

    uint16_t front;
    uint16_t back;

    pthread_mutex_t mutex;
    pthread_cond_t readCondition;
    pthread_cond_t writeCondition;

    MtQueueFreeFunction_t freeFunction;
} MtQueueData_t;

//
// External interface
//

MtQueue_t mtQueue_init(uint16_t numEntries, MtQueueFreeFunction_t freeFunction)
{
    MtQueue_t mq = MtQueue_NULL;

    MtQueueData_t* data = (MtQueueData_t*) malloc(sizeof(MtQueueData_t)); 
    if(data == NULL) { return mq; }

    data->queue = (void**) malloc(numEntries * sizeof(void*));
    if(data->queue == NULL)
    {
        free(data);
        return mq;
    }

    data->capacity = numEntries;
    data->count    = 0;
    data->front    = 0;
    data->back     = 0;

    pthread_mutex_init(&data->mutex, NULL);
    pthread_cond_init(&data->readCondition, NULL);
    pthread_cond_init(&data->writeCondition, NULL);

    data->freeFunction = freeFunction;

    mq.q = data;

    return mq;
}

void mtQueue_free(MtQueue_t queue)
{
    MtQueueData_t* data = (MtQueueData_t*) queue.q;
    if(data == NULL) { return; }

    for(; data->count != 0; --(data->count))
    {
        data->freeFunction(data->queue[data->front]);
        ++(data->front);
        data->front %= data->capacity;
    }

    free(data->queue);
    free(data);
}

bool mtQueue_isNull(MtQueue_t queue)
{
    return queue.q == NULL;
}

void mtQueue_push(MtQueue_t queue, void* item)
{
    MtQueueData_t* data = (MtQueueData_t*) queue.q;

    pthread_mutex_lock(&data->mutex);

    while(data->count == data->capacity) {
        pthread_cond_wait(&data->writeCondition, &data->mutex);
    }

    data->queue[data->back] = item;
    ++(data->back);
    data->back %= data->capacity;
    ++(data->count);

    pthread_cond_signal(&data->readCondition);
    pthread_mutex_unlock(&data->mutex);
}

void* mtQueue_pop(MtQueue_t queue)
{
    MtQueueData_t* data = (MtQueueData_t*) queue.q;

    void* item = 0;

    pthread_mutex_lock(&data->mutex);

    while(data->count == 0) {
        pthread_cond_wait(&data->readCondition, &data->mutex);
    }

    item = data->queue[data->front];
    ++(data->front);
    data->front %= data->capacity;
    --(data->count);

    pthread_cond_signal(&data->writeCondition);
    pthread_mutex_unlock(&data->mutex);

    return item;
}

void MtQueueThreadCleanup(MtQueue_t queue)
{
    MtQueueData_t* data = (MtQueueData_t*) queue.q;
    pthread_mutex_unlock(&data->mutex);
}
