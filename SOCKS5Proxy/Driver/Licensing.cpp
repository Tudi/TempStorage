#include <windows.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Logger.h"
#include "ConfigHandler.h"
#include "Licensing.h"
#include "Utils.h"

#include "curl/curl.h"
#pragma comment(lib, "curl\\libcurl.a")
#pragma comment(lib, "curl\\libcurl.dll.a")

struct MemoryStruct {
    char* memory;
    size_t size;
};

static size_t WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct* mem = (struct MemoryStruct*)userp;

    char* ptr = (char*)realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL) 
    {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;

}
class LicenseStatusStore
{
public:
    LicenseStatusStore()
    {
        LastCheckStamp = 0;
        StartedMonitoring = 0;
        CheckCounter = 0;
        CheckPassCounter = 0;
        ShutDownStamp = time(NULL) + SHUTDOWN_ON_NO_LICENSE_MINUTES * 60;
        EncryptionSeed = GetTickCount(); // can be "whatever" as long as it's not constant
    }
    void Update()
    {
        time_t TimeNow = time(NULL);
        //no need to spam updates unless it's time
        if (TimeNow - LastCheckStamp < LICENSE_RECHECK_PERIOD_MINUTES * 60)
            return;
        LastCheckStamp = TimeNow;

        //too much time passed and we failed to receive a valid license
        if (TimeNow >= ShutDownStamp)
            SetProgramTerminated();

        //query the server with our keyfile

        //load the key file content
        char DirPath[MAX_PATH];
        sprintf_s(DirPath, sizeof(DirPath), "%skey.txt", GetFullPath());
        //if file exists, it will have a size
        int KeySize = GetFileSize2(DirPath);
        if (KeySize <= 0)
            return;
        //open key file
        FILE* KeyFile;
        errno_t fopenerr = fopen_s(&KeyFile, DirPath, "rt");
        if (KeyFile == NULL)
            return;
        //allocate bytes for the key
        char* KeyBytes = (char*)malloc(KeySize + 2);
        fread_s(KeyBytes, KeySize, 1, KeySize, KeyFile);
        KeyBytes[KeySize] = 0;
        fclose(KeyFile);

        int KeyCkeckSuccess = 1;

        CURL* curl;
        CURLcode res;

        curl = curl_easy_init();
        if (curl) 
        {
            //curl_version_info_data* vinfo = curl_version_info(CURLVERSION_NOW);
            struct MemoryStruct chunk;

            chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */
            chunk.size = 0;    /* no data at this point */

#ifndef _DEBUG
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
#endif

            curl_easy_setopt(curl, CURLOPT_URL, LICENSE_SERVER_URL);

            //get the fingerprint to this computer so that the license can not be shared between multiple PCs
            char* ComputerFingerPrint = GetLocalFingerprint();
            char* ComputerFingerPrintEncrypted = EncodeFingerPrint(ComputerFingerPrint, EncryptionSeed);

            char POSTFIELD[8000];
            sprintf_s(POSTFIELD, sizeof(POSTFIELD), "key=%s&FP=%s", KeyBytes, ComputerFingerPrintEncrypted);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, POSTFIELD);

            /* cert is stored PEM coded in file... */
            /* since PEM is default, we needn't set it for PEM */
            //curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");

            /* set the cert for client authentication */
            curl_easy_setopt(curl, CURLOPT_SSLCERT, "curl-ca-bundle.crt");

            /* set the private key (file or ID in engine) */
//            curl_easy_setopt(curl, CURLOPT_SSLKEY, pKeyName);

              /* send all data to this function  */
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

            /* we pass our 'chunk' struct to the callback function */
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);

            res = curl_easy_perform(curl);
            if (res != CURLE_OK)
            {
#ifdef _DEBUG
                const char* ErrorStr = curl_easy_strerror(res);
                fprintf(stderr, "curl_easy_perform() failed: %s\n", ErrorStr);
#endif
                KeyCkeckSuccess = 0;
            }
            curl_easy_cleanup(curl);

            if (chunk.size > 0)
            {
                char* ComputerFingerPrintDecrypted = DecodeFingerPrint(chunk.memory, EncryptionSeed);
                //did the page content say that the key file was ok ?
                if (ComputerFingerPrintDecrypted == NULL || strcmp(ComputerFingerPrintDecrypted, ComputerFingerPrint) != 0)
                    KeyCkeckSuccess = 0;

                free(chunk.memory);
                chunk.memory = NULL;
                free(ComputerFingerPrintDecrypted);
            }

            free(ComputerFingerPrint);
            free(ComputerFingerPrintEncrypted);
            EncryptionSeed *= 0x03050709;
        }
        free(KeyBytes);
        KeyBytes = NULL;

        //we should be able to calculate amount of time we have been running. Can do a double check on shutdown timer
        if (CheckCounter == 0)
            StartedMonitoring = TimeNow;
        CheckCounter++;

        if (KeyCkeckSuccess == 1)
        {
            ShutDownStamp = TimeNow + SHUTDOWN_ON_NO_LICENSE_MINUTES * 60;
            CheckPassCounter++;
        }
    }
    LicenseStatusCodes GetLicenseStatus()
    {
        if (CheckPassCounter == 0 && CheckCounter > 0)
            return LS_CHECKED_INVALID;
        if (CheckPassCounter == 0 && CheckCounter == 0)
        {
            //try to get status now
            Update();
            if (CheckPassCounter > 0)
                return LS_CHECKED_VALID;
            return LS_CHECKED_INVALID;
        }
        if (StartedMonitoring != 0)
        {
            time_t TimePassedSinceChecking = time(NULL) - StartedMonitoring;
            time_t ExpectedCheckCounter = TimePassedSinceChecking / 60 / LICENSE_RECHECK_PERIOD_MINUTES;
            if (ExpectedCheckCounter < CheckCounter - 2)
                return LS_CHECK_MALFUNCTION;
        }
        return LS_CHECKED_VALID;
    }
private:
    time_t LastCheckStamp;
    time_t ShutDownStamp;
    int CheckPassCounter;   //maybe give grace period if there was a time the check passed
    int CheckCounter;
    time_t StartedMonitoring;
    int EncryptionSeed;
};

LicenseStatusStore sTSS;

DWORD PeriodicCheckLicenseThread(LPVOID arg)
{
    AutoMonitorThreadExit AM; //don't delete me
    while (IsWaitingForUserExitProgram())
    {
        sTSS.Update();
        Sleep(500); //if period is too large, it will make the shutdown stall
    }
    return 0;
}

void StartLicenseMonitorThread()
{
    HANDLE thread = CreateThread(NULL, 1, (LPTHREAD_START_ROUTINE)PeriodicCheckLicenseThread, (LPVOID)NULL, 0, NULL);
    if (thread == NULL)
    {
        sLog.Log(LL_ERROR, __FILE__, __LINE__, "failed to create commands listener thread (%d)", GetLastError());
        SetProgramTerminated();
        return;
    }
    CloseHandle(thread);
}

LicenseStatusCodes GetLicenseStatus()
{
    return sTSS.GetLicenseStatus();
}