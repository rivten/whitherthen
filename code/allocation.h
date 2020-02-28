
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

