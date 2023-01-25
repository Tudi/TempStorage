#include <mt_queue.h>
#include <assert_mt.h>
#include <logger.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

//
// Variables
//

static MtQueue_t queue = MtQueue_NULL;

static uint16_t numEntries = 4;

//
// Prototypes
//

int test_MtQueue_push_pop_twoThreads_succeeds();
void test_MtQueue_cleanup();

void* thread1(void* arg);
void* thread2(void* arg);
void threadCleanupHandler(void* arg);

void dummyFreeInt(void* item);

//
// Tests
//

int main(int argc, char* argv[])
{
    atexit(test_MtQueue_cleanup);

    pthread_detach(pthread_self());

    return test_MtQueue_push_pop_twoThreads_succeeds();
}

int test_MtQueue_push_pop_twoThreads_succeeds()
{
    LOG_MESSAGE(INFO_LOG_MSG, "Test %s started...", __func__);

    queue = mtQueue_init(numEntries, dummyFreeInt);
    ASSERT_MT_INT_EQUAL(mtQueue_isNull(queue), 0);

    pthread_t thread1Id;
    pthread_t thread2Id;

    ASSERT_MT_INT_EQUAL(pthread_create(&thread1Id, NULL, thread1, NULL), 0);
    ASSERT_MT_INT_EQUAL(pthread_create(&thread2Id, NULL, thread2, NULL), 0);

    sleep(3);

    LOG_MESSAGE(INFO_LOG_MSG, programError ? "Test failed." : "Test passed.");

    return programError ? EINVAL : 0;
}

void test_MtQueue_cleanup()
{
    mtQueue_free(queue);
}

void* thread1(void* arg)
{
    pthread_detach(pthread_self());
    pthread_cleanup_push(threadCleanupHandler, &queue);

    int32_t item = (int32_t) (uint64_t) mtQueue_pop(queue);
    ASSERT_MT_INT_EQUAL(1, item);
    item = (int32_t) (uint64_t) mtQueue_pop(queue);
    ASSERT_MT_INT_EQUAL(2, item);
    sleep(1);

    mtQueue_push(queue, (void*) 3);
    mtQueue_push(queue, (void*) 4);
    mtQueue_push(queue, (void*) 5);
    mtQueue_push(queue, (void*) 6);
    mtQueue_push(queue, (void*) 7);

    pthread_cleanup_pop(0);

    return NULL;
}

void* thread2(void* arg)
{
    pthread_detach(pthread_self());
    pthread_cleanup_push(threadCleanupHandler, &queue);

    sleep(1);
    mtQueue_push(queue, (void*) 1);
    mtQueue_push(queue, (void*) 2);
    sleep(1);

    int32_t item = (int32_t) (uint64_t) mtQueue_pop(queue);
    ASSERT_MT_INT_EQUAL(3, item);
    item = (int32_t) (uint64_t) mtQueue_pop(queue);
    ASSERT_MT_INT_EQUAL(4, item);
    item = (int32_t) (uint64_t) mtQueue_pop(queue);
    ASSERT_MT_INT_EQUAL(5, item);
    item = (int32_t) (uint64_t) mtQueue_pop(queue);
    ASSERT_MT_INT_EQUAL(6, item);
    item = (int32_t) (uint64_t) mtQueue_pop(queue);
    ASSERT_MT_INT_EQUAL(7, item);

    pthread_cleanup_pop(0);

    return NULL;
}

void threadCleanupHandler(void* arg)
{
    MtQueue_t* q = (MtQueue_t*) arg;
    MtQueueThreadCleanup(*q);
}

void dummyFreeInt(void* item) { }
