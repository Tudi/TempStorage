#ifndef _WORKER_THREADS_H_
#define _WORKER_THREADS_H_

int prepareThreadParams();
// this thread will periodically write to the console statistics about worker threads
int startLoggerThread();
// actual worker threads doing requests to ferrari-c
int startWorkerThreads();
// tell worker threads to stop as soon as possible
void stopWorkerThreads();

#endif