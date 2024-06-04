#include <mutex>
#include <vector>
#include "curl/include/curl/curl.h"
#include "Session/ApplicationSession.h"
#include "CurlInterface.h"
#include "Util/InitFileHandler.h"
#include "ResourceManager/LogManager.h"
#include "ResourceManager/ConfigManager.h"
#include "Util/Allocator.h"

#ifdef _DEBUG
    #pragma comment( lib, "../dependencies/curl/lib/libcurl-d.lib" )
#else
    #pragma comment( lib, "../dependencies/curl/lib/libcurl.lib" )
#endif

typedef struct CURLJobQueueElement
{
    char* url;
    char* postData;
    CURL_AsyncCallback cb;
    void* userData;
    bool isDuplicate; // a newer event entered the queue. Do not spam server with similar requests
    unsigned __int64 queueStamp;
}CURLJobQueueElement;

struct CURL_WorkerThreadParams
{
    std::mutex CURLJobQueueMutex;
    std::vector<CURLJobQueueElement*> CURLJobQueue;
    HANDLE CURL_WakeupEvent = NULL;
};

#define DISABLE_ASYNC_THREADS // right now it's not used. Might readd later
#define CURL_WORKER_THREADS 5 // alerts / Module data / SMS notification / EMAIL notification
CURL_WorkerThreadParams workerThreadParams[CURL_WORKER_THREADS];

void CURL_AsyncExecuteThread(WatchdogThreadData* wtd, CURL_WorkerThreadParams *wtp)
{
    AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityNormal, LogSourceGroups::LogSourceCURL, 0, 0,
        "CURL Async thread started");

    while (sAppSession.IsApplicationRunning() == true && wtd->ShouldShutDown() == false)
    {
        // let watchdog know this thread is functioning as expected
        wtd->SignalHeartbeat();

        std::unique_lock<std::mutex> lock(wtp->CURLJobQueueMutex);
        // Check if the queue is empty
        if (wtp->CURLJobQueue.empty()) {
            // Release the mutex and wait for a job to be added
            lock.unlock();
            wtd->BlockThreadUntil(CURL_ASYNC_LOOP_SLEEP_MS);
        }
        else {
            // Pop a job from the queue
            CURLJobQueueElement* job = wtp->CURLJobQueue.front();
            wtp->CURLJobQueue.erase(wtp->CURLJobQueue.begin());

            // Release the mutex so other threads may push new work
            lock.unlock();

            if (job->isDuplicate)
            {
                AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceCURL, 0, 0,
                    "CURL:Async thread skipped calling %s", job->url);
            }
            else
            {
                // Execute the job
                CURLcode resCode;
                char* response = GetURLResponse(job->url, job->postData, &resCode);

                AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceCURL, 0, 0,
                    "CURL:Async thread finished calling %s - %s. Response was '%s'. Api lag %zu", job->url, 
                    job->postData, response, GetTickCount64() - job->queueStamp);

                // Execute the callback with the data
                if (job->cb != NULL)
                {
                    job->cb(resCode, response, job->userData);
                }

                AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceCURL, 0, 0,
                    "CURL:Async thread finished handling %s response. Total lag %zu", job->url, GetTickCount64() - job->queueStamp);

                // destroy the response. Callback should have duplicated it if needed
                InternalFree(response);
            }

            // Free up queue resources
            InternalFree(job->url);
            InternalFree(job->postData);
            InternalFree(job);
        }
    }

    while (!wtp->CURLJobQueue.empty()) {
        // Pop a job from the queue
        CURLJobQueueElement* job = wtp->CURLJobQueue.front();
        wtp->CURLJobQueue.erase(wtp->CURLJobQueue.begin());
        InternalFree(job->url);
        InternalFree(job->postData);
        InternalFree(job);
    }

    // Let watchdog know we exited
    wtd->MarkDead();
}

void CURL_Init()
{
    AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityNormal, LogSourceGroups::LogSourceCURL, 0, 0,
        "CURL:Initializing CURL");

    CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (res != CURLE_OK) {
        // Handle initialization error
        AddLogEntry(LogDestinationFlags::LDF_ALL, LogSeverityValue::LogSeveritySever, LogSourceGroups::LogSourceCURL, 0, 0,
            "CURL:Failed to initialize CURL");
        return;
    }

    // create watchdog data
#ifndef DISABLE_ASYNC_THREADS
    for (size_t i = 0; i < CURL_WORKER_THREADS; i++)
    {
        CURL_WorkerThreadParams* wtp = &workerThreadParams[i];
        WatchdogThreadData* wtd;
        InternalNew(wtd, WatchdogThreadData);
        if (wtd == NULL)
        {
            return;
        }

        // start worker thread to push log messages to server
        std::thread* myThread = new std::thread([wtd, wtp]()
            {
                CURL_AsyncExecuteThread(wtd, wtp);
            });
        char wtName[512];
        sprintf_s(wtName, "APIAsync %zd", i);
        wtd->Init(myThread, CURL_ASYNC_LOOP_SLEEP_MS, wtName);

        // Make the application wait until this thread also exits
        sAppSession.AddModuleThread(wtd);

        // get the handle to the wakup signal
        wtp->CURL_WakeupEvent = wtd->GetEventHandle();
    }
#endif
}

void CURL_Shutdown()
{
    AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityNormal, LogSourceGroups::LogSourceCURL, 0, 0,
        "CURL:CURL shutting down");

    curl_global_cleanup();
}

// Callback function to write the received data to a string
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) 
{
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

char* GetURLResponse(const char* url, const char* postData, CURLcode* out_res)
{
    CURL* curl;
    CURLcode res;
    std::string response;

    curl = curl_easy_init();
    if (curl) 
    {
        // Set the URL to request
        curl_easy_setopt(curl, CURLOPT_URL, url);

        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);

        if (postData != NULL)
        {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData);
        }

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // if the URL is using https, check if config specifies to ignore SSL certificate issuer check
        if (url[4] == 's' && sConfigManager.GetInt(ConfigOptionIds::API_IgnoreCertificateIssuer) == 1)
        {
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        }

        // Perform the request
        res = curl_easy_perform(curl);

        if (out_res != NULL)
        {
            *out_res = res;
        }

        // Check for errors
        if (res != CURLE_OK) 
        {
            return NULL;
        }

        // Cleanup
        curl_easy_cleanup(curl);
    }

    return InternalStrDup(response.c_str());
}

void GetURLResponseAsync(const char* url, const char* postData, CURL_AsyncCallback cb, void* userData, int flags)
{
    CURLJobQueueElement *job = (CURLJobQueueElement*)InternalMalloc(sizeof(CURLJobQueueElement));
    if (job == NULL)
    {
        return;
    }

    AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceCURL, 0, 0,
        "CURL:CURL add async call to %s", url);

    job->isDuplicate = 0;
    job->url = InternalStrDup(url);
    job->postData = InternalStrDup(postData);
    job->cb = cb;
    job->userData = userData;
    job->queueStamp = GetTickCount64();

    size_t bestIndex = 0;
    size_t bestCount = INT_MAX;

    // some rare cases async calls should execute in serial way
    if ((flags & CURL_ASYNC_Flags::CAF_FORCE_THREAD0) == 0)
    {
        for (size_t i = 0; i < CURL_WORKER_THREADS; i++)
        {
            size_t curSize = workerThreadParams[i].CURLJobQueue.size();
            if (curSize < bestCount)
            {
                bestCount = curSize;
                bestIndex = i;
            }
        }
    }

    // do not spam the same request multiple times. Maybe we are already killing the backend
    for (size_t i = 0; i < CURL_WORKER_THREADS; i++)
    {
        if (!workerThreadParams[i].CURLJobQueue.empty())
        {
            // lock the queue to avoid reading oudated info
            std::lock_guard<std::mutex> lock(workerThreadParams[i].CURLJobQueueMutex);

            // consider old jobs with the same callback as outdated
            for (CURLJobQueueElement* jobExisting : workerThreadParams[i].CURLJobQueue)
            {
                if (job->cb == jobExisting->cb &&   // same callback
                    strcmp(job->url, jobExisting->url) == 0 && // same url
                    strcmp(job->postData, jobExisting->postData) == 0) // same post data
                {
                    jobExisting->isDuplicate = 1;
                }
            }
        }
    }

    // push the job to the queue
    std::unique_lock<std::mutex> lock(workerThreadParams[bestIndex].CURLJobQueueMutex);
    workerThreadParams[bestIndex].CURLJobQueue.push_back(job);
    lock.unlock();

    // tell worker thread to stop sleeping right now
    SetEvent(workerThreadParams[bestIndex].CURL_WakeupEvent);
}
