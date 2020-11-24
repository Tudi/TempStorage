#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <ctime>
#include "Logger.h"

//in multi threaded environment console writes might get mixed up. we convert it to serial writes
HANDLE lock = NULL;
//singleton resource
Logger sLog;

Logger::Logger()
{
    //by default we only write to console
    outf = NULL;
    LevelFlags = LL_Warning | LL_ERROR | LL_Always;
    OutputFlags = LOF_Console;
}

Logger::~Logger()
{
    if (outf != NULL)
    {
        fclose(outf);
        outf = NULL;
    }
    LevelFlags = 0;
    OutputFlags = 0;
}

//if requested, but not defined, we create a default output file
void Logger::InitOutputFile()
{
    if (outf != NULL)
        return;
    errno_t er = fopen_s(&outf, "logs.txt", "at");
}

//replacement function for printf
void Logger::Log(int Level, const char* File, int Line, const char* msg, ...)
{
    if ((LevelFlags & Level) == 0)
        return;
    if (OutputFlags == 0)
        return;

    va_list args;
    va_start(args, msg);

    char TimeInfo[50];
    if (LevelFlags & LL_TimeInfo)
    {
        time_t* rawtime = new time_t;
        struct tm* timeinfo = new tm;
        time(rawtime);
        errno_t er = localtime_s(timeinfo, rawtime);
        sprintf_s(TimeInfo, sizeof(TimeInfo), "[%d-%d %d:%d:%03d]", timeinfo->tm_mon, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, GetTickCount() % 1000);
        delete rawtime;
        delete timeinfo;
    }

#if !defined(_SERVICE_BUILD)
    if (OutputFlags & LOF_Console)
    {
        if (lock == NULL)
            lock = CreateMutex(NULL, FALSE, NULL);
        if (lock == NULL)
        {
            fprintf(stderr, "error: failed to create mutex (%d)\n", GetLastError());
            exit(EXIT_FAILURE);
        }

        WaitForSingleObject(lock, INFINITE);
        if (LevelFlags & LL_TimeInfo)
            fprintf(stdout, "%s", TimeInfo);
        vfprintf(stdout, msg, args);
        ReleaseMutex(lock);
    }
#endif

    if (OutputFlags & LOF_File)
    {
        if (outf == NULL)
            InitOutputFile();
        if (outf == NULL)
            return;
        if (LevelFlags & LL_TimeInfo)
            fprintf(outf, "%s", TimeInfo);
        vfprintf(outf, msg, args);
        fflush(outf);
    }
    va_end(args);
}

//before enabling log to file, you should set up output file first
void Logger::SetLogFile(const char* Path)
{
    char DirPath[MAX_PATH];

    if (Path[0] == '.' || Path[0] == '\\' || Path[0] == '/')
    {
        int StartIndex = 0;
        if (Path[StartIndex] == '.')StartIndex++;
        if (Path[StartIndex] == '\\')StartIndex++;
        if (Path[StartIndex] == '\\')StartIndex++;
        if (Path[StartIndex] == '/')StartIndex++;
        sprintf_s(DirPath, sizeof(DirPath), "%s", &Path[StartIndex]);
    }
    else
        strcpy_s(DirPath, sizeof(DirPath), Path);

    //does it end with  '\' ? or maybe it's a filename ?
    int len = (int)strlen(DirPath);
    if (DirPath[len - 1] == '\\' || DirPath[len - 1] == '/')
        sprintf_s(DirPath, sizeof(DirPath), "%slogs.txt", DirPath);

    FILE* NewOutf;
    errno_t er = fopen_s(&NewOutf, DirPath, "at");

    //failed to open, try again with fixed value
    if (NewOutf == NULL)
        er = fopen_s(&NewOutf, DirPath, "wt");
    if(NewOutf == NULL)
        er = fopen_s(&NewOutf, "logs.txt", "at");

    //new file is good to be used
    if (NewOutf != NULL)
    {
        if (outf != NULL)
            fclose(outf);
        outf = NewOutf;
    }
}
