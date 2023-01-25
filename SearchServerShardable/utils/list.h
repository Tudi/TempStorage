#ifndef LIST_H
#define LIST_H

#include <stdbool.h>

// Types

typedef
struct
{
    void* lst;
} List_t;

typedef struct
{
  void* ls;
  void* ps;
} ListIterator_t;

typedef int (*ListCompareKeysFunction_t)(void*, void*);
typedef void (*ListFreeItemFunction_t)(void*);
typedef void* (*ListKeyFromItemFunction_t)(void*);

// Constants

#define List_NULL ((List_t) { .lst = NULL })

// Functions

List_t list_init(ListCompareKeysFunction_t compareKeysFn, ListFreeItemFunction_t freeItemFn,
    ListKeyFromItemFunction_t keyFromItemFn);
void list_free(List_t ls);
bool list_isNull(List_t ls);

bool list_addItem(List_t ls, void* value);

ListIterator_t list_begin(List_t ls);
bool list_end(ListIterator_t* iter);

void list_iteratorNext(ListIterator_t* iter);
void* list_iteratorPop(ListIterator_t* iter);
void* list_iteratorValue(ListIterator_t* iter);

ListIterator_t list_search(List_t ls, void* key);
void list_deleteItem(ListIterator_t* iter);

#endif // LIST_H
