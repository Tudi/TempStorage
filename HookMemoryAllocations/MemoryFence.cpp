#include "StdAfx.h"
#include "MemoryFence.h"

int    GetFenceSize()
{
#if MEMORY_FENCE_SIZE > 0
    return sizeof(MemoryFenceGuard) * 2;
#else
    return 0;
#endif
}

#if !defined(DISABLE_HEADER_FENCE) || DISABLE_HEADER_FENCE == 0
void    *InitFence(void *InitAlloc, int OriginalAllocSize)
{
#if MEMORY_FENCE_SIZE == 0
    return InitAlloc;
#endif

    char *cInitAlloc = (char *)InitAlloc;
    MemoryFenceGuard *Header = (MemoryFenceGuard*)InitAlloc;
    MemoryFenceGuard *Footer = (MemoryFenceGuard*)&cInitAlloc[OriginalAllocSize + sizeof(MemoryFenceGuard)];
    ASSERT(sizeof(Header->Fence) % sizeof(int) == 0);

    memset(InitAlloc, 0, OriginalAllocSize);

    Header->OriPointer = InitAlloc;
    Footer->OriPointer = InitAlloc;

    Header->ShadowPointer = (void*)&cInitAlloc[sizeof(MemoryFenceGuard)];
    Footer->ShadowPointer = (void*)&cInitAlloc[sizeof(MemoryFenceGuard)];

    Header->AllocSize = OriginalAllocSize;
    Footer->AllocSize = OriginalAllocSize;

    int *IntFenceHeader = (int*)Header->Fence;
    int *IntFenceFooter = (int*)Footer->Fence;
    int IntFenceSize = MEMORY_FENCE_SIZE / sizeof(int);
    for (int i = 0; i < IntFenceSize; i++)
    {
        IntFenceHeader[i] = MEMORY_FENCE_SIGNITURE;
        IntFenceFooter[i] = MEMORY_FENCE_SIGNITURE;
    }

    return Header->ShadowPointer;
}

int    CheckFence(void *ShadowPointer, int Size)
{
#if MEMORY_FENCE_SIZE == 0
    return 0;
#endif
    if (ShadowPointer == NULL)
        return 0;

    char *cInitAlloc = (char *)ShadowPointer;
    MemoryFenceGuard *Header = (MemoryFenceGuard*)&cInitAlloc[-sizeof(MemoryFenceGuard)];

    if (Header->ShadowPointer != ShadowPointer)
        return FENCE_CHECK_FAIL_BAD_HEADER_SHADOWPOINTER;

    if (Header->OriPointer != Header)
        return FENCE_CHECK_FAIL_BAD_HEADER_ORIPOINTER;

    if (Size != Header->AllocSize)
        return FENCE_CHECK_FAIL_BAD_SIZE;

    int OriginalAllocSize = Header->AllocSize;
    cInitAlloc = (char*)Header->OriPointer;

    MemoryFenceGuard *Footer = (MemoryFenceGuard*)&cInitAlloc[OriginalAllocSize + sizeof(MemoryFenceGuard)];

    int *IntFenceHeader = (int*)Header->Fence;
    int *IntFenceFooter = (int*)Footer->Fence;
    int IntFenceSize = sizeof(Header->Fence) / sizeof(int);
    for (int i = 0; i < IntFenceSize; i++)
    {
        if (IntFenceHeader[i] != MEMORY_FENCE_SIGNITURE)
            return FENCE_CHECK_FAIL_BAD_HEADER_FENCE;
        if (IntFenceFooter[i] != MEMORY_FENCE_SIGNITURE)
            return FENCE_CHECK_FAIL_BAD_FOOTER_FENCE;
    }

    if (memcmp(Header, Footer, sizeof(MemoryFenceGuard)) != 0)
        return FENCE_CHECK_FAIL_BAD_HEADER_FOOTER_MISMATCH;
        
    return 0;
}

void    *GetOriginalPointerForFree(void *ShadowPointer)
{
#if MEMORY_FENCE_SIZE == 0
    return ShadowPointer;
#endif
    if (ShadowPointer == NULL)
        return NULL;
    char *cInitAlloc = (char *)ShadowPointer;
    MemoryFenceGuard *Header = (MemoryFenceGuard*)&cInitAlloc[-sizeof(MemoryFenceGuard)];
    return Header->OriPointer;
}
#else
void    *InitFence(void *InitAlloc, int OriginalAllocSize)
{
#if MEMORY_FENCE_SIZE == 0
    return InitAlloc;
#endif

    char *cInitAlloc = (char *)InitAlloc;
    MemoryFenceGuard *Footer = (MemoryFenceGuard*)&cInitAlloc[OriginalAllocSize];

    memset(InitAlloc, 0, OriginalAllocSize);

    Footer->OriPointer = InitAlloc;

    Footer->ShadowPointer = InitAlloc;

    Footer->AllocSize = OriginalAllocSize;

    int *IntFenceFooter = (int*)Footer->Fence;
    int IntFenceSize = MEMORY_FENCE_SIZE / sizeof(int);
    for (int i = 0; i < IntFenceSize; i++)
        IntFenceFooter[i] = MEMORY_FENCE_SIGNITURE;

    return Footer->ShadowPointer;
}

int    CheckFence(void *ShadowPointer, int Size )
{
#if MEMORY_FENCE_SIZE == 0
    return 0;
#endif
    if (ShadowPointer == NULL)
        return 0;

    char *cInitAlloc = (char *)ShadowPointer;

    MemoryFenceGuard *Footer = (MemoryFenceGuard*)&cInitAlloc[Size];

    if (Footer->ShadowPointer != ShadowPointer)
        return FENCE_CHECK_FAIL_BAD_HEADER_SHADOWPOINTER;

    if (Footer->OriPointer != Footer)
        return FENCE_CHECK_FAIL_BAD_HEADER_ORIPOINTER;

    if( Size != Footer->AllocSize )
        return FENCE_CHECK_FAIL_BAD_SIZE;

    int *IntFenceFooter = (int*)Footer->Fence;
    int IntFenceSize = MEMORY_FENCE_SIZE / sizeof(int);
    for (int i = 0; i < IntFenceSize; i++)
    {
        if (IntFenceFooter[i] != MEMORY_FENCE_SIGNITURE)
            return FENCE_CHECK_FAIL_BAD_FOOTER_FENCE;
    }

    return 0;
}

void    *GetOriginalPointerForFree(void *ShadowPointer)
{
    return ShadowPointer;
}
#endif

void    InitDeallocatedBlock(void *ptr, int size)
{
    char *cptr = (char*)ptr;
    for (int i = 0; i < size; i++)
        cptr[i] = 0xDD;
}

int     CheckDeallocatedBlock(const void *ptr, int size)
{
    char *cptr = (char*)ptr;
    for (int i = 0; i < size; i++)
        if (cptr[i] != 0xDD)
            return 1;
    return 0;
}