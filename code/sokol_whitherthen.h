
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

struct sokol_state
{
    ticket_mutex MemoryMutex;
    sokol_memory_block MemorySentinel;

    char EXEFileName[SOKOL_STATE_FILE_NAME_COUNT];
    char *OnePastLastEXEFileNameSlash;
};

