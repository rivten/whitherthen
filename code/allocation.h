
#ifdef __cplusplus
extern "C" {
#endif

enum platform_memory_block_flags
{
    PlatformMemory_NotRestored = 0x1,
    PlatformMemory_OverflowCheck = 0x2,
    PlatformMemory_UnderflowCheck = 0x4,
};

struct platform_memory_block
{
    u64 Flags;
    u64 Size;
    u8 *Base;
    umm Used;
    platform_memory_block *ArenaPrev;
};

#define PLATFORM_ALLOCATE_MEMORY(name) platform_memory_block *name(memory_index Size, u64 Flags)
typedef PLATFORM_ALLOCATE_MEMORY(platform_allocate_memory);
    
#define PLATFORM_DEALLOCATE_MEMORY(name) void name(platform_memory_block *Block)
typedef PLATFORM_DEALLOCATE_MEMORY(platform_deallocate_memory);
    
#ifdef __cplusplus
}
#endif

#ifdef ALLOCATION_IMPLEMENTATION
enum sokol_memory_block_flag
{
    SokolMem_AllocatedDuringLooping = 0x1,
    SokolMem_FreedDuringLooping = 0x2,
};

struct sokol_memory_block
{
    platform_memory_block Block;
    sokol_memory_block* Prev;
    sokol_memory_block* Next;
    u64 LoopingFlags;
};

// NOTE(hugo): This could be pointers to struct and then passed to the 
// allocation system by another system
global sokol_memory_block* GlobalMemorySentinel;
global ticket_mutex* GlobalMemoryMutex;

// NOTE(hugo) : Not very happy about this one in particular...
// There is a allocation.h, allocation.cpp and 
// some structs still defined in sokol_whitherthen.h
PLATFORM_ALLOCATE_MEMORY(SokolAllocateMemory)
{
    // NOTE(casey): We require memory block headers not to change the cache
    // line alignment of an allocation
    // TODO(hugo): Write Win32 and Linux specific code
    Assert(sizeof(sokol_memory_block) == 64);

    umm PageSize = 4096;
    umm TotalSize = Size + sizeof(sokol_memory_block);
    umm BaseOffset = sizeof(sokol_memory_block);
    umm ProtectOffset = 0;
    if(Flags & PlatformMemory_UnderflowCheck)
    {
        TotalSize = Size + 2*PageSize;
        BaseOffset = 2*PageSize;
        ProtectOffset = PageSize;
    }
    else if(Flags & PlatformMemory_OverflowCheck)
    {
        umm SizeRoundedUp = AlignPow2(Size, PageSize);
        TotalSize = SizeRoundedUp + 2*PageSize;
        BaseOffset = PageSize + SizeRoundedUp - Size;
        ProtectOffset = PageSize + SizeRoundedUp;
    }

    sokol_memory_block* Block = (sokol_memory_block *)malloc(TotalSize);
    memset(Block, 0, sizeof(sokol_memory_block));
    Assert(Block);
    Block->Block.Base = (u8 *)Block + BaseOffset;
    Assert(Block->Block.Used == 0);
    Assert(Block->Block.ArenaPrev == 0);

    if(Flags & (PlatformMemory_UnderflowCheck|PlatformMemory_OverflowCheck))
    {
        NotImplemented;
        //DWORD OldProtect = 0;
        //BOOL Protected = VirtualProtect((u8 *)Block + ProtectOffset, PageSize, PAGE_NOACCESS, &OldProtect);
        //Assert(Protected);
    }

    sokol_memory_block* Sentinel = GlobalMemorySentinel;
    Block->Next = Sentinel;
    Block->Block.Size = Size;
    Block->Block.Flags = Flags;
    Block->LoopingFlags = 0;
    //if(Win32IsInLoop(&GlobalWin32State) && !(Flags & PlatformMemory_NotRestored))
    //{
        //Block->LoopingFlags = Win32Mem_AllocatedDuringLooping;
    //}

    BeginTicketMutex(GlobalMemoryMutex);
    Block->Prev = Sentinel->Prev;
    Block->Prev->Next = Block;
    Block->Next->Prev = Block;
    EndTicketMutex(GlobalMemoryMutex);

    platform_memory_block *PlatBlock = &Block->Block;
    return(PlatBlock);
}

internal void SokolFreeMemoryBlock(sokol_memory_block* Block)
{
    BeginTicketMutex(GlobalMemoryMutex);
    Block->Prev->Next = Block->Next;
    Block->Next->Prev = Block->Prev;
    EndTicketMutex(GlobalMemoryMutex);

    free(Block);
}

PLATFORM_DEALLOCATE_MEMORY(SokolDeallocateMemory)
{
    // TODO(hugo): Write Win32 and Linux specific code
    if(Block)
    {
        sokol_memory_block* SokolBlock = (sokol_memory_block *)Block;
        //if(Win32IsInLoop(&GlobalWin32State) && !(Win32Block->Block.Flags & PlatformMemory_NotRestored))
        //{
        //    Win32Block->LoopingFlags = Win32Mem_FreedDuringLooping;
        //}
        // else
        // {
        SokolFreeMemoryBlock(SokolBlock);
        // }
    }
}
#endif
