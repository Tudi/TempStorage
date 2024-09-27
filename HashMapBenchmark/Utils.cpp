#include "StdAfx.h"

SIZE_T GetHeapMemoryUsage()
{
	HANDLE hHeap = GetProcessHeap();  // Get handle to the default heap
	if (hHeap == NULL) {
		return 0;
	}

	PROCESS_HEAP_ENTRY entry;
	entry.lpData = NULL;

	SIZE_T totalAllocatedBytes = 0;

	// Walk through the heap to find allocated blocks
	while (HeapWalk(hHeap, &entry) != FALSE)
	{
		if ((entry.wFlags & PROCESS_HEAP_ENTRY_BUSY) != 0) // Memory is allocated
		{
			totalAllocatedBytes += entry.cbData;  // Accumulate the allocated size
		}
	}

	if (GetLastError() != ERROR_NO_MORE_ITEMS)
	{
		return 0;
	}

	return totalAllocatedBytes;
}
