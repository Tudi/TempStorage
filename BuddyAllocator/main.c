#include <stdlib.h>

#define NodeStatus_Free 1
#define NodeStatus_Taken 2
#define NodeStatus_Split 3

typedef struct BuddyPart BuddyPart;

struct BuddyPart
{
	size_t Mysize;			// each buddy might have 2 sub buddies
	void *BaseMemory;	// this block that can be split or use it directly
	int NodeStatus;		// newly created nodes should be free
	BuddyPart *SubBuddy[3];
};

typedef struct
{
	int RawMemorySize;
	void *InitialMemoryStart;
	BuddyPart BuddyMemory;
}buddy_allocator_t;

/**
* Create a buddy allocator
* @param raw_memory Backing memory
* @param memory_size Backing memory size
* @return the new buddy allocator
*/
buddy_allocator_t *buddy_allocator_create(void *raw_memory, size_t	raw_memory_size)
{
	buddy_allocator_t *ret = (buddy_allocator_t*)raw_memory;
	ret->RawMemorySize = raw_memory_size;
	ret->InitialMemoryStart = raw_memory;
	ret->BuddyMemory.BaseMemory = ((char*)raw_memory) + sizeof(buddy_allocator_t);
	ret->BuddyMemory.SubBuddy[0] = NULL;
	ret->BuddyMemory.SubBuddy[1] = NULL;
	ret->BuddyMemory.SubBuddy[2] = NULL;
	ret->BuddyMemory.Mysize = raw_memory_size - sizeof(buddy_allocator_t);
	ret->BuddyMemory.NodeStatus = NodeStatus_Free;
	return ret;
}

/**
* Destroy a buddy allocator
* @param buddy_allocator
*/
void buddy_allocator_destroy(buddy_allocator_t *buddy_allocator)
{
	buddy_allocator->RawMemorySize = 0;
	buddy_allocator->InitialMemoryStart = NULL;
}

BuddyPart *SearchSmallestFreeNode(BuddyPart *CurNode, size_t size)
{
	if (CurNode->Mysize >= size && CurNode->Mysize / 2 < size && CurNode->NodeStatus == NodeStatus_Free)
		return CurNode;
	//can no longer search for a good node on this branch
	if (CurNode->Mysize < size)
		return NULL;
	if (CurNode->NodeStatus == NodeStatus_Taken)
		return NULL;
	//Try to split ourself if not done yet
	if (CurNode->SubBuddy[0] == NULL)
	{
		BuddyPart *part = (BuddyPart *)malloc(sizeof(BuddyPart));
		part->BaseMemory = CurNode->BaseMemory;
		part->Mysize = CurNode->Mysize / 2;
		part->NodeStatus = NodeStatus_Free;
		part->SubBuddy[0] = NULL;
		part->SubBuddy[1] = NULL;
		part->SubBuddy[2] = CurNode; // parent node
		CurNode->SubBuddy[0] = part;

		part = (BuddyPart *)malloc(sizeof(BuddyPart));
		part->BaseMemory = (char *)CurNode->BaseMemory + CurNode->Mysize / 2;
		part->Mysize = CurNode->Mysize / 2;
		part->NodeStatus = NodeStatus_Free;
		part->SubBuddy[0] = NULL;
		part->SubBuddy[1] = NULL;
		part->SubBuddy[2] = CurNode; // parent node
		CurNode->SubBuddy[1] = part;

		CurNode->NodeStatus = NodeStatus_Split;
	}
	BuddyPart *Res = SearchSmallestFreeNode(CurNode->SubBuddy[0], size);
	if(Res == NULL)
		Res = SearchSmallestFreeNode(CurNode->SubBuddy[1], size);
	return Res;
}

/**
* Allocate memory
* @param buddy_allocator The buddy allocator
* @param size Size of memory to allocate
* @return pointer to the newly allocated memory, or @a NULL if out of memory
*/
void *buddy_allocator_alloc(buddy_allocator_t *buddy_allocator, size_t size)
{
	//quick sanity check to avoid checking 
	if (size >= buddy_allocator->BuddyMemory.Mysize || size == 0)
		return NULL;
	//keep searching for a node that is larger or equal than us, but next node would be too small
	BuddyPart *res = SearchSmallestFreeNode(&buddy_allocator->BuddyMemory, size);
	if (res == NULL)
		return NULL;
	//mark this node as taken
	res->NodeStatus = NodeStatus_Taken;
	return res->BaseMemory;
}

BuddyPart *SearchMemoryNodeAndFree(BuddyPart *CurNode, void *Block)
{
	if (CurNode->BaseMemory == Block)
	{
		//free this node
		CurNode->NodeStatus = NodeStatus_Free;
		//keep merging nodes until we find a node that is no longer mergable
		BuddyPart *ParentNode = CurNode->SubBuddy[2];
		while (ParentNode != NULL)
		{
			if (ParentNode->SubBuddy[0]->NodeStatus == NodeStatus_Free && ParentNode->SubBuddy[1]->NodeStatus == NodeStatus_Free)
			{
				free(ParentNode->SubBuddy[0]);
				free(ParentNode->SubBuddy[1]);
				ParentNode->NodeStatus = NodeStatus_Free;
				ParentNode->SubBuddy[0] = NULL;
				ParentNode->SubBuddy[1] = NULL;
				ParentNode = ParentNode->SubBuddy[2];
			}
			//nothing to merge anymore
			else
				break;
		}
		return CurNode;
	}
	BuddyPart *Res = NULL;
	if (CurNode->SubBuddy[0] != NULL)
	{
		Res = SearchMemoryNodeAndFree(CurNode->SubBuddy[0], Block);
		if (Res != NULL)
			return Res;
		Res = SearchMemoryNodeAndFree(CurNode->SubBuddy[1], Block);
	}
	return Res;
}

/**
* Deallocates a perviously allocated memory area.
* If @a ptr is @a NULL, it simply returns
* @param buddy_allocator The buddy allocator
* @param ptr The memory area to deallocate
*/
void buddy_allocator_free(buddy_allocator_t *buddy_allocator, void *ptr)
{
	SearchMemoryNodeAndFree(&buddy_allocator->BuddyMemory, ptr);
}

int main()
{
	size_t MemeoryToPlayWithSize = 5 * 1024 * 1024;
	void *MemoryBlock = malloc(MemeoryToPlayWithSize);
	buddy_allocator_t *MainBuddy = buddy_allocator_create(MemoryBlock, MemeoryToPlayWithSize);
	void *t1 = buddy_allocator_alloc(MainBuddy, 32 * 1024);
	void *t2 = buddy_allocator_alloc(MainBuddy, 64 * 1024);
	void *t3 = buddy_allocator_alloc(MainBuddy, 60 * 1024);
	void *t4 = buddy_allocator_alloc(MainBuddy, 150 * 1024);
	buddy_allocator_free(MainBuddy,t2);
	buddy_allocator_free(MainBuddy,t1);
	void *t5 = buddy_allocator_alloc(MainBuddy, 100 * 1024);
	void *t6 = buddy_allocator_alloc(MainBuddy, 100 * 1024);
}