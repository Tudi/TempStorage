#ifdef USE_NONBLOCKING_SOCKETS
#include "ThreadPool.h"
#include <process.h>

#define THREAD_RESERVE 2
CThreadPool ThreadPool;

CThreadPool::CThreadPool()
{
	_threadsExitedSinceLastCheck = 0;
	_threadsRequestedSinceLastCheck = 0;
	_threadsEaten = 0;
	_threadsFreedSinceLastCheck = 0;
	InitializeCriticalSection(&_mutex);
}

CThreadPool::~CThreadPool()
{
	DeleteCriticalSection(&_mutex);
}

bool CThreadPool::ThreadExit(Thread * t)
{
	EnterCriticalSection(&_mutex);
	
	// we're definitely no longer active
	m_activeThreads.erase(t);

	// do we have to kill off some threads?
	if(_threadsToExit > 0)
	{
		// kill us.
		--_threadsToExit;
		++_threadsExitedSinceLastCheck;
		if(t->DeleteAfterExit)
			m_freeThreads.erase(t);

		LeaveCriticalSection(&_mutex);
		delete t;
		t = NULL;
		return false;
	}

	// enter the "suspended" pool
	++_threadsExitedSinceLastCheck;
	++_threadsEaten;
	std::set<Thread*>::iterator itr = m_freeThreads.find(t);

	if(itr != m_freeThreads.end())
	{
//		printf("Thread %u duplicated with thread %u\n", (*itr)->ControlInterface.GetId(), t->ControlInterface.GetId());
	}
	m_freeThreads.insert(t);
	
	//Log.Debug("ThreadPool", "Thread %u entered the free pool.", t->ControlInterface.GetId());
	LeaveCriticalSection(&_mutex);
	return true;
}

void CThreadPool::ExecuteTask(ThreadBase * ExecutionTarget)
{
	Thread * t;
	EnterCriticalSection(&_mutex);
	++_threadsRequestedSinceLastCheck;
	--_threadsEaten;

	// grab one from the pool, if we have any.
	if(m_freeThreads.size())
	{
		t = *m_freeThreads.begin();
		m_freeThreads.erase(m_freeThreads.begin());

		// execute the task on this thread.
		t->ExecutionTarget = ExecutionTarget;

		// resume the thread, and it should start working.
		t->ControlInterface.Resume();
		//Log.Debug("ThreadPool", "Thread %u left the thread pool.", t->ControlInterface.GetId());
	}
	else
	{
		// creating a new thread means it heads straight to its task.
		// no need to resume it :)
		t = StartThread(ExecutionTarget);

		if (t == NULL)
		{
			LeaveCriticalSection(&_mutex);
			return;
		}
	}

	// add the thread to the active set
	//Log.Debug("ThreadPool", "Thread %u is now executing task at 0x%p.", t->ControlInterface.GetId(), ExecutionTarget);
	m_activeThreads.insert(t);
	LeaveCriticalSection(&_mutex);
}

unsigned int CThreadPool::GetLastThreadUpdateStamp()
{
	unsigned int min_stamp = 0xFFFFFFFF;
	EnterCriticalSection(&_mutex);
	for(ThreadSet::iterator itr = m_activeThreads.begin(); itr != m_activeThreads.end(); ++itr)
	{
		if((*itr)->ExecutionTarget)
		{
			unsigned int this_stamp = (*itr)->ExecutionTarget->last_updated;
			if( this_stamp && this_stamp < min_stamp )
			{
				min_stamp = this_stamp;
//printf("Timestamp for thread is %u \n",this_stamp);
			}
		}
	}
	LeaveCriticalSection(&_mutex);
	return min_stamp;
}

void CThreadPool::Startup()
{
	int i;
	int tcount = THREAD_RESERVE;

	for (i = 0; i < tcount; ++i)
	{
		if (StartThread(NULL) == NULL)
			break;
	}
}

void CThreadPool::ShowStats()
{
}

void CThreadPool::IntegrityCheck()
{
	EnterCriticalSection(&_mutex);
	int gobbled = _threadsEaten;

    if(gobbled < 0)
	{
		// this means we requested more threads than we had in the pool last time.
        // spawn "gobbled" + THREAD_RESERVE extra threads.
		unsigned int new_threads = abs(gobbled) + THREAD_RESERVE;
		_threadsEaten=0;

		for(unsigned int i = 0; i < new_threads; ++i)
		{
			if (StartThread(NULL) == NULL)
				break;
		}

//		Log.Debug("ThreadPool", "IntegrityCheck: (gobbled < 0) Spawning %u threads.", new_threads);
	}
	else if(gobbled < THREAD_RESERVE)
	{
        // this means while we didn't run out of threads, we were getting damn low.
		// spawn enough threads to keep the reserve amount up.
		unsigned int new_threads = (THREAD_RESERVE - gobbled);
		for(unsigned int i = 0; i < new_threads; ++i)
		{
			if (StartThread(NULL) == NULL)
				break;
		}

//		Log.Debug("ThreadPool", "IntegrityCheck: (gobbled <= 5) Spawning %u threads.", new_threads);
	}
	else if(gobbled > THREAD_RESERVE)
	{
		// this means we had "excess" threads sitting around doing nothing.
		// lets kill some of them off.
		unsigned int kill_count = (gobbled - THREAD_RESERVE);
		KillFreeThreads(kill_count);
		_threadsEaten -= kill_count;
//		Log.Debug("ThreadPool", "IntegrityCheck: (gobbled > 5) Killing %u threads.", kill_count);
	}
	else
	{
		// perfect! we have the ideal number of free threads.
//		Log.Debug("ThreadPool", "IntegrityCheck: Perfect!");
	}
	/*if(m_freeThreads.size() < 5)
	{
		unsigned int j = 5 - m_freeThreads.size();
		Log.Debug("ThreadPool", "Spawning %u threads.", j);
		for(unsigned int i = 0; i < j; ++i)
			StartThread(NULL);
	}*/

	_threadsExitedSinceLastCheck = 0;
	_threadsRequestedSinceLastCheck = 0;
	_threadsFreedSinceLastCheck = 0;

	LeaveCriticalSection(&_mutex);
}

void CThreadPool::KillFreeThreads(unsigned int count)
{
//	Log.Debug("ThreadPool", "Killing %u excess threads.", count);
	EnterCriticalSection(&_mutex);
	Thread * t;
	ThreadSet::iterator itr;
	unsigned int i;
	for(i = 0, itr = m_freeThreads.begin(); i < count && itr != m_freeThreads.end(); ++i, ++itr)
	{
		t = *itr;
		t->ExecutionTarget = NULL; 
		t->DeleteAfterExit = true;
		++_threadsToExit;
		t->ControlInterface.Resume();
	}
	LeaveCriticalSection(&_mutex);
}

void CThreadPool::Shutdown()
{
	EnterCriticalSection(&_mutex);
	size_t tcount = m_activeThreads.size() + m_freeThreads.size();		// exit all
//	Log.Debug("ThreadPool", "Shutting down %u threads.", tcount);
	KillFreeThreads((unsigned int)m_freeThreads.size());
	_threadsToExit += (unsigned int)m_activeThreads.size();

	for(ThreadSet::iterator itr = m_activeThreads.begin(); itr != m_activeThreads.end(); ++itr)
	{
		if((*itr)->ExecutionTarget)
			(*itr)->ExecutionTarget->OnShutdown();
	}
	LeaveCriticalSection(&_mutex);

	for(;;)
	{
		EnterCriticalSection(&_mutex);
		if(m_activeThreads.empty() == false || m_freeThreads.empty() == false)
		{
//			Log.Debug("ThreadPool", "%u threads remaining...",m_activeThreads.size() + m_freeThreads.size() );
			LeaveCriticalSection(&_mutex);
			Sleep(1000);
			continue;
		}

		break;
	}
}

bool RunThread(ThreadBase * target)
{
	bool res = false;
	res = target->run();
	return res;
}

static unsigned long WINAPI thread_proc(void* param)
{
	Thread * t = (Thread*)param;
	EnterCriticalSection(&t->SetupMutex);
	unsigned int tid = t->ControlInterface.GetId();
	bool ht = (t->ExecutionTarget != NULL);
	LeaveCriticalSection(&t->SetupMutex);
	//Log.Debug("ThreadPool", "Thread %u started.", t->ControlInterface.GetId());

	for(;;)
	{
		if(t->ExecutionTarget != NULL)
		{
			const bool _delete = t->ExecutionTarget->run();
			EnterCriticalSection(&t->SetupMutex);
			if(_delete)
				delete t->ExecutionTarget;
			t->ExecutionTarget = NULL;
			LeaveCriticalSection(&t->SetupMutex);
		}

		if(!ThreadPool.ThreadExit(t))
		{
//			Log.Debug("ThreadPool", "Thread %u exiting.", tid);
			break;
		}
		else
		{
			if(ht)
//				Log.Debug("ThreadPool", "Thread %u waiting for a new task.", tid);
			// enter "suspended" state. when we return, the threadpool will either tell us to fuk off, or to execute a new task.
			t->ControlInterface.Suspend();
			// after resuming, this is where we will end up. start the loop again, check for tasks, then go back to the threadpool.
		}
	}

	// at this point the t pointer has already been freed, so we can just cleanly exit.
	//ExitThread(0);

	// not reached
	return 0;
}

Thread * CThreadPool::StartThread(ThreadBase * ExecutionTarget)
{
	HANDLE h;
	Thread * t = new Thread;
	
	t->DeleteAfterExit = false;
	t->ExecutionTarget = ExecutionTarget;
	//h = (HANDLE)_beginthreadex(NULL, 0, &thread_proc, (void*)t, 0, NULL);
	EnterCriticalSection(&t->SetupMutex);
	h = CreateThread(NULL, 0, &thread_proc, (LPVOID)t, 0, (LPDWORD)&t->ControlInterface.thread_id);
	t->ControlInterface.Setup(h);
	LeaveCriticalSection(&t->SetupMutex);

	return t;
}


#endif