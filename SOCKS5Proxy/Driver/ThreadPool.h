#ifndef __THREADPOOL_H
#define __THREADPOOL_H

/*********************************************
* Manage threads that inherit the threading class. Can shut down threads if requested
*********************************************/

#ifdef USE_NONBLOCKING_SOCKETS
#include "ThreadStarter.h"
#include <set>
#include <Windows.h>

class ThreadController
{
public:
	HANDLE hThread;
	unsigned int thread_id;

	void Setup(HANDLE h)
	{
		hThread = h;
	}

	void Suspend()
	{
		SuspendThread(hThread);
	}

	void Resume()
	{
		if(!ResumeThread(hThread))
		{
			DWORD le = GetLastError();
		}
	}

	void Join()
	{
		WaitForSingleObject(hThread, INFINITE);
	}

	unsigned int GetId() { return thread_id; }
};

struct Thread
{
	Thread()
	{
		InitializeCriticalSection(&SetupMutex);
	}
	~Thread()
	{
		DeleteCriticalSection(&SetupMutex);
	}
	ThreadBase * ExecutionTarget;
	ThreadController ControlInterface;
	CRITICAL_SECTION SetupMutex;
	bool DeleteAfterExit;
};

typedef std::set<Thread*> ThreadSet;

class CThreadPool
{
	int GetNumCpus();

	unsigned int _threadsRequestedSinceLastCheck;
	unsigned int _threadsFreedSinceLastCheck;
	unsigned int _threadsExitedSinceLastCheck;
	unsigned int _threadsToExit;
	int _threadsEaten;
	CRITICAL_SECTION _mutex;

    ThreadSet m_activeThreads;
	ThreadSet m_freeThreads;

public:
	CThreadPool();
	~CThreadPool();

	// call every 2 minutes or so.
	void IntegrityCheck();

	// call at startup
	void Startup();

	// shutdown all threads
	void Shutdown();
	
	// return true - suspend ourselves, and wait for a future task.
	// return false - exit, we're shutting down or no longer needed.
	bool ThreadExit(Thread * t);

	// creates a thread, returns a handle to it.
	Thread * StartThread(ThreadBase * ExecutionTarget);

	// grabs/spawns a thread, and tells it to execute a task.
	void ExecuteTask(ThreadBase * ExecutionTarget);

	// prints some neat debug stats
	void ShowStats();

	// kills x free threads
	void KillFreeThreads(unsigned int count);

	// resets the gobble counter
	void Gobble() { _threadsEaten=(int)m_freeThreads.size(); }

	// gets active thread count
	unsigned int GetActiveThreadCount() { return (unsigned int)m_activeThreads.size(); }

	// gets free thread count
	unsigned int GetFreeThreadCount() { return (unsigned int)m_freeThreads.size(); }

	//check if any of the threads got deadlocked
	unsigned int GetLastThreadUpdateStamp();
};

extern CThreadPool ThreadPool;

#endif
#endif