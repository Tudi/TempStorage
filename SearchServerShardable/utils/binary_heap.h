#ifndef BINARY_HEAP_H
#define BINARY_HEAP_H

#include <stdbool.h>
#include <stdint.h>

// Types

typedef
struct
{
    void* bh;
} BinaryHeap_t;

typedef int (*BinaryHeapCompareFunction_t)(void*, void*);
typedef void (*BinaryHeapFreeItemFunction_t)(void*);

// Constants

#define BinaryHeap_NULL ((BinaryHeap_t) { .bh = NULL })

// Functions

BinaryHeap_t binaryHeap_init(BinaryHeapCompareFunction_t compareFn,
    BinaryHeapFreeItemFunction_t freeItemFn, uint32_t capacity);
void binaryHeap_free(BinaryHeap_t heap);
bool binaryHeap_isNull(BinaryHeap_t heap);

uint32_t binaryHeap_capacity(BinaryHeap_t heap);
uint32_t binaryHeap_size(BinaryHeap_t heap);

bool binaryHeap_addItem(BinaryHeap_t heap, void* value);
void binaryHeap_pushItemAndDeleteFront(BinaryHeap_t heap, void* value);

void* binaryHeap_getFront(BinaryHeap_t heap);
void* binaryHeap_popFront(BinaryHeap_t heap);
void* binaryHeap_getElementByIndex(BinaryHeap_t heap, int32_t index);
bool binaryHeap_deleteFront(BinaryHeap_t heap);

#endif // BINARY_HEAP_H
