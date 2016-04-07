#pragma once

#define MEMORY_FENCE_SIZE               0      //better make it multiple of 16 just ot be sure you do not break Intrinsic allignment requirements

#define MEMORY_FENCE_SIGNITURE          0xBADBEEF0

#define CHECK_WRITE_AFTER_DEALLOC       0       //instead free, we should periodically check if the content of the memory blocks changed
                                                // this can not guarantee that we did not write on it. It has a CHANCE to detect pointer use after delete
//#ifdef _CRTDBG_MAP_ALLOC
    #define DISABLE_HEADER_FENCE        1       //because custom allocators will realize if a pointer is invalid
                                                // reallocate will also crash if pointer is invalid
//#endif

#pragma pack(push, 1)   //make sure we ignore compiler data allignment, we need byte precision here
struct MemoryFenceGuard 
{
    union {
        char    Fence[MEMORY_FENCE_SIZE];
        char    Fence16[16];
    };
    union {
        void    *OriPointer;
        int     Pointer128bit[4];   //16 bytes !!
    };
    union {
        void    *ShadowPointer;
        int     Pointer128bit[4];   //16 bytes !!
    };
    union {
        int     AllocSize;
        int     Alloc128bit[4];     //16 bytes !!
    };
};
#pragma pack(pop)

enum FenceCheckErrorCodes
{
    FENCE_CHECK_FAIL_BAD_HEADER_SHADOWPOINTER = 1,
    FENCE_CHECK_FAIL_BAD_HEADER_ORIPOINTER,
    FENCE_CHECK_FAIL_BAD_HEADER_FENCE,
    FENCE_CHECK_FAIL_BAD_FOOTER_FENCE,
    FENCE_CHECK_FAIL_BAD_HEADER_FOOTER_MISMATCH,
    FENCE_CHECK_FAIL_BAD_SIZE,
};
int     GetFenceSize();
void    *InitFence(void *InitAlloc, int OriginalAllocSize);
int     CheckFence(void *ShadowPointer, int Size);
void    *GetOriginalPointerForFree(void *ShadowPointer);

void    InitDeallocatedBlock(void *ptr, int size);
int     CheckDeallocatedBlock(const void *ptr, int size);