#pragma once

//#define BUILD_TEST_OVERHEAD

#ifdef _DEBUG
    #define RepeatSendRecvCount     100
    #define BUFSIZE                 1*4*1024*1024 
#else
    #ifdef BUILD_TEST_OVERHEAD
        #define RepeatSendRecvCount     1000
        #define BUFSIZE                 2
    #else
        #define RepeatSendRecvCount     1000
        #define BUFSIZE                 1*4*1024*1024 
    #endif
#endif

enum StatusCodes
{
    StatusCodeNotInitialized = 0,
    StatusCode_WaitClientFinish = 3,
    StatusCode_WaitServerFinish = 4,
};

struct SharedHeader
{
    //will rewrite this with a better semaphore soon
    int BufferStatus;
    unsigned char Data[BUFSIZE];
};