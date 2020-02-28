
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

#ifdef __cplusplus
}
#endif

#include "dev_ui.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "file.h"
#include "allocation.h"

typedef struct platform_api
{
    platform_get_all_files_of_type_begin *GetAllFilesOfTypeBegin;
    platform_get_all_files_of_type_end *GetAllFilesOfTypeEnd;
    platform_get_file_by_path *GetFileByPath;
    platform_open_file *OpenFile;
    platform_read_data_from_file *ReadDataFromFile;
    platform_write_data_to_file *WriteDataToFile;
    platform_atomic_replace_file_contents *AtomicReplaceFileContents;
    platform_file_error *FileError;
    platform_close_file *CloseFile;
        
    platform_allocate_memory *AllocateMemory;
    platform_deallocate_memory *DeallocateMemory;

#if COMPILE_INTERNAL
    dev_ui Dev;
#endif
} platform_api;

extern platform_api Platform;

typedef struct game_memory
{
    struct game_state *GameState;
    
    struct renderer_texture_queue* TextureQueue;

    b32 ExecutableReloaded;
    platform_api PlatformAPI;
} game_memory;

struct game_render_commands;
#define GAME_UPDATE_AND_RENDER(name) void name(game_memory *Memory, game_input *Input, game_render_commands *RenderCommands)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

//#define GAME_GET_SOUND_SAMPLES(name) void name(game_memory *Memory, game_sound_output_buffer *SoundBuffer)
//typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);

#ifdef __cplusplus
}
#endif
