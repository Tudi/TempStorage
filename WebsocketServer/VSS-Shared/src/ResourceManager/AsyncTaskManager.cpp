#include <thread>
#include <mutex>
#include <Windows.h>
#include "Session/ApplicationSession.h"
#include "Util/Allocator.h"
#include "LogManager.h"
#include "ResourceManager/AsyncTaskManager.h"

AsyncTaskManager::AsyncTaskManager()
{
	for (size_t priority = 0; priority < AsyncTaskPriorityRanks::Priority_Max; priority++)
	{
		m_Queues[priority].reserve(10); // random number :(
	}
}

AsyncTaskManager::~AsyncTaskManager()
{
	DestructorCheckMemLeaks();
}

void AsyncTaskManager::DestructorCheckMemLeaks()
{
}

void AsyncTaskManager::AsyncTaskManager_AsyncExecuteThread(WatchdogThreadData* wtd)
{
	while (sAppSession.IsApplicationRunning() &&
		wtd->ShouldShutDown() == false)
	{
		// let watchdog know this thread is functioning as expected
		wtd->SignalHeartbeat();

		{
			for (size_t priority = 0; priority < AsyncTaskPriorityRanks::Priority_Max; priority++)
			{
				while (sAsyncTaskManager.m_Queues[priority].empty() == false)
				{
					std::unique_lock<std::mutex> lock(sAsyncTaskManager.m_QueueLocks[priority], std::defer_lock);
					lock.lock();
					// same check, but this time threadsafe
					if (sAsyncTaskManager.m_Queues[priority].empty())
					{
						lock.unlock();
						continue;
					}
					// copy data. No reference as data might get pushed
					AsyncTaskQueueElement te = sAsyncTaskManager.m_Queues[priority].pop_ref();
					lock.unlock();
#ifdef GEN_TASK_TRACK_INFO
					te.startStamp = GetTickCount64();
					AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceAsyncTaskManager, 0, 0,
						"AsyncTaskManager:Started executing %s. Size %llu. Waited in queue %d", te.func, sAsyncTaskManager.m_Queues[priority].size(), GetTickCount64() - te.queueStamp);
#endif

					// execute the task
					te.cb(te.userData);

#ifdef GEN_TASK_TRACK_INFO
					AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceAsyncTaskManager, 0, 0,
						"AsyncTaskManager:Done executing %s. Time to execute %d", te.func, GetTickCount64() - te.startStamp);
#endif
				}
			}
		}

		wtd->BlockThreadUntilNextCycle();
	}

	// Let watchdog know we exited
	wtd->MarkDead();
}

void AsyncTaskManager::Init(size_t maxThreads)
{
	if (maxThreads > ATM_INVALID_LARGE_THREAD_COUNT)
	{
		maxThreads = (int)ATM_INVALID_LARGE_THREAD_COUNT;
	}

	// create worker thread
	sAppSession.CreateWorkerThreadGroup(WTG_AsyncTasks, (int)maxThreads, "AsyncTasks", AsyncTaskManager_AsyncExecuteThread, ATM_TASK_WORKERTHREAD_SLEEP);
}

void AsyncTaskManager::AddAsyncTask_(AsyncTaskPriorityRanks priority, AsyncTask cb, void* userData, const char* file, const char* func, bool isUnique)
{
	file; func;
#ifdef _DEBUG
	if (priority >= AsyncTaskPriorityRanks::Priority_Max)
	{
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeveritySever, LogSourceGroups::LogSourceAsyncTaskManager, 0, 0,
			"AsyncTaskManager:priority out of bounds");
		return;
	}
#endif
	// no more async tasks as we are shutting down
	if (sAppSession.IsApplicationRunning() == false)
	{
		return;
	}
	std::unique_lock<std::mutex> lock(m_QueueLocks[priority], std::defer_lock);
	lock.lock();

	// some tasks might get spam added, but we only need 1 to be executed
	if (isUnique)
	{
		for (auto itr = m_Queues[priority].begin(); itr != m_Queues[priority].end(); itr++)
		{
			if (itr->cb == cb && itr->userData == userData)
			{
				AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceAsyncTaskManager, 0, 0,
					"AsyncTaskManager:Unique task already exists. Exiting");
				lock.unlock();
				return;
			}
		}
	}

	// make sure we can push new tasks into our circular buffer
	if (m_Queues[priority].isFull())
	{
		m_Queues[priority].reserve(m_Queues[priority].size() + 10);
		AddLogEntry(LogDestinationFlags::LDF_LOCAL, LogSeverityValue::LogSeverityDebug, LogSourceGroups::LogSourceAsyncTaskManager, 0, 0,
			"AsyncTaskManager:Task queue was full. Extending it to %lld elements", m_Queues[priority].size());
	}

	// copy data. No reference as data might get pushed
	AsyncTaskQueueElement &te = m_Queues[priority].push();
	te.cb = cb;
	te.userData = userData;
#ifdef GEN_TASK_TRACK_INFO
	te.file = file;
	te.func = func;
	te.queueStamp = GetTickCount64();
#endif
	lock.unlock();

	// wake up a worker thread to handle this task
	sAppSession.WakeUpWorkerThread(WTG_AsyncTasks);
}