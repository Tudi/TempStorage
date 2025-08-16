#include <stdint.h>

/*
* Most C programs fail due to allocations/memory corruption/debugging
* As first step be prepared to handle the future
* Buffer pools are esential for high performance applications to :
*	- avoid memory fragmentation
*	- increase memory locality
*	- more predictible memory usage patterns and statistics
*	- fast memory allocations
* Todo :
*	- better debugging options : bounds checking
*	- add profiling stats for special builds
*	- add option to check for memory leaks ( not expecting any at the moment )
*/

typedef struct BufferPool BufferPool;
typedef struct PoolManager PoolManager;

// all allocated block will be rounded up so that next allocation will alway happen at an alligned value
#define USE_ALLIGNMENT_TO 16

// only while debugging
#define USE_BOUNDS_CHECKING

// init settings
void init_pool_manager();
void destroy_pool_manager();

// fast, non fragmenting memory allocation where you do not need to deallocate : netflows
void *alloc_pooled(BufferPool *pPool, int64_t size);

// this is only used for debugging
#ifdef USE_BOUNDS_CHECKING
	int check_bounds_pooled(BufferPool* pPool, void* memblock);
#else
	// presume no memory corruption
	#define check_bounds_pooled(pPool, memblock) 0
#endif

// allocate all memory allocated from the pool
void free_all_pooled(BufferPool *pPool);

// designed to create a pool manager for a worker thread
PoolManager* pool_manager_create();
// once the worker thread is destroyed, we can release the manager also
void pool_manager_destroy(PoolManager* mgr);

// single threaded ( non thread safe ) pop a pool from the list of pools
BufferPool* pool_manager_pop_st(PoolManager* mgr);
// push back a pool to the pool manager. This will flush all pool allocations. Make sure to call it on context destroy
void pool_manager_push_st(PoolManager* mgr, BufferPool* pool);

/*
* Implementation for object alloc pools. Used to avoid creating flows or list elements
*/
typedef struct StructPool StructPool;

StructPool* struct_pool_create(size_t elem_size, size_t count);
void* struct_pool_alloc(StructPool* pool);
int struct_pool_free(StructPool* pool, void* ptr);
void struct_pool_destroy(StructPool* pool);