#include <binary_heap.h>
#include <stdlib.h>

//
// Types
//

typedef
struct
{
    BinaryHeapCompareFunction_t compareFn;
    BinaryHeapFreeItemFunction_t freeItemFn;

    uint32_t capacity;
    uint32_t size;
    void** values;
} BinaryHeapData_t;

//
// Local prototypes
//

static uint32_t parent(uint32_t i);
static uint32_t left(uint32_t i);
static uint32_t right(uint32_t i);

static void swap(void** values, uint32_t i, uint32_t j);

static void percolateUp(BinaryHeapData_t* data, uint32_t i);
static void percolateDown(BinaryHeapData_t* data, uint32_t i);

//
// External interface
//

BinaryHeap_t binaryHeap_init(BinaryHeapCompareFunction_t compareFn,
    BinaryHeapFreeItemFunction_t freeItemFn, uint32_t capacity)
{
    BinaryHeap_t heap = BinaryHeap_NULL;

    BinaryHeapData_t* data = (BinaryHeapData_t*) malloc(sizeof(BinaryHeapData_t));
    if(data == NULL) { return heap; }

    data->compareFn = compareFn;
    data->freeItemFn = freeItemFn;

    data->capacity = capacity;
    data->size = 0;
    data->values = (void**) calloc(capacity, sizeof(void*));
    if(data->values == NULL)
    {
        free(data);
        return heap;
    }

    heap.bh = data;

    return heap;
}

void binaryHeap_free(BinaryHeap_t heap)
{
    BinaryHeapData_t* data = (BinaryHeapData_t*) heap.bh;
    if(data == NULL) { return; }

    for(uint32_t i = 0; i < data->size; ++i) {
        data->freeItemFn(data->values[i]);
    }

    free(data->values);
    free(data);
}

bool binaryHeap_isNull(BinaryHeap_t heap)
{
    return heap.bh == NULL;
}

uint32_t binaryHeap_capacity(BinaryHeap_t heap)
{
    BinaryHeapData_t* data = (BinaryHeapData_t*) heap.bh;

    return data->capacity;
}

uint32_t binaryHeap_size(BinaryHeap_t heap)
{
    BinaryHeapData_t* data = (BinaryHeapData_t*) heap.bh;

    return data->size;
}

bool binaryHeap_addItem(BinaryHeap_t heap, void* value)
{
    BinaryHeapData_t* data = (BinaryHeapData_t*) heap.bh;

    if(value == NULL || data->size == data->capacity) { return false; }

    data->values[data->size] = value;
    percolateUp(data, data->size);

    ++(data->size);

    return true;
}

void binaryHeap_pushItemAndDeleteFront(BinaryHeap_t heap, void* value)
{
    BinaryHeapData_t* data = (BinaryHeapData_t*) heap.bh;

    if(data->size < data->capacity)
    {
        data->values[data->size] = value;
        percolateUp(data, data->size);

        ++(data->size);
    }
    else
    {
        if(data->compareFn(value, data->values[0]) < 0)
        {
            data->freeItemFn(value);
        }
        else
        {
            data->freeItemFn(data->values[0]);

            data->values[0] = value;
            percolateDown(data, 0);
        }
    }
}

void* binaryHeap_getFront(BinaryHeap_t heap)
{
    BinaryHeapData_t* data = (BinaryHeapData_t*) heap.bh;

    return data->values[0];
}

void* binaryHeap_popFront(BinaryHeap_t heap)
{
    BinaryHeapData_t* data = (BinaryHeapData_t*) heap.bh;

    void* item = NULL;

    if(data->size > 0)
    {
        item = data->values[0];

        data->values[0] = data->values[data->size - 1];
        data->values[data->size - 1] = NULL;
        --(data->size);

        percolateDown(data, 0);
    }

    return item;
}

bool binaryHeap_deleteFront(BinaryHeap_t heap)
{
    BinaryHeapData_t* data = (BinaryHeapData_t*) heap.bh;

    if(data->size == 0) { return false; }

    data->freeItemFn(data->values[0]);

    data->values[0] = data->values[data->size - 1];
    data->values[data->size - 1] = NULL;
    --(data->size);

    percolateDown(data, 0);

    return true;
}

//
// Local functions
//

static uint32_t left(uint32_t i)
{
    return 2 * i + 1;
}

static uint32_t right(uint32_t i)
{
    return 2 * i + 2;
}

static uint32_t parent(uint32_t i)
{
    return (i - 1) / 2;
}

static void swap(void** values, uint32_t i, uint32_t j)
{
    void* temp = values[i];
    values[i] = values[j];
    values[j] = temp;
}

static void percolateUp(BinaryHeapData_t* data, uint32_t i)
{
    while(i > 0 && data->compareFn(data->values[i], data->values[parent(i)]) < 0)
    {
        swap(data->values, i, parent(i));
        i = parent(i);
    }
}

static void percolateDown(BinaryHeapData_t* data, uint32_t i)
{
    while(true)
    {
        uint32_t smallest = i;

        uint32_t l = left(smallest);
        uint32_t r = right(smallest);

        if(l < data->size && data->compareFn(data->values[smallest], data->values[l]) > 0) {
            smallest = l;
        }

        if(r < data->size && data->compareFn(data->values[smallest], data->values[r]) > 0) {
            smallest = r;
        }

        if(i == smallest) { break; }

        swap(data->values, i, smallest);
        i = smallest;
    }
}

void* binaryHeap_getElementByIndex(BinaryHeap_t heap, int32_t index)
{
    BinaryHeapData_t* data = (BinaryHeapData_t*)heap.bh;
    if (index >= data->size)
    {
        return NULL;
    }
    return data->values[index];
}