
#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include "debug_interface.h"

typedef struct game_button_state
{
    int HalfTransitionCount;
    bool32 EndedDown;
} game_button_state;
    
typedef struct game_controller_input
{
    b32 IsConnected;
    b32 IsAnalog;
    f32 StickAverageX;
    f32 StickAverageY;
    f32 ClutchMax; // NOTE(casey): This is the "dodge" clutch, eg. triggers or space bar?
    
    union
    {
        game_button_state Buttons[12];
        struct
        {
            game_button_state MoveUp;
            game_button_state MoveDown;
            game_button_state MoveLeft;
            game_button_state MoveRight;
            
            game_button_state ActionUp;
            game_button_state ActionDown;
            game_button_state ActionLeft;
            game_button_state ActionRight;
            
            game_button_state LeftShoulder;
            game_button_state RightShoulder;
            
            game_button_state Back;
            game_button_state Start;
            
            // NOTE(casey): All buttons must be added above this line
            
            game_button_state Terminator;
        };
    };
} game_controller_input;
    
enum game_input_mouse_button
{
    PlatformMouseButton_Left,
    PlatformMouseButton_Middle,
    PlatformMouseButton_Right,
    PlatformMouseButton_Extended0,
    PlatformMouseButton_Extended1,
    
    PlatformMouseButton_Count,
};

typedef struct game_input
{
    r32 dtForFrame;
    
    game_controller_input KeyboardController;
    
    // NOTE(casey): Signals back to the platform layer
    b32 QuitRequested;
    
    // NOTE(casey): For debugging only
    game_button_state MouseButtons[PlatformMouseButton_Count];
    v3 ClipSpaceMouseP;
    b32 ShiftDown, AltDown, ControlDown;
    b32 FKeyPressed[13]; // NOTE(casey): 1 is F1, etc., for clarity - 0 is not used!
} game_input;

inline b32 WasPressed(game_button_state State)
{
    b32 Result = ((State.HalfTransitionCount > 1) ||
                  ((State.HalfTransitionCount == 1) && (State.EndedDown)));
    
    return(Result);
}

inline b32 IsDown(game_button_state State)
{
    b32 Result = (State.EndedDown);
    
    return(Result);
}

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
    
typedef struct platform_api
{
    platform_allocate_memory *AllocateMemory;
    platform_deallocate_memory *DeallocateMemory;
} platform_api;

extern platform_api Platform;

typedef struct game_memory
{
    struct game_state *GameState;
    
    b32 ExecutableReloaded;
    platform_api PlatformAPI;

#ifdef COMPILE_INTERNAL
	ImGuiContext* ImGuiContext;
#endif
} game_memory;

struct game_render_commands;
#define GAME_UPDATE_AND_RENDER(name) void name(game_memory *Memory, game_input *Input, game_render_commands *RenderCommands)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

//#define GAME_GET_SOUND_SAMPLES(name) void name(game_memory *Memory, game_sound_output_buffer *SoundBuffer)
//typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);

#ifdef __cplusplus
}
#endif
