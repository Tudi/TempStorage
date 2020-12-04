#pragma once
#include <omp.h>
/*
	- Store Buffers
	- if write index catches read index, it should throw away buffers
	- should be thread safe (cross platform)
*/

struct CircularBuffer
{
	int ReadIndex;
	int WriteIndex;
	int MaxIndex;
	void **BufferArray;
	omp_lock_t lock; // read / write lock
};

//create a circular buffer
CircularBuffer *CreateCircularBuffer(int Size);
//add a buffer to the circular buffer. Returns 0 if it was able to store it. Else buffer should be deallocated
int AddBuffer(CircularBuffer *cb, void *p, omp_lock_t *lock );
//get a buffer from the circular buffer. Returns 0 if it was able to fetch a valid buffer
int GetBuffer(CircularBuffer *cb, void **p, omp_lock_t *lock );
// Check if more buffers can be stored
int CanStoreMore(CircularBuffer *cb);
// return a future buffer from the queue if possible. It will not pop the buffer from the queue !
int PeekBuffer(CircularBuffer* cb, void** p, int index, omp_lock_t* lock);