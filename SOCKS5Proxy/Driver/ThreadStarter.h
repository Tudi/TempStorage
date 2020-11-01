#ifndef _THREADING_STARTER_H
#define _THREADING_STARTER_H

/*********************************************
* Base class required to be able to run the class in the threadpool
*********************************************/

#ifdef USE_NONBLOCKING_SOCKETS
class ThreadBase
{
public:
	ThreadBase() { last_updated = 0; }
	virtual ~ThreadBase() {}
	virtual bool run() = 0;
	virtual void OnShutdown() { last_updated = 0; }
	void* THREAD_HANDLE;
	unsigned int	last_updated;		//used to monitor if it got deadlocked
};
#endif
#endif

