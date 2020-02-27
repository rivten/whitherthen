
struct sokol_game_function_table
{
    // IMPORTANT(casey): The  callbacks can be 0!  You must check before calling.
    // (or check the IsValid boolean in sokol_loaded_code)
    game_update_and_render *UpdateAndRender;
    //game_get_sound_samples *GetSoundSamples;
};
global char *SokolGameFunctionTableNames[] =
{
    "GameUpdateAndRender",
    //"GameGetSoundSamples",
};

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

struct sokol_state
{
    ticket_mutex MemoryMutex;
    sokol_memory_block MemorySentinel;

    char EXEFileName[SOKOL_STATE_FILE_NAME_COUNT];
    char *OnePastLastEXEFileNameSlash;
};

