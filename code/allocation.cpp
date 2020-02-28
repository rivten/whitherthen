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

    sokol_memory_block* Sentinel = &GlobalSokolState.MemorySentinel;
    Block->Next = Sentinel;
    Block->Block.Size = Size;
    Block->Block.Flags = Flags;
    Block->LoopingFlags = 0;
    //if(Win32IsInLoop(&GlobalWin32State) && !(Flags & PlatformMemory_NotRestored))
    //{
        //Block->LoopingFlags = Win32Mem_AllocatedDuringLooping;
    //}

    BeginTicketMutex(&GlobalSokolState.MemoryMutex);
    Block->Prev = Sentinel->Prev;
    Block->Prev->Next = Block;
    Block->Next->Prev = Block;
    EndTicketMutex(&GlobalSokolState.MemoryMutex);

    platform_memory_block *PlatBlock = &Block->Block;
    return(PlatBlock);
}

internal void SokolFreeMemoryBlock(sokol_memory_block* Block)
{
    BeginTicketMutex(&GlobalSokolState.MemoryMutex);
    Block->Prev->Next = Block->Next;
    Block->Next->Prev = Block->Prev;
    EndTicketMutex(&GlobalSokolState.MemoryMutex);

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
