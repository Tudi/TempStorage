#include <config_reader.h>
#include <app_errors.h>
#include <logger.h>
#include <utils.h>
#include <strings_ext.h>
#include <profile_client.h>
#include <profiling.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

#define MAX_SERVER_NETWORK_RESPONSE_SIZE (30*1024*1024)
#define NS_TO_MS MS_TO_NS

typedef struct WorkerThreadParam
{
	int threadIndex;
    int idNow;
    int messagesSent;
    char* JSONContent;
    pthread_t thread;
}WorkerThreadParam;

// worker thread states and params
WorkerThreadParam *WorkerThreadParams;
int serverIsRunning;

static void writeIdAtLoc(char* str, int id)
{
    while (id > 0)
    {
        *str = '0' + (id % 10);
        id = id / 10;
        str--;
    }
}

static void* workerThreadFunc(void* arg)
{
    WorkerThreadParam* param = (WorkerThreadParam*)arg;
    unsigned char* jsonData = malloc(MAX_SERVER_NETWORK_RESPONSE_SIZE);
    unsigned int jsonDataSize = MAX_SERVER_NETWORK_RESPONSE_SIZE;
    unsigned int numBytesRead;

    int sockt = -1;

    int ret = connectToServer(appConf.serverIP, appConf.serverPort, &sockt);
    if(ret != 0)
    {
        LOG_MESSAGE(INFO_LOG_MSG, "Error: connectToServer(address = %s, port = %d) returned %d.\n",
            appConf.serverIP, appConf.serverPort, ret);
        return NULL;
    }

    unsigned char responseCode = 0;

    while (serverIsRunning)
    {
        int64_t startStamp = GetProfilingStamp();
        if (appConf.requestType == 7) //SAVE_COMPANY_REQ   = 7
        {
            writeIdAtLoc(param->JSONContent + appConf.idStartPos + SPACES_FOR_ID_VAL - 1, param->idNow);
            ret = sendCompanyToServer(sockt, param->JSONContent, appConf.JSONSize, &responseCode);
        }
        else if (appConf.requestType == 2) //SAVE_PROFILE_REQ   = 2
        {
            writeIdAtLoc(param->JSONContent + appConf.idStartPos + SPACES_FOR_ID_VAL - 1, param->idNow);
            ret = sendProfileToServer(sockt, param->JSONContent, appConf.JSONSize, &responseCode);
        }
        else if (appConf.requestType == 1) //GET_PROFILE_REQ   = 1
        {
            numBytesRead = 0;
            ret = getProfileFromServer(sockt, param->idNow, &responseCode,
                (char*)jsonData, jsonDataSize, &numBytesRead);
        }
        else if (appConf.requestType == 6) //GET_COMPANY_REQ   = 6
        {
            numBytesRead = 0;
            ret = getCompanyFromServer(sockt, param->idNow, &responseCode,
                (char*)jsonData, jsonDataSize, &numBytesRead);
        }
        else
        {
            // least demanding request to test number of connections possible
            uint8_t completionPercentage = 0;
            ret = getSearchStatusFromServer(sockt, 0, &responseCode, &completionPercentage);
            if (ret != 0)
            {
                param->messagesSent--; // cause we will increment it later
            }
            ret = 0;
            responseCode = 0; // we only care that we made the call and not what it returns
        }

        int64_t endStamp = GetProfilingStamp();
        if ((ret != 0) || (responseCode != 0))
        {
            LOG_MESSAGE(INFO_LOG_MSG, "Error: API call returned %d - server response = %u.\n",
                ret, responseCode);
        }
        else
        {
            param->idNow += appConf.entryIdIncrease;
            param->messagesSent++;
        }
        int64_t timeDiff = (endStamp - startStamp) / NS_TO_MS;
        if (timeDiff > 500)
        {
            LOG_MESSAGE(INFO_LOG_MSG, "Warning: requets took %lld milliseconds.\n", timeDiff);
        }
    }

    ret = endConnectionToServer(sockt, &responseCode);
    if ((ret != 0) || (responseCode != 0))
    {
        LOG_MESSAGE(INFO_LOG_MSG, "Error: endConnectionToServer() returned %d - server response = %u.\n",
            ret, responseCode);
    }

    free(param->JSONContent);
    free(jsonData);

    printf("Thread %d exited\n", param->threadIndex);
    return NULL;
}

int createWorkerThread(int index)
{
    WorkerThreadParam* param = &WorkerThreadParams[index];
    param->threadIndex = index;
    param->JSONContent = strdup(appConf.JSONContent);
    param->idNow = appConf.entryIdStart + (index - 1) * appConf.entryIdIncreaseThread;// first index is 1
    int createRet = pthread_create(&param->thread, NULL, workerThreadFunc, param);
    if (createRet != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: pthread_create(thread = %d) "
            "returned %d. errno = %d (\"%s\").", index, createRet, errno, strerror(errno));
        return ERR_FAILED_TO_CREATE_THREAD;
    }
    return 0;
}

int startWorkerThreads()
{
    // thread 0 is the logger thread
    for (size_t i = 1; i <= appConf.workerThreadCount; i++)
    {
        createWorkerThread(i);
    }

    // wait for worker threads to finish
    int ret = 0;

    for (size_t i = 1; i <= appConf.workerThreadCount; ++i)
    {
        pthread_join(WorkerThreadParams[i].thread, NULL);
        // if a worker thread exited for some reason, we can kill logger thread also
        serverIsRunning = 0;
    }
    pthread_join(WorkerThreadParams[0].thread, NULL);

    // no more threads mean we no longer need the sate variables
    free(WorkerThreadParams);
    WorkerThreadParams = NULL;

    return ret;
}

#define gotoxy(x,y) printf("\033[%d;%dH", (y), (x))
static void* loggerThreadFunc(void* arg)
{
    int64_t StartStamp = GetProfilingStamp();
    int linesWritten = 0;
    while (serverIsRunning)
    {
        // wait one second for some data to be available
        sleep(1);
        // try to overwrite te same text block
        if (linesWritten > 0)
        {
            printf("\033[%dA", linesWritten);
        }
        // count the number of lines we have written so that we can overwrite them next time
        linesWritten = 0;
        // how many seconds have passed since we started profiling. So we can calculate requests / second
        int64_t NowStamp = GetProfilingStamp();
        int64_t SecondsSinceRunning = (NowStamp - StartStamp)/ NS_TO_MS / 1000;
        int totalMessagesSent = 0;
        for (size_t i = 1; i < appConf.workerThreadCount; i++)
        {
            if (WorkerThreadParams[i].threadIndex == 0)
            {
                continue;
            }
            totalMessagesSent += WorkerThreadParams[i].messagesSent;
            printf("Thread %zu uses id %d, messages sent %d, bytes sent %zu\n", i, 
                WorkerThreadParams[i].idNow, WorkerThreadParams[i].messagesSent, 
                WorkerThreadParams[i].messagesSent * appConf.JSONSize);
            linesWritten++;
        }
        int totalBytesSent = totalMessagesSent * appConf.JSONSize;
        printf("Inserts / second = %.02f | ", (float)totalMessagesSent / (float)SecondsSinceRunning);
        float bytesPerSecond = (float)totalBytesSent / (float)SecondsSinceRunning;
        if (bytesPerSecond < 1024)
        {
            printf("Bytes / second = %.02f\n", bytesPerSecond);
        }
        else if (bytesPerSecond < 1024 * 1024)
        {
            printf("KBytes / second = %.02f\n", bytesPerSecond / 1024.0f);
        }
        else if (bytesPerSecond < 1024 * 1024 * 1024)
        {
            printf("MBytes / second = %.02f\n", bytesPerSecond / 1024.0f / 1024.0f);
        }
        linesWritten++;
    }
    return NULL;
}

int prepareThreadParams()
{
    size_t bytesRequired = sizeof(WorkerThreadParam) * (appConf.workerThreadCount + 1);
    WorkerThreadParams = malloc(bytesRequired);
    if (WorkerThreadParams == NULL)
    {
        return 1;
    }
    memset(WorkerThreadParams, 0, bytesRequired);
    serverIsRunning = 1;
    return 0;
}

int startLoggerThread()
{
    int createRet = pthread_create(&WorkerThreadParams[0].thread, NULL, loggerThreadFunc, NULL);
    if (createRet != 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: pthread_create(thread = %hu) "
            "returned %d. errno = %d (\"%s\").", 0, createRet, errno, strerror(errno));
        return ERR_FAILED_TO_CREATE_THREAD;
    }

    return 0;
}

void stopWorkerThreads()
{
    serverIsRunning = 0;
}
