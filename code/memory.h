
global platform_allocate_memory* MemAllocateMemory;
global platform_deallocate_memory* MemDeallocateMemory;

struct memory_arena
{
    // TODO(casey): If we see perf problems here, maybe move Used/Base/Size out?
    platform_memory_block *CurrentBlock;
    umm MinimumBlockSize;
    
    u64 AllocationFlags;
    s32 TempCount;
};

struct temporary_memory
{
    memory_arena *Arena;
    platform_memory_block *Block;
    umm Used;
};

inline void
SetMinimumBlockSize(memory_arena *Arena, memory_index MinimumBlockSize)
{
    Arena->MinimumBlockSize = MinimumBlockSize;
}

inline memory_index
GetAlignmentOffset(memory_arena *Arena, memory_index Alignment)
{
    memory_index AlignmentOffset = 0;
    
    memory_index ResultPointer = (memory_index)Arena->CurrentBlock->Base + Arena->CurrentBlock->Used;
    memory_index AlignmentMask = Alignment - 1;
    if(ResultPointer & AlignmentMask)
    {
        AlignmentOffset = Alignment - (ResultPointer & AlignmentMask);
    }
    
    return(AlignmentOffset);
}

enum arena_push_flag
{
    ArenaFlag_ClearToZero = 0x1,
};
struct arena_push_params
{
    u32 Flags;
    u32 Alignment;
};

inline arena_push_params
DefaultArenaParams(void)
{
    arena_push_params Params;
    Params.Flags = ArenaFlag_ClearToZero;
    Params.Alignment = 4;
    return(Params);
}

inline arena_push_params
AlignNoClear(u32 Alignment)
{
    arena_push_params Params = DefaultArenaParams();
    Params.Flags &= ~ArenaFlag_ClearToZero;
    Params.Alignment = Alignment;
    return(Params);
}

inline arena_push_params
Align(u32 Alignment, b32 Clear)
{
    arena_push_params Params = DefaultArenaParams();
    if(Clear)
    {
        Params.Flags |= ArenaFlag_ClearToZero;
    }
    else
    {
        Params.Flags &= ~ArenaFlag_ClearToZero;
    }
    Params.Alignment = Alignment;
    return(Params);
}

inline arena_push_params
NoClear(void)
{
    arena_push_params Params = DefaultArenaParams();
    Params.Flags &= ~ArenaFlag_ClearToZero;
    return(Params);
}

struct arena_bootstrap_params
{
    u64 AllocationFlags;
    umm MinimumBlockSize;
};

inline arena_bootstrap_params
DefaultBootstrapParams(void)
{
    arena_bootstrap_params Params = {};
    return(Params);
}

inline arena_bootstrap_params
NonRestoredArena(void)
{
    arena_bootstrap_params Params = DefaultBootstrapParams();
    Params.AllocationFlags = PlatformMemory_NotRestored;
    return(Params);
}

#if COMPILE_INTERNAL
#define DEBUG_MEMORY_NAME(Name) DEBUG_NAME_(__FILE__, __LINE__, __COUNTER__),
#define INTERNAL_MEMORY_PARAM char *GUID,
#define INTERNAL_MEMORY_PASS GUID,
#else
#define DEBUG_MEMORY_NAME(Name)
#define INTERNAL_MEMORY_PARAM 
#define INTERNAL_MEMORY_PASS 
#endif

#define PushStruct(Arena, type, ...) (type *)PushSize_(DEBUG_MEMORY_NAME("PushStruct") Arena, sizeof(type), ## __VA_ARGS__)
#define PushArray(Arena, Count, type, ...) (type *)PushSize_(DEBUG_MEMORY_NAME("PushArray") Arena, (Count)*sizeof(type), ## __VA_ARGS__)
#define PushSize(Arena, Size, ...) PushSize_(DEBUG_MEMORY_NAME("PushSize") Arena, Size, ## __VA_ARGS__)
#define PushCopy(...) PushCopy_(DEBUG_MEMORY_NAME("PushCopy") __VA_ARGS__)
#define PushStringZ(...) PushStringZ_(DEBUG_MEMORY_NAME("PushStringZ") __VA_ARGS__)
#define PushString(...) PushString_(DEBUG_MEMORY_NAME("PushString") __VA_ARGS__)
#define PushBuffer(...) PushBuffer_(DEBUG_MEMORY_NAME("PushBuffer") __VA_ARGS__)
#define PushAndNullTerminate(...) PushAndNullTerminate_(DEBUG_MEMORY_NAME("PushAndNullTerminate") __VA_ARGS__)
#define BootstrapPushStruct(type, Member, ...) (type *)BootstrapPushSize_(DEBUG_MEMORY_NAME("BootstrapPushSize") sizeof(type), OffsetOf(type, Member), ## __VA_ARGS__)

inline memory_index
GetEffectiveSizeFor(memory_arena *Arena, memory_index SizeInit, arena_push_params Params = DefaultArenaParams())
{
    memory_index Size = SizeInit;
    
    memory_index AlignmentOffset = GetAlignmentOffset(Arena, Params.Alignment);
    Size += AlignmentOffset;
    
    return(Size);
}

inline void *
PushSize_(INTERNAL_MEMORY_PARAM
          memory_arena *Arena, memory_index SizeInit, arena_push_params Params = DefaultArenaParams())
{
    void *Result = 0;
    
    Assert(Params.Alignment <= 128);
    Assert(IsPow2(Params.Alignment));
    
    memory_index Size = 0;
    if(Arena->CurrentBlock)
    {
        Size = GetEffectiveSizeFor(Arena, SizeInit, Params);
    }
    
    if(!Arena->CurrentBlock ||
       ((Arena->CurrentBlock->Used + Size) > Arena->CurrentBlock->Size))
    {
        Size = SizeInit; // NOTE(casey): The base will automatically be aligned now!
        if(Arena->AllocationFlags & (PlatformMemory_OverflowCheck|
                                     PlatformMemory_UnderflowCheck))
        {
            Arena->MinimumBlockSize = 0;
            Size = AlignPow2(Size, Params.Alignment);
        }
        else if(!Arena->MinimumBlockSize)
        {
            // TODO(casey): Tune default block size eventually?
            Arena->MinimumBlockSize = 1024*1024;
        }
        
        memory_index BlockSize = Maximum(Size, Arena->MinimumBlockSize);
        
        platform_memory_block *NewBlock = 
            MemAllocateMemory(BlockSize, Arena->AllocationFlags);
        NewBlock->ArenaPrev = Arena->CurrentBlock;
        Arena->CurrentBlock = NewBlock;
        //DEBUG_RECORD_BLOCK_ALLOCATION(Arena->CurrentBlock);
    }    
    
    Assert((Arena->CurrentBlock->Used + Size) <= Arena->CurrentBlock->Size);
    
    memory_index AlignmentOffset = GetAlignmentOffset(Arena, Params.Alignment);
    umm OffsetInBlock = Arena->CurrentBlock->Used + AlignmentOffset;
    Result = Arena->CurrentBlock->Base + OffsetInBlock;
    Arena->CurrentBlock->Used += Size;
    
    Assert(Size >= SizeInit);
    
    // NOTE(casey): This is just to guarantee that nobody passed in an alignment
    // on their first allocation that was _greater_ that than the page alignment
    Assert(Arena->CurrentBlock->Used <= Arena->CurrentBlock->Size);
    
    if(Params.Flags & ArenaFlag_ClearToZero)
    {
        ZeroSize(SizeInit, Result);
    }
    
    //DEBUG_RECORD_ALLOCATION(Arena->CurrentBlock, GUID, Size, SizeInit, OffsetInBlock);
    
    return(Result);
}

internal void *
PushCopy_(INTERNAL_MEMORY_PARAM
          memory_arena *Arena, umm Size, void *Source, arena_push_params Params = DefaultArenaParams())
{
    void *Result = PushSize_(INTERNAL_MEMORY_PASS Arena, Size, Params);
    Copy(Size, Source, Result);
    return(Result);
}

// NOTE(casey): This is generally not for production use, this is probably
// only really something we need during testing, but who knows
inline char *
PushStringZ_(INTERNAL_MEMORY_PARAM
             memory_arena *Arena, char *Source)
{
    u32 Size = 1;
    for(char *At = Source;
        *At;
        ++At)
    {
        ++Size;
    }
    
    char *Dest = (char *)PushSize_(INTERNAL_MEMORY_PASS Arena, Size, NoClear());
    for(u32 CharIndex = 0;
        CharIndex < Size;
        ++CharIndex)
    {
        Dest[CharIndex] = Source[CharIndex];
    }
    
    return(Dest);
}

internal buffer
PushBuffer_(INTERNAL_MEMORY_PARAM
            memory_arena *Arena, umm Size, arena_push_params Params = DefaultArenaParams())
{
    buffer Result;
    Result.Count = Size;
    Result.Data = (u8 *)PushSize_(INTERNAL_MEMORY_PASS Arena, Result.Count, Params);
    
    return(Result);
}

internal string
PushString_(INTERNAL_MEMORY_PARAM
            memory_arena *Arena, char *Source)
{
    string Result;
    Result.Count = StringLength(Source);
    Result.Data = (u8 *)PushCopy_(INTERNAL_MEMORY_PASS Arena, Result.Count, Source);
    
    return(Result);
}

internal string
PushString_(INTERNAL_MEMORY_PARAM
            memory_arena *Arena, string Source)
{
    string Result;
    Result.Count = Source.Count;
    Result.Data = (u8 *)PushCopy_(INTERNAL_MEMORY_PASS Arena, Result.Count, Source.Data);
    
    return(Result);
}

inline char *
PushAndNullTerminate_(INTERNAL_MEMORY_PARAM
                      memory_arena *Arena, u32 Length, char *Source)
{
    char *Dest = (char *)PushSize_(INTERNAL_MEMORY_PASS Arena, Length + 1, NoClear());
    for(u32 CharIndex = 0;
        CharIndex < Length;
        ++CharIndex)
    {
        Dest[CharIndex] = Source[CharIndex];
    }
    Dest[Length] = 0;
    
    return(Dest);
}

inline temporary_memory
BeginTemporaryMemory(memory_arena *Arena)
{
    temporary_memory Result;
    
    Result.Arena = Arena;
    Result.Block = Arena->CurrentBlock;
    Result.Used = Arena->CurrentBlock ? Arena->CurrentBlock->Used : 0;
    
    ++Arena->TempCount;
    
    return(Result);
}

inline void
FreeLastBlock(memory_arena *Arena)
{
    platform_memory_block *Free = Arena->CurrentBlock;
    //DEBUG_RECORD_BLOCK_FREE(Free);
    Arena->CurrentBlock = Free->ArenaPrev;
    MemDeallocateMemory(Free);
}

inline void
EndTemporaryMemory(temporary_memory TempMem)
{
    memory_arena *Arena = TempMem.Arena;
    while(Arena->CurrentBlock != TempMem.Block)
    {
        FreeLastBlock(Arena);
    }
    
    if(Arena->CurrentBlock)
    {
        Assert(Arena->CurrentBlock->Used >= TempMem.Used);
        Arena->CurrentBlock->Used = TempMem.Used;
        //DEBUG_RECORD_BLOCK_TRUNCATE(Arena->CurrentBlock);
    }
    
    Assert(Arena->TempCount > 0);
    --Arena->TempCount;
}

inline void
KeepTemporaryMemory(temporary_memory TempMem)
{
    memory_arena *Arena = TempMem.Arena;
    
    Assert(Arena->TempCount > 0);
    --Arena->TempCount;
}

inline void
Clear(memory_arena *Arena)
{
    while(Arena->CurrentBlock)
    {
        // NOTE(casey): Because the arena itself may be stored in the last block,
        // we must ensure that we don't look at it after freeing.
        b32 ThisIsLastBlock = (Arena->CurrentBlock->ArenaPrev == 0);
        FreeLastBlock(Arena);
        if(ThisIsLastBlock)
        {
            break;
        }
    }
}

inline void
CheckArena(memory_arena *Arena)
{
    Assert(Arena->TempCount == 0);
}

inline void *
BootstrapPushSize_(INTERNAL_MEMORY_PARAM umm StructSize, umm OffsetToArena,
                   arena_bootstrap_params BootstrapParams = DefaultBootstrapParams(), 
                   arena_push_params Params = DefaultArenaParams())
{
    memory_arena Bootstrap = {};
    Bootstrap.AllocationFlags = BootstrapParams.AllocationFlags;
    Bootstrap.MinimumBlockSize = BootstrapParams.MinimumBlockSize;
    void *Struct = PushSize_(INTERNAL_MEMORY_PASS &Bootstrap, StructSize, Params);
    *(memory_arena *)((u8 *)Struct + OffsetToArena) = Bootstrap;
    
    return(Struct);
}
