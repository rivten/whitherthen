
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

typedef struct platform_file_handle
{
    b32 NoErrors;
    void *Platform;
} platform_file_handle;

typedef struct platform_file_info
{
    platform_file_info *Next;
    u64 FileDate; // NOTE(casey): This is a 64-bit number that _means_ the date to the platform, but doesn't have to be understood by the app as any particular date.
    u64 FileSize;
    char *BaseName; // NOTE(casey): Doesn't include a path or an extension
    void *Platform;
} platform_file_info;
typedef struct platform_file_group
{
    u32 FileCount;
    platform_file_info *FirstFileInfo;
    void *Platform;
} platform_file_group;

typedef enum platform_file_type
{
    PlatformFileType_Count,
} platform_file_type;
    
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

#define PLATFORM_GET_ALL_FILE_OF_TYPE_BEGIN(name) platform_file_group name(platform_file_type Type)
typedef PLATFORM_GET_ALL_FILE_OF_TYPE_BEGIN(platform_get_all_files_of_type_begin);
    
#define PLATFORM_GET_ALL_FILE_OF_TYPE_END(name) void name(platform_file_group *FileGroup)
typedef PLATFORM_GET_ALL_FILE_OF_TYPE_END(platform_get_all_files_of_type_end);
    
enum platform_open_file_mode_flags
{
    OpenFile_Read = 0x1,
    OpenFile_Write = 0x2,
};
//#define PLATFORM_OPEN_FILE(name) platform_file_handle name(platform_file_group *FileGroup, platform_file_info *Info, u32 ModeFlags)
//typedef PLATFORM_OPEN_FILE(platform_open_file);
#define PLATFORM_OPEN_FILE(name) platform_file_handle name(char* FileName, platform_file_info *Info)
typedef PLATFORM_OPEN_FILE(platform_open_file);
    
#define PLATFORM_GET_FILE_BY_PATH(name) platform_file_info *name(platform_file_group *FileGroup, char *Path, u32 ModeFlags)
typedef PLATFORM_GET_FILE_BY_PATH(platform_get_file_by_path);
    
#define PLATFORM_READ_DATA_FROM_FILE(name) void name(platform_file_handle *Handle, u64 Offset, u64 Size, void *Dest)
typedef PLATFORM_READ_DATA_FROM_FILE(platform_read_data_from_file);
    
#define PLATFORM_WRITE_DATA_TO_FILE(name) void name(platform_file_handle *Handle, u64 Offset, u64 Size, void *Source)
typedef PLATFORM_WRITE_DATA_TO_FILE(platform_write_data_to_file);
    
#define PLATFORM_ATOMIC_REPLACE_FILE_CONTENTS(name) b32 name(platform_file_info *Info, u64 Size, void *Source)
typedef PLATFORM_ATOMIC_REPLACE_FILE_CONTENTS(platform_atomic_replace_file_contents);
    
#define PLATFORM_FILE_ERROR(name) void name(platform_file_handle *Handle, char *Message)
typedef PLATFORM_FILE_ERROR(platform_file_error);
    
#define PLATFORM_CLOSE_FILE(name) void name(platform_file_handle *Handle)
typedef PLATFORM_CLOSE_FILE(platform_close_file);
    
#define PlatformNoFileErrors(Handle) ((Handle)->NoErrors)
    
#define PLATFORM_ALLOCATE_MEMORY(name) platform_memory_block *name(memory_index Size, u64 Flags)
typedef PLATFORM_ALLOCATE_MEMORY(platform_allocate_memory);
    
#define PLATFORM_DEALLOCATE_MEMORY(name) void name(platform_memory_block *Block)
typedef PLATFORM_DEALLOCATE_MEMORY(platform_deallocate_memory);
    
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
} platform_api;

extern platform_api Platform;

typedef struct game_memory
{
    struct game_state *GameState;
    
    struct renderer_texture_queue* TextureQueue;

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
