#include <stdlib.h>
#include <string.h>
#include "CircularBuffer.h"

CircularBuffer *CreateCircularBuffer(int Size)
{
	if(Size <= 0)
		return NULL;
	CircularBuffer *ret = (CircularBuffer *)malloc(sizeof(CircularBuffer));
	if(ret == NULL)
		return NULL;
	//safety
	memset(ret,0,sizeof(CircularBuffer));
	ret->ReadIndex = 0;
	ret->WriteIndex = 0;
	ret->MaxIndex = Size;
	ret->BufferArray = (void **)malloc(Size * sizeof(void*));
	if( ret->BufferArray == NULL )
	{
		free(ret);
		return NULL;
	}
	memset(ret->BufferArray,0,Size * sizeof(void*));
	return ret;
}

int CanStoreMore(CircularBuffer *cb)
{
	if( cb == NULL )
		return 0;
	int NextIndex = ( cb->WriteIndex + 1 ) % cb->MaxIndex;
	//queue is full
	if( cb->ReadIndex == NextIndex || cb->BufferArray[NextIndex] != NULL)
		return 0;
	return 1;
}

int AddBuffer(CircularBuffer *cb, void *p, omp_lock_t *lock )
{
	if( cb == NULL )
		return -1;
	int NextIndex = ( cb->WriteIndex + 1 ) % cb->MaxIndex;
	//queue is full
	if( cb->ReadIndex == NextIndex || cb->BufferArray[NextIndex] != NULL )
		return -1;
	//add the buffer
	cb->BufferArray[cb->WriteIndex] = p;
	cb->WriteIndex = NextIndex;
	return 0;
}

int GetBuffer(CircularBuffer *cb, void **p, omp_lock_t *lock )
{
	if (p == NULL)
		return -1;
	//set return before anything else
	*p = NULL;
	if( cb == NULL )
		return -1;
	//queue is empty 
	if( cb->ReadIndex == cb->WriteIndex || cb->BufferArray[cb->ReadIndex] == NULL)
		return -1;
	//get the buffer
	*p = cb->BufferArray[cb->ReadIndex];
	cb->BufferArray[cb->ReadIndex] = NULL;
	cb->ReadIndex = ( cb->ReadIndex + 1 ) % cb->MaxIndex;
	return 0;
}

int PeekBuffer(CircularBuffer* cb, void** p, int index, omp_lock_t* lock)
{
	if (p == NULL)
		return -1;
	//set return before anything else
	*p = NULL;
	if (cb == NULL)
		return -1;
	int SearchedIndex = (cb->ReadIndex + index) % cb->MaxIndex;
	//queue is empty 
	if (cb->BufferArray[SearchedIndex] == NULL)
		return -1;
	//get the buffer
	*p = cb->BufferArray[SearchedIndex];
	return 0;
}