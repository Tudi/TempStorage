#include <windows.h>
#include <stdio.h>
#include "../PipeWriteData/Profile.h"

#define BUFSIZE 1*4*1024*1024 

int main(void)
{
//    CHAR chBuf[BUFSIZE];
    DWORD dwRead, dwWritten;
    HANDLE hStdin, hStdout;
    BOOL bSuccess;
    unsigned char* Buff = (unsigned char* )malloc(BUFSIZE+16);

    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (
        (hStdout == INVALID_HANDLE_VALUE) ||
        (hStdin == INVALID_HANDLE_VALUE)
        )
        ExitProcess(1);

    // Send something to this process's stdout using printf.
    printf("\n ** This is a message from the child process. ** \n");

    // This simple algorithm uses the existence of the pipes to control execution.
    // It relies on the pipe buffers to ensure that no data is lost.
    // Larger applications would use more advanced process control.

    __int64 Start = GetTick();
    for (;;)
    {
        // Read from standard input and stop on error or no data.
        bSuccess = ReadFile(hStdin, Buff, BUFSIZE, &dwRead, NULL);

        if (!bSuccess || dwRead == 0)
            break;

        // Write to standard output and stop on error.
        bSuccess = WriteFile(hStdout, Buff, dwRead, &dwWritten, NULL);

        if (!bSuccess)
            break;
    }
    double Duration = GetCounterDiff(Start);
    printf("Time (ms) the test it took : %f", (float)Duration);

    free(Buff);
    return 0;
}