#include <stdio.h>
#include <thread>
#include <conio.h>
#include <windows.h>

static int AppIsAlive = 1;

void IncreaseValue(int* val)
{
    *val = *val + 1;
}

void threadFunction()
{
    int threadCounter = 0;
    while (AppIsAlive == 1)
    {
        printf("counter = %d\n", threadCounter);
        IncreaseValue(&threadCounter);
        Sleep(1000);
    }
}

int main()
{
    printf("Print numbers until key is pressed\n");
    // create a background thread
    std::thread myThread(threadFunction);
    _getch();
    // shut down worker thread
    AppIsAlive = 0;
    // wait for worker thread to exit
    myThread.join();

    return 0;
}