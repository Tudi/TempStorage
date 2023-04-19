#include <stdio.h>
#include <thread>
#include <conio.h>
#include <windows.h>

static int AppIsAlive = 1;

void IncreaseValue(int* val)
{
    *val = *val + 1;
}

// whatever, just to make the function body large enough so we can do something else
void DoMultipleStuffSoFunctionBodyGetsLarge(int* val)
{
    *val = *val + 3;
    *val = *val * 2;
    *val = *val - 1;
    *val = *val + 3;
    *val = *val / 2;
    *val = *val - 2;
    *val = *val * 5;
    *val = *val + 3;
    *val = *val * 2;
    *val = *val - 1;
    *val = *val + 3;
    *val = *val / 2;
    *val = *val - 2;
    *val = *val * 5;
}

void threadFunction()
{
    int threadCounter = 0;
    int dummyVal = 100;
    while (AppIsAlive == 1)
    {
        printf("counter = %d. Dummy = %d\n", threadCounter, dummyVal);
        IncreaseValue(&threadCounter);
        DoMultipleStuffSoFunctionBodyGetsLarge(&dummyVal);
        Sleep(1000);
    }
}

int main()
{
    void* dummyAlloc = malloc(32);
    snprintf((char*)dummyAlloc, 32, "You got me");

    printf("Print numbers until key is pressed\n");
    // create a background thread
    std::thread myThread(threadFunction);
    _getch();
    // shut down worker thread
    AppIsAlive = 0;
    // wait for worker thread to exit
    myThread.join();
    
    free(dummyAlloc);

    return 0;
}