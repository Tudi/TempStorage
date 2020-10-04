#include "mio.hpp"
#include <stdio.h>
#include "SharedStructs.h"
#include "../PipeWriteData/Profile.h"

void allocate_file(const std::string& path, const int size)
{
    FILE* f;
    errno_t er = fopen_s(&f, path.c_str(), "wb");
    if (f == NULL)
        return;
    char t='0';
    fseek(f, size, SEEK_SET);
    size_t written_count = fwrite(&t, 1, 1, f);
    fclose(f);
}

int handle_error(const std::error_code& error)
{
    const auto& errmsg = error.message();
    std::printf("error mapping file: %s, exiting...\n", errmsg.c_str());
    return error.value();
}

int main()
{
    const auto path = "../file.txt";
    unsigned char* tBuff = (unsigned char*)malloc(BUFSIZE);
    allocate_file(path, sizeof(SharedHeader));
    std::error_code error;
	mio::mmap_sink rw_mmap = mio::make_mmap_sink(path, 0, mio::map_entire_file, error);
    if (error)
        return handle_error(error);
    SharedHeader* SharedBuff = (SharedHeader *)(rw_mmap.begin());
    printf("Created shared memory\n");
    //init shared memory
    printf("Waiting for client to come online to start speed profiling\n");
    SharedBuff->BufferStatus = StatusCode_WaitClientFinish;
    while (SharedBuff->BufferStatus == StatusCode_WaitClientFinish)
        Sleep(0);
    //fill the buffer with values
    __int64 Start = GetTick();
    for (int i = 0; i < RepeatSendRecvCount; i++)
    {
#ifdef _DEBUG
        printf("Sending buffer index %d\n", i);
#endif
        //set data we want to send to the client
        unsigned char SendData = i & 255;
        memset(SharedBuff->Data, SendData, sizeof(SharedBuff->Data));
        //wait for the client to read it and set status
#ifdef _DEBUG
        printf("waiting for client to finish reading it\n");
#endif
        SharedBuff->BufferStatus = StatusCode_WaitClientFinish;
        while (SharedBuff->BufferStatus == StatusCode_WaitClientFinish)
            Sleep(0);
        //read what the client sent us
        memcpy(tBuff, SharedBuff->Data, sizeof(SharedBuff->Data));
        //sanity check, see if host wrote what we expected it to write
        unsigned char RecvData = (i+1) & 255;
        if (tBuff[0] != RecvData)
            printf("Was expecting host to send %d and we got %d\n", RecvData, tBuff[0]);
    }
    double Duration = GetCounterDiff(Start);
#ifdef BUILD_TEST_OVERHEAD
    printf("This is a special test where pipe overhead is tested instead bandwidth. Transfering only %d bytes %d times\n", BUFSIZE, RepeatSendRecvCount);
#endif
    printf("Time (ms) the test it took : %f\n", (float)Duration);
    __int64 BytesWritten = __int64(BUFSIZE) * __int64(RepeatSendRecvCount);
    __int64 BytesRead = __int64(BUFSIZE) * __int64(RepeatSendRecvCount);
    printf("Bytes written = %lld + Bytes read = %lld\n", BytesWritten, BytesRead);
    __int64 TransferRate = __int64(((BytesWritten + BytesRead) * __int64(1000)) / Duration);
    printf("Transfer Speed %lld bytes/s = %lld kb/s = %lld mb/s = %lld gb/s\n", TransferRate, TransferRate / 1024, TransferRate / 1024 / 1024, TransferRate / 1024 / 1024 / 1024);

    //std::fill(rw_mmap.begin(), rw_mmap.end(), 'a');
    rw_mmap.sync(error);
    if (error) 
        return handle_error(error); 
    rw_mmap.unmap();
    free(tBuff);
    return 0;
}