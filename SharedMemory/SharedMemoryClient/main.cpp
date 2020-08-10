#include <stdio.h>
#include "../SharedMemoryHost/mio.hpp"
#include "../SharedMemoryHost/SharedStructs.h"

void WaitFileExists(const std::string& path)
{
    while (1)
    {
        FILE* f;
        errno_t er = fopen_s(&f, path.c_str(), "rb");
        if (f == NULL)
        {
            break;
            Sleep(1);
        }
        else
        {
            fclose(f);
            break;
        }
    }
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
    WaitFileExists(path);
    unsigned char* tBuff = (unsigned char* )malloc(BUFSIZE);
    std::error_code error;
    mio::mmap_sink rw_mmap = mio::make_mmap_sink(path, 0, mio::map_entire_file, error);
    if (error)
        return handle_error(error);
    SharedHeader* SharedBuff = (SharedHeader*)rw_mmap.begin();
    printf("Created shared memory\n");
    printf("Waiting for host to come online to start speed profiling\n");
    while (SharedBuff->BufferStatus != StatusCode_WaitClientFinish)
        Sleep(0);
    SharedBuff->BufferStatus = StatusCode_WaitServerFinish;
    //fill the buffer with values
    for (int i = 0; i < RepeatSendRecvCount; i++)
    {
#ifdef _DEBUG
        printf("waiting for host to finish writing it\n");
#endif
        while (SharedBuff->BufferStatus != StatusCode_WaitClientFinish)
            Sleep(0);
#ifdef _DEBUG
        printf("Reading data from host\n");
#endif
        memcpy(tBuff, SharedBuff->Data, sizeof(SharedBuff->Data));
        //sanity check, see if host wrote what we expected it to write
        unsigned char RecvData = (i + 0) & 255;
        if (tBuff[0] != RecvData)
            printf("Was expecting host to send %d and we got %d\n", RecvData, tBuff[0]);
#ifdef _DEBUG
        printf("Sending buffer index %d\n", i+1);
#endif
        //set data we want to send to the host
        unsigned char SendData = (i+1) & 255;
        memset(SharedBuff->Data, SendData, sizeof(SharedBuff->Data));
        //wait for the host to read it and set status
        SharedBuff->BufferStatus = StatusCode_WaitServerFinish;
    }

    //std::fill(rw_mmap.begin(), rw_mmap.end(), 'a');
    rw_mmap.sync(error);
    if (error)
        return handle_error(error);
    rw_mmap.unmap();
    free(tBuff);
    return 0;
}