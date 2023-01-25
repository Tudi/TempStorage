#include <list.h>
#include <stdlib.h>

//
// Types
//

struct ListItem_t;

struct ListItem_t
{
    struct ListItem_t* previous;
    struct ListItem_t* next;

    void* value;
};

typedef
struct
{
    ListCompareKeysFunction_t compareKeysFn;
    ListFreeItemFunction_t freeItemFn;
    ListKeyFromItemFunction_t keyFromItemFn;

    struct ListItem_t* head;
} ListData_t;

//
// External interface
//

List_t list_init(ListCompareKeysFunction_t compareKeysFn, ListFreeItemFunction_t freeItemFn,
    ListKeyFromItemFunction_t keyFromItemFn)
{
    List_t ls = List_NULL;

    ListData_t* data = (ListData_t*) malloc(sizeof(ListData_t));
    if(data != NULL)
    {
        data->compareKeysFn = compareKeysFn;
        data->freeItemFn = freeItemFn;
        data->keyFromItemFn = keyFromItemFn;
        data->head = NULL;

        ls.lst = data;
    }

    return ls;
}

void list_free(List_t ls)
{
    ListData_t* data = (ListData_t*) ls.lst;
    if(data == NULL) { return; }

    struct ListItem_t* item = data->head;
    
    while(item != NULL)
    {
        struct ListItem_t* nextItem = item->next;
        data->freeItemFn(item->value);
        free(item);
        item = nextItem;
    }

    free(data);
}

bool list_isNull(List_t ls)
{
    return ls.lst == NULL;
}

bool list_addItem(List_t ls, void* value)
{
    ListData_t* data = (ListData_t*) ls.lst;

    struct ListItem_t* newItem = (struct ListItem_t*) malloc(sizeof(struct ListItem_t));
    if(newItem == NULL) { return false; }

    newItem->previous = NULL;
    newItem->next = NULL;
    newItem->value = value;

    if(data->head == NULL)
    {
        data->head = newItem;
        return true;
    }

    struct ListItem_t* previousItem = NULL;

    for(struct ListItem_t* item = data->head; item != NULL; item = item->next)
    {
        if(data->compareKeysFn(data->keyFromItemFn(value), data->keyFromItemFn(item->value)) < 0)
        {
            newItem->previous = item->previous;
            newItem->next = item;

            if(item->previous != NULL) { item->previous->next = newItem; }

            item->previous = newItem;

            if(data->head == item) { data->head = newItem; }

            return true;
        }

        previousItem = item;
    }

    previousItem->next = newItem;
    newItem->previous = previousItem;

    return true;
}

ListIterator_t list_begin(List_t ls)
{
    ListData_t* data = (ListData_t*) ls.lst;

    return (ListIterator_t) { .ls = ls.lst, .ps = data->head };
}

bool list_end(ListIterator_t* iter)
{
    return iter->ps == NULL;
}

void list_iteratorNext(ListIterator_t* iter)
{
    struct ListItem_t* item = (struct ListItem_t*) iter->ps;

    if(item != NULL) { iter->ps = item->next; }
}

void* list_iteratorPop(ListIterator_t* iter)
{
    ListData_t* data = (ListData_t*) iter->ls;
    struct ListItem_t* item = (struct ListItem_t*) iter->ps;

    if(data->head == item) { data->head = item->next; }

    iter->ps = item->next;

    if(item->previous != NULL) { item->previous->next = item->next; }
    if(item->next != NULL) { item->next->previous = item->previous; }

    void* value = item->value;
    free(item);

    return value;
}

void* list_iteratorValue(ListIterator_t* iter)
{
    struct ListItem_t* item = (struct ListItem_t*) iter->ps;
    
    return item->value;
}

ListIterator_t list_search(List_t ls, void* key)
{
    ListData_t* data = (ListData_t*) ls.lst;

    struct ListItem_t* item = data->head;

    for(; item != NULL; item = item->next) {
        if(data->compareKeysFn(key, data->keyFromItemFn(item->value)) == 0) { break; }
    }

    return (ListIterator_t) { .ls = ls.lst, .ps = item };
}

void list_deleteItem(ListIterator_t* iter)
{
    ListData_t* data = (ListData_t*) iter->ls;

    data->freeItemFn(list_iteratorPop(iter));
}
