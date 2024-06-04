#pragma once

#include <vector>
#include <mutex>
#include "Util/VariousFuncs.h"

// execute functions asynchroniously
// tasks will get executed based on priorities. Always lowest
// it has a pool of threads. Will launch new threads if too much work needs to be done
// ! you can't queue tasks if the application is shutting down

#define ATM_TASK_WORKERTHREAD_SLEEP	1000
#define ATM_INVALID_LARGE_THREAD_COUNT	200
#ifdef _DEBUG
	#define GEN_TASK_TRACK_INFO
#endif

// worker threads will fetch tasks until they empty low priority queues
// once a queue is empty, they will fetch from next queue
enum AsyncTaskPriorityRanks : unsigned __int64
{
	Priority_1, 
	Priority_2,
	Priority_3,
	Priority_Max
};

typedef void (*AsyncTask)(void *userParam);

class WatchdogThreadData;

// used by the server to periodically update sessions
// Has N worker threads that will try to split the work
class AsyncTaskManager
{
public:
	inline static AsyncTaskManager& getInstance()
	{
		// This is a thread-safe way to create the instance
#ifdef _DEBUG
		static AsyncTaskManager* instance = new AsyncTaskManager();
		return *instance;
#else
		static AsyncTaskManager instance;
		return instance;
#endif
	}
	~AsyncTaskManager();
	void DestructorCheckMemLeaks();

	// Creates N worker threads that will process incomming packets
	void Init(size_t maxThreads);

#define AddAsyncTask(callBackFunc, userParams) sAsyncTaskManager.AddAsyncTask_(AsyncTaskPriorityRanks::Priority_1, callBackFunc,userParams, __FILE__, __FUNCTION__, false);
#define AddAsyncTask1(priority, callBackFunc, userParams, isUnique) sAsyncTaskManager.AddAsyncTask_(priority, callBackFunc, userParams, __FILE__, __FUNCTION__, isUnique);
	void AddAsyncTask_(AsyncTaskPriorityRanks priority, AsyncTask cb, void *userData, const char *file, const char *func, bool isUnique);
private:
	AsyncTaskManager();
	AsyncTaskManager(const AsyncTaskManager&) = delete;
	AsyncTaskManager& operator=(const AsyncTaskManager&) = delete;

	// the worker function that will process sessions
	static void AsyncTaskManager_AsyncExecuteThread(WatchdogThreadData* wtd);

	typedef struct AsyncTaskQueueElement
	{
		AsyncTask cb;
		void* userData;
#ifdef GEN_TASK_TRACK_INFO
		const char* file;
		const char* func;
		unsigned __int64 queueStamp;
		unsigned __int64 startStamp;
#endif
	}CURLJobQueueElement;

	std::mutex m_QueueLocks[AsyncTaskPriorityRanks::Priority_Max];
	CircularBuffer<AsyncTaskQueueElement> m_Queues[AsyncTaskPriorityRanks::Priority_Max];
};

#define sAsyncTaskManager AsyncTaskManager::getInstance()
