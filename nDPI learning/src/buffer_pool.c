#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "buffer_pool.h"
#include "ini_file_handler.h"
#include "log_manager.h"

#define BLOCK_ALLOCK_SIZE (2 * 1024 * 1024)
#define MIN_FREE_BYTES_KEEP_BLOCK 512

typedef struct PoolNode {
    BufferPool* pool;
    struct PoolNode* next;
} PoolNode;

struct PoolManager {
    PoolNode* head; // contains a list of nodes that have a pool attached
    PoolNode* empty_nodes; // list of nodes that do not have a pool attached
    pthread_mutex_t lock;
};

// based on config settings
int64_t g_block_size = BLOCK_ALLOCK_SIZE;
int64_t g_min_remaining = MIN_FREE_BYTES_KEEP_BLOCK;
int64_t g_default_pools_in_manager = 1;
int64_t g_default_blocks_in_pool = 1;
int64_t g_default_structs_in_pool = 100;
//PoolManager g_pool_manager;

// hopefully will never be required. Just in case we do some error hunting in the future
// Make sure the header takes up a multitude of USE_ALLIGNMENT_TO > failing tests will let you know if you hit the right spot
#define BOUNDS_SIGNITURE_VALUE 0xFBADDADFEEDFACEF
#define BOUNDS_SIGNATURE_SIZE sizeof(uint64_t) // should not change this
#define BOUNDS_SIZE_STORE sizeof(uint64_t)
#define BOUNDS_BLOCKS_BEFORE 1   
#define BOUNDS_BLOCKS_AFTER 1    

void pool_manager_init(PoolManager* pPoolManager);
BufferPool* buffer_pool_create();
void pool_manager_push_st(PoolManager* mgr, BufferPool* pool);
void init_pool_manager() {
    int64_t blockSize = get_ini_int_value("BufferPool", "BlockAllocSize", BLOCK_ALLOCK_SIZE);
    // sanity bounds check init
    if (blockSize > 1024 && blockSize < 512 * 1024 * 1024) {
        g_block_size = blockSize;
    }

    int64_t minFreeSizeKeepBlock = get_ini_int_value("BufferPool", "MinFreeToKeep", MIN_FREE_BYTES_KEEP_BLOCK);
    // sanity bounds check init
    if (minFreeSizeKeepBlock > MIN_FREE_BYTES_KEEP_BLOCK && minFreeSizeKeepBlock < 512 * 1024 * 1024) {
        g_min_remaining = minFreeSizeKeepBlock;
    }

    int64_t numInitialPools = get_ini_int_value("BufferPool", "InitialBufferPools", 100);
    // sanity bounds check
    if (numInitialPools < 1)
        numInitialPools = 100;
    else if (numInitialPools > 10000)
        numInitialPools = 100;
    g_default_pools_in_manager = numInitialPools;

    int64_t numStructsPools = get_ini_int_value("BufferPool", "InitialStructPoolElements", 100);
    // sanity bounds check
    if (numStructsPools < 1)
        numStructsPools = 100;
    else if (numStructsPools > 100000)
        numStructsPools = 100;
    g_default_structs_in_pool = numStructsPools;

    // init the global pool manager
/*    pool_manager_init(&g_pool_manager);
    for (int64_t i = 0; i < g_default_pools_in_manager; i++) {
        BufferPool* pPool = buffer_pool_create();
        if (pPool) {
            pool_manager_push_st(&g_pool_manager, pPool);
        }
    }
    */
}

void destroy_pool_manager()
{
    // destroy the global pool manager
//    pool_manager_destroy(&g_pool_manager);
}

typedef struct MemoryBlock {
    uint8_t *data;
    int64_t used;
    struct MemoryBlock *next;
} MemoryBlock;

struct BufferPool {
    MemoryBlock *current;
    MemoryBlock *blocks;    // in-use + current
    MemoryBlock *consumed;  // full or low-space
    MemoryBlock *freed;     // available for reuse after reset
};

MemoryBlock *buffer_block_create(size_t size) {
    MemoryBlock *memblock = malloc(sizeof(MemoryBlock));
    memblock->data = malloc(size);
    memblock->used = 0;
    memblock->next = NULL;
    return memblock;
}

void buffer_block_reset(MemoryBlock *memblock) {
    memblock->used = 0;
    memblock->next = NULL;
}

BufferPool *buffer_pool_create() {
    BufferPool *pool = malloc(sizeof(BufferPool));
    pool->current = NULL;
    pool->blocks = NULL;
    pool->consumed = NULL;
    pool->freed = NULL;

    // prealloc couple of memory blocks to avoid fluctuating memory usage
    for (int64_t i = 0; i < g_default_blocks_in_pool; i++) {
        MemoryBlock* new_blk = buffer_block_create(g_block_size);
        if (new_blk) {
            new_blk->next = pool->blocks;
            pool->blocks = new_blk;
        }
    }
    return pool;
}

void* alloc_from_block(MemoryBlock* block, int64_t size, int64_t size_padded) {
    uint8_t* ptr = block->data + block->used;
    block->used += size_padded;
#ifdef USE_BOUNDS_CHECKING
    uint64_t* ptr_bounds_size = (uint64_t *)(ptr);
    uint64_t *ptr_bounds_start = (uint64_t*)(ptr + BOUNDS_SIZE_STORE);
    uint8_t *ptr_user = (uint8_t*)(ptr + BOUNDS_SIZE_STORE + BOUNDS_BLOCKS_BEFORE * BOUNDS_SIGNATURE_SIZE);
    int64_t user_size = size - BOUNDS_SIZE_STORE - BOUNDS_BLOCKS_BEFORE * BOUNDS_SIGNATURE_SIZE - BOUNDS_BLOCKS_AFTER * BOUNDS_SIGNATURE_SIZE;
    uint64_t* ptr_bounds_trail = (uint64_t*)(ptr_user + user_size);

    // store the size of the allocated space so we can check trailing bytes later on
    *ptr_bounds_size = user_size;

    for (int64_t i = 0; i < BOUNDS_BLOCKS_BEFORE; i++) {
        ptr_bounds_start[i] = BOUNDS_SIGNITURE_VALUE;
    }
    for (int64_t i = 0; i < BOUNDS_BLOCKS_AFTER; i++) {
        ptr_bounds_trail[i] = BOUNDS_SIGNITURE_VALUE;
    }
    return ptr_user;
#else 
    return ptr;
#endif
}

void* alloc_pooled(BufferPool* pool, int64_t size) {

#ifdef USE_BOUNDS_CHECKING
    size = BOUNDS_SIZE_STORE + BOUNDS_BLOCKS_BEFORE * BOUNDS_SIGNATURE_SIZE + size + BOUNDS_BLOCKS_AFTER * BOUNDS_SIGNATURE_SIZE;
#endif
#if USE_ALLIGNMENT_TO != 0
    int64_t size_padded = (size + USE_ALLIGNMENT_TO - 1) & ~(USE_ALLIGNMENT_TO - 1);  // same as size = ((size + (USE_ALLIGNMENT_TO - 1)) / USE_ALLIGNMENT_TO)* USE_ALLIGNMENT_TO;
#endif

    // special case that should never happen
    if (size_padded > g_block_size) {
        // Oversized: standalone block, goes to consumed
        MemoryBlock* new_blk = buffer_block_create(size_padded);
        // consume it, and also init bounds checking if we are using it
        void* ptr = alloc_from_block(new_blk, size, size_padded);
        // push it to our list of buffers
        new_blk->next = pool->consumed;
        pool->consumed = new_blk;
        // warn the sys admin to increase pool sizes or fix a bug
        AddLogEntryB(LDF_LOCAL, LogSeverityWarn, LogSourceBufferPool, "Requested %lld bytes, max block size is %lld. Huge performance hit. Increase block sizes in config", size_padded, g_block_size);
        // return the allocated pointer
        return ptr;
    }

    // Try to find a block in `blocks` with enough free space
    MemoryBlock* prev = NULL;
    MemoryBlock* blk = pool->blocks;

    // not expecting to have more than 1-2 elements. If it does, should increase g_min_remaining to avoid such situation
    while (blk) {
        int64_t remaining = g_block_size - blk->used;
        if (remaining >= size_padded) {
            void* ptr = alloc_from_block(blk, size, size_padded);

            // Check if it’s now consumed
            if ((g_block_size - blk->used) < g_min_remaining) {
                // Remove from `blocks` list
                if (prev) {
                    prev->next = blk->next;
                }

                // Move to `consumed`
                blk->next = pool->consumed;
                pool->consumed = blk;
            }

            return ptr;
        }

        prev = blk;
        blk = blk->next;
    }

    // No suitable block found — get one from freed list or allocate new
    MemoryBlock* new_blk = NULL;
    if (pool->freed) {
        new_blk = pool->freed;
        pool->freed = new_blk->next;
        buffer_block_reset(new_blk);
    }
    else {
        new_blk = buffer_block_create(g_block_size);
    }

    void* ptr = alloc_from_block(new_blk, size, size_padded);

    // Decide whether to keep in blocks or move to consumed immediately
    int64_t remaining = g_block_size - new_blk->used;
    if (remaining < g_min_remaining) {
        new_blk->next = pool->consumed;
        pool->consumed = new_blk;
    }
    else {
        new_blk->next = pool->blocks;
        pool->blocks = new_blk;
    }

    return ptr;
}

#ifdef USE_BOUNDS_CHECKING
int check_bounds_pooled(BufferPool* pool, void* memblock) {

    // sanity check
    if (!pool || !memblock) {
        return -1;
    }

    int64_t foundInPool = 0;
    MemoryBlock* blk = pool->blocks;
    while (blk) {
        void* block_start = blk->data;
        void* block_end = (uint8_t *)blk->data + blk->used;
        if (block_start<= memblock && memblock < block_end) {
            foundInPool = 1;
            break;
        }
        blk = blk->next;
    }
    if (foundInPool == 0) {
        blk = pool->consumed;
        while (blk) {
            void* block_start = blk->data;
            void* block_end = (uint8_t*)blk->data + blk->used;
            if (block_start <= memblock && memblock < block_end) {
                foundInPool = 1;
                break;
            }
            blk = blk->next;
        }
    }
    if (foundInPool == 0) {
        AddLogEntryB(LDF_LOCAL, LogSeverityWarn, LogSourceBufferPool, "Can't do bounds checking on a non pool allocated memory block");
        return -1;
    }
    uint8_t *ptr_user = (uint8_t*)memblock;
    uint64_t* ptr_bounds_size = (uint64_t*)(ptr_user - BOUNDS_SIZE_STORE - BOUNDS_BLOCKS_BEFORE * BOUNDS_SIGNATURE_SIZE);
    if ((uint8_t*)ptr_bounds_size < blk->data) {
        AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceBufferPool,
            "Pointer to bounds size metadata is out of range");
        return -1;
    }
    int64_t user_size = *ptr_bounds_size;

    uint64_t* ptr_bounds_start = (uint64_t*)(ptr_user - BOUNDS_BLOCKS_BEFORE * BOUNDS_SIGNATURE_SIZE);
    int64_t bounds_check_fails_pre = 0;
    for (int64_t i = 0; i < BOUNDS_BLOCKS_BEFORE; i++) {
        if (ptr_bounds_start[i] != BOUNDS_SIGNITURE_VALUE) {
            bounds_check_fails_pre++;
        }
    }
    if (bounds_check_fails_pre) {
        AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceBufferPool, "Bounds check failed ! failed block before %lld", bounds_check_fails_pre);
        return 1;
    }
    uint64_t* ptr_bounds_trail = (uint64_t*)(ptr_user + user_size);
    int64_t bounds_check_fails_post = 0;
    for (int64_t i = 0; i < BOUNDS_BLOCKS_AFTER; i++) {
        if (ptr_bounds_trail[i] != BOUNDS_SIGNITURE_VALUE) {
            bounds_check_fails_post++;
        }
    }
    if (bounds_check_fails_post) {
        AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceBufferPool, "Bounds check failed ! Failed blocks after %lld", bounds_check_fails_post);
        return 1;
    }
    return 0;
}
#endif
void free_all_pooled(BufferPool *pool) {
    // Move all used blocks to freed
    MemoryBlock *memblock = pool->blocks;
    while (memblock) {
        MemoryBlock *next = memblock->next;
        memblock->next = pool->freed;
        pool->freed = memblock;
        memblock = next;
    }

    MemoryBlock *consumed = pool->consumed;
    while (consumed) {
        MemoryBlock *next = consumed->next;
        consumed->next = pool->freed;
        pool->freed = consumed;
        consumed = next;
    }

    pool->current = NULL;
    pool->blocks = NULL;
    pool->consumed = NULL;
}

void buffer_pool_destroy(BufferPool *pool) {
    free_all_pooled(pool);

    MemoryBlock *memblock = pool->freed;
    while (memblock) {
        MemoryBlock *next = memblock->next;
        free(memblock->data);
        free(memblock);
        memblock = next;
    }

    free(pool);
}

//////////////////////////////////////////
// Pool manager implementation
//////////////////////////////////////////

void pool_manager_init(PoolManager* pPoolManager) {
    pPoolManager->head = NULL;
    pPoolManager->empty_nodes = NULL;
    pthread_mutex_init(&pPoolManager->lock, NULL);
}

PoolManager* pool_manager_create() {
    PoolManager* mgr = malloc(sizeof(PoolManager));
    pool_manager_init(mgr);

    // push couple of pools into the manager to avoid later stalls
    for (int64_t i = 0; i < g_default_pools_in_manager; i++) {
        BufferPool* pPool = buffer_pool_create();
        if (pPool) {
            pool_manager_push_st(mgr, pPool);
        }
    }

    return mgr;
}

void pool_manager_destroy(PoolManager* mgr) {
    pthread_mutex_lock(&mgr->lock);
    PoolNode* node = mgr->head;
    while (node) {
        PoolNode* next = node->next;
        buffer_pool_destroy(node->pool); // from earlier
        free(node);
        node = next;
    }
    mgr->head = NULL;

    node = mgr->empty_nodes;
    int64_t missing_node_count = 0;
    while (node) {
        PoolNode* next = node->next;
        free(node);
        node = next;
        missing_node_count++;
    }
    mgr->empty_nodes = NULL;

    if (missing_node_count) {
        AddLogEntryB(LDF_LOCAL, LogSeverityWarn, LogSourceBufferPool, "Pool manager is missing %lld pools at shutdown", missing_node_count);
    }
    pthread_mutex_unlock(&mgr->lock);
    pthread_mutex_destroy(&mgr->lock);
    free(mgr);
}

void pool_manager_push_mt(PoolManager* mgr, BufferPool* pool) {
    pthread_mutex_lock(&mgr->lock);

    free_all_pooled(pool);

    PoolNode* node;
    if (mgr->empty_nodes) {
        node = mgr->empty_nodes;
        mgr->empty_nodes = mgr->empty_nodes->next;
    }
    else {
        node = malloc(sizeof(PoolNode));
    }
    node->pool = pool;
    node->next = mgr->head;
    mgr->head = node;

    pthread_mutex_unlock(&mgr->lock);
}

BufferPool* pool_manager_pop_mt(PoolManager* mgr) {
    pthread_mutex_lock(&mgr->lock);

    BufferPool* pool = NULL;
    if (mgr->head) {
        PoolNode* node = mgr->head;
        mgr->head = node->next;
        pool = node->pool;
        node->next = mgr->empty_nodes;
        mgr->empty_nodes = node;
    }

    pthread_mutex_unlock(&mgr->lock);

    if (!pool) {
        pool = buffer_pool_create(); // fallback
    }

    return pool;
}

void pool_manager_push_st(PoolManager* mgr, BufferPool* pool) {
    free_all_pooled(pool);

    PoolNode* node;
    if (mgr->empty_nodes) {
        node = mgr->empty_nodes;
        mgr->empty_nodes = mgr->empty_nodes->next;
    }
    else {
        node = malloc(sizeof(PoolNode));
    }
    node->pool = pool;
    node->next = mgr->head;
    mgr->head = node;
}

BufferPool* pool_manager_pop_st(PoolManager* mgr) {
    BufferPool* pool = NULL;
    if (mgr->head) {
        PoolNode* node = mgr->head;
        mgr->head = node->next;
        pool = node->pool;
        node->next = mgr->empty_nodes;
        mgr->empty_nodes = node;
    }

    if (!pool) {
        pool = buffer_pool_create(); // fallback
    }

    return pool;
}

/*
BufferPool* get_buffer_pool() {
    return pool_manager_pop_mt(&g_pool_manager);
}
*/

//////////////////////////////////////////
// Struct Pool implementation
//////////////////////////////////////////

typedef struct FixedBufferChunk {
    void* memory;
    struct FixedBufferChunk* next;
} FixedBufferChunk;

typedef struct StructPool {
    size_t elem_size;
    size_t elem_size_padded;
    size_t elems_per_chunk;

    void** freelist;
    size_t capacity;
    size_t top;

    FixedBufferChunk* chunks;
} StructPool;

static int add_chunk(StructPool* pool) {
    FixedBufferChunk* chunk = malloc(sizeof(FixedBufferChunk));
    if (!chunk) {
        return 0;
    }

    size_t raw_size = pool->elem_size_padded * pool->elems_per_chunk;
#ifdef USE_BOUNDS_CHECKING
    size_t bounds_padding = BOUNDS_SIZE_STORE + BOUNDS_BLOCKS_BEFORE * BOUNDS_SIGNATURE_SIZE + BOUNDS_BLOCKS_AFTER * BOUNDS_SIGNATURE_SIZE;
#if USE_ALLIGNMENT_TO != 0
    bounds_padding = (bounds_padding + USE_ALLIGNMENT_TO - 1) & ~(USE_ALLIGNMENT_TO - 1);
#endif
    raw_size += bounds_padding * pool->elems_per_chunk;
#endif
    chunk->memory = malloc(raw_size);
    if (!chunk->memory) {
        free(chunk);
        return 0;
    }

    chunk->next = pool->chunks;
    pool->chunks = chunk;

    void* base = chunk->memory;
    size_t new_capacity = pool->capacity + pool->elems_per_chunk;
    pool->freelist = realloc(pool->freelist, new_capacity * sizeof(void*));

    for (size_t i = 0; i < pool->elems_per_chunk; ++i) {
#ifdef USE_BOUNDS_CHECKING
        uint8_t* ptr = (uint8_t*)base + i * (pool->elem_size_padded + bounds_padding);
        uint64_t* ptr_bounds_size = (uint64_t*)ptr;
        uint64_t* ptr_bounds_start = (uint64_t*)(ptr + BOUNDS_SIZE_STORE);
        uint8_t* user_ptr = ptr + BOUNDS_SIZE_STORE + BOUNDS_BLOCKS_BEFORE * BOUNDS_SIGNATURE_SIZE;
        uint64_t* ptr_bounds_trail = (uint64_t*)(user_ptr + pool->elem_size);

        *ptr_bounds_size = pool->elem_size;
        for (uint64_t j = 0; j < BOUNDS_BLOCKS_BEFORE; j++) {
            ptr_bounds_start[j] = BOUNDS_SIGNITURE_VALUE;
        }
        for (uint64_t j = 0; j < BOUNDS_BLOCKS_AFTER; j++) {
            ptr_bounds_trail[j] = BOUNDS_SIGNITURE_VALUE;
        }

        pool->freelist[pool->top++] = user_ptr;
#else
        uint8_t* ptr = (uint8_t*)base + i * pool->elem_size_padded;
        pool->freelist[pool->top++] = ptr;
#endif
    }

    pool->capacity = new_capacity;
    return 1;
}

StructPool* struct_pool_create(size_t elem_size, size_t count) {
    StructPool* pool = malloc(sizeof(StructPool));
    if (!pool) {
        return NULL;
    }

#if USE_ALLIGNMENT_TO != 0
    size_t elem_size_padded = (elem_size + USE_ALLIGNMENT_TO - 1) & ~(USE_ALLIGNMENT_TO - 1);
#endif

    if (count == 0)
        count = g_default_structs_in_pool;

    pool->elem_size = elem_size;
    pool->elem_size_padded = elem_size_padded;
    pool->elems_per_chunk = count;
    pool->capacity = 0;
    pool->top = 0;
    pool->freelist = NULL;
    pool->chunks = NULL;

    if (!add_chunk(pool)) {
        free(pool);
        return NULL;
    }

    return pool;
}

void* struct_pool_alloc(StructPool* pool) {
    if (pool->top == 0) {
        if (!add_chunk(pool)) {
            return NULL;
        }
    }
    return pool->freelist[--pool->top];
}

int struct_pool_free(StructPool* pool, void* ptr) {
#ifdef USE_BOUNDS_CHECKING
    uint64_t found_in_pool = 0;
    FixedBufferChunk* chunk = pool->chunks;
    size_t bounds_padding = BOUNDS_SIZE_STORE + BOUNDS_BLOCKS_BEFORE * BOUNDS_SIGNATURE_SIZE + BOUNDS_BLOCKS_AFTER * BOUNDS_SIGNATURE_SIZE;
#if USE_ALLIGNMENT_TO != 0
    bounds_padding = (bounds_padding + USE_ALLIGNMENT_TO - 1) & ~(USE_ALLIGNMENT_TO - 1);
#endif
    size_t total_elem_size = pool->elem_size_padded + bounds_padding;

    while (chunk) {
        uint8_t* base = (uint8_t*)chunk->memory;
        for (size_t i = 0; i < pool->elems_per_chunk; ++i) {
            uint8_t* user_ptr = base + i * total_elem_size + BOUNDS_SIZE_STORE + BOUNDS_BLOCKS_BEFORE * BOUNDS_SIGNATURE_SIZE;
            if (user_ptr == ptr) {
                found_in_pool = 1;

                uint64_t* ptr_bounds_size = (uint64_t*)(user_ptr - BOUNDS_SIZE_STORE - BOUNDS_BLOCKS_BEFORE * BOUNDS_SIGNATURE_SIZE);
                uint64_t* ptr_bounds_start = (uint64_t*)(user_ptr - BOUNDS_BLOCKS_BEFORE * BOUNDS_SIGNATURE_SIZE);
                uint64_t* ptr_bounds_trail = (uint64_t*)(user_ptr + *ptr_bounds_size);

                uint64_t pre_fail = 0, post_fail = 0;
                for (uint64_t j = 0; j < BOUNDS_BLOCKS_BEFORE; ++j) {
                    if (ptr_bounds_start[j] != BOUNDS_SIGNITURE_VALUE) {
                        pre_fail++;
                    }
                }
                for (uint64_t j = 0; j < BOUNDS_BLOCKS_AFTER; ++j) {
                    if (ptr_bounds_trail[j] != BOUNDS_SIGNITURE_VALUE) {
                        post_fail++;
                    }
                }

                if (pre_fail || post_fail) {
                    AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceBufferPool,
                        "Bounds check failed in struct_pool_free: before=%d, after=%d", pre_fail, post_fail);
                    return 1;
                }

                break;
            }
        }
        chunk = chunk->next;
    }

    if (!found_in_pool) {
        AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceBufferPool,
            "Attempt to free a pointer not allocated from this struct pool");
        return -1;
    }
#endif

    if (pool->top == pool->capacity) {
        AddLogEntryB(LDF_LOCAL, LogSeverityError, LogSourceBufferPool,
            "Trying to push more buffers to pool than requested");
        return -1;
    }

    pool->freelist[pool->top++] = ptr;
    return 0;
}


void struct_pool_destroy(StructPool* pool) {
    if (!pool) {
        return;
    }

    FixedBufferChunk* chunk = pool->chunks;
    while (chunk) {
        FixedBufferChunk* next = chunk->next;
        free(chunk->memory);
        free(chunk);
        chunk = next;
    }

    free(pool->freelist);
    free(pool);
}
