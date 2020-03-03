#if COMPILE_WINDOWS
#elif COMPILE_LINUX
#else
#error You must specify either COMPILE_WINDOWS=1 or COMPILE_LINUX=1 in the build script !
#endif

#define SOKOL_ASSERT(Expression) if(!(Expression)) {*(volatile int *)0 = 0;}
#define SOKOL_IMPL
#define SOKOL_GLCORE33
#include "sokol_app.h"
#include "sokol_gfx.h"

#include "sokol_time.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable: 4267)
#endif
#define FONS_STATIC
#define FONTSTASH_IMPLEMENTATION
#include "fontstash.h"
#ifdef _WIN32
#pragma warning(pop)
#endif

#define FILE_IMPLEMENTATION
#define DEVUI_IMPLEMENTATION
#define ALLOCATION_IMPLEMENTATION
#include "platform.h"
#include "intrinsics.h"
#include "math.h"
#include "shared.h"
#include "memory.h"

#ifdef COMPILE_WINDOWS
#define SOKOL_STATE_FILE_NAME_COUNT MAX_PATH
#elif COMPILE_LINUX
#define SOKOL_STATE_FILE_NAME_COUNT  512
#endif
#include "sokol_whitherthen.h"

#include "renderer.h"
#include "renderer.cpp"
#include "sokol_renderer.h"
#include "sokol_renderer.cpp"

#if COMPILE_WINDOWS
#elif COMPILE_LINUX
#include <sys/stat.h> // stat
#include <dlfcn.h> // dlopen, dlsym, dlclose
#include <fcntl.h>
#include <unistd.h>
#include <errno.h> // errno
#endif

global uint64_t LastTime = 0;
global bool ShowImgui = false;

global game_input GlobalInput[2] = {};
global game_input* NewInput = nullptr;
global game_input* OldInput = nullptr;
global game_memory GameMemory = {};
global platform_renderer* Renderer = nullptr;
global sokol_renderer_function_table RendererFunctions = {};
global sokol_game_function_table Game = {}; 

global sokol_state GlobalSokolState;

internal void SokolBuildEXEPathFileName(sokol_state *State, char *FileName, u32 Unique, int DestCount, char *Dest)
{
    string A =
    {
        (umm)(State->OnePastLastEXEFileNameSlash - State->EXEFileName),
        (u8 *)State->EXEFileName,
    };
    string B = WrapZ(FileName);
    if(Unique == 0)
    {
        FormatString(DestCount, Dest, "%S%S", A, B);
    }
    else
    {
        FormatString(DestCount, Dest, "%S%d_%S", A, Unique, B);
    }
}

internal void SokolBuildEXEPathFileName(sokol_state *State, char *FileName, int DestCount, char *Dest)
{
    SokolBuildEXEPathFileName(State, FileName, 0, DestCount, Dest);
}


internal void SokolGetEXEFileName(sokol_state *State)
{
#if COMPILE_WINDOWS
    // NOTE(casey): Never use MAX_PATH in code that is user-facing, because it
    // can be dangerous and lead to bad results.
    DWORD SizeOfFilename = GetModuleFileNameA(0, State->EXEFileName, sizeof(State->EXEFileName));
    State->OnePastLastEXEFileNameSlash = State->EXEFileName;
    for(char *Scan = State->EXEFileName;
        *Scan;
        ++Scan)
    {
        if(*Scan == '\\')
        {
            State->OnePastLastEXEFileNameSlash = Scan + 1;
        }
    }
#elif COMPILE_LINUX
    // NOTE(casey): Never use MAX_PATH in code that is user-facing, because it
    // can be dangerous and lead to bad results.
    ssize_t NumRead = readlink("/proc/self/exe", State->EXEFileName, ArrayCount(State->EXEFileName) - 1);
    if (NumRead > 0)
    {
        State->OnePastLastEXEFileNameSlash = State->EXEFileName;
        for(char *Scan = State->EXEFileName;
            *Scan;
            ++Scan)
        {
            if(*Scan == '/')
            {
                State->OnePastLastEXEFileNameSlash = Scan + 1;
            }
        }
    }
#endif
}

#include "dll_loading.h"

#define TEXTURE_COUNT 128
#define TEXTURE_TRANSFER_BUFFER_SIZE (128*1024*1024)

internal void SokolInit(void)
{
    sokol_state* State = &GlobalSokolState;
    State->MemorySentinel.Prev = &State->MemorySentinel;
    State->MemorySentinel.Next = &State->MemorySentinel;

	GlobalMemorySentinel = &State->MemorySentinel;
	GlobalMemoryMutex = &State->MemoryMutex;

    SokolGetEXEFileName(State);

#if COMPILE_WINDOWS
    char* GameDLLName = "whitherthen.dll";
#elif COMPILE_LINUX
    char* GameDLLName = "libWhitherthen.so";
#endif

    char SourceGameCodeDLLFullPath[SOKOL_STATE_FILE_NAME_COUNT];
    SokolBuildEXEPathFileName(State, GameDLLName,
                              sizeof(SourceGameCodeDLLFullPath), SourceGameCodeDLLFullPath);

#if COMPILE_WINDOWS
    char CodeLockFullPath[SOKOL_STATE_FILE_NAME_COUNT];
    SokolBuildEXEPathFileName(State, "lock.tmp",
                              sizeof(CodeLockFullPath), CodeLockFullPath);
#endif
    
#if 0
    char RendererCodeDLLFullPath[SOKOL_STATE_FILE_NAME_COUNT];
    SokolBuildEXEPathFileName(State, "libWhitherthenSokolGFX.so",
                              sizeof(RendererCodeDLLFullPath), RendererCodeDLLFullPath);
#endif
    

    NewInput = &GlobalInput[0];
    OldInput = &GlobalInput[1];

	u32 MaxQuadCountPerFrame = (1 << 20);
	platform_renderer_limits Limits = {};
	Limits.MaxQuadCountPerFrame = MaxQuadCountPerFrame;
    Limits.MaxTextureCount = TEXTURE_COUNT;
    Limits.TextureTransferBufferSize = TEXTURE_TRANSFER_BUFFER_SIZE;

#if 0
	sokol_loaded_code RendererCode = {};
	RendererCode.DLLFullPath = RendererCodeDLLFullPath;
	RendererCode.FunctionCount = ArrayCount(SokolRendererFunctionTableNames);
	RendererCode.FunctionNames = SokolRendererFunctionTableNames;
	RendererCode.Functions = (void **)&RendererFunctions;

	SokolLoadCode(State, &RendererCode);
	if(!RendererCode.IsValid)
	{
		Assert(false);
	}
#else
	RendererFunctions.LoadRenderer = &RendererInit;
	RendererFunctions.BeginFrame = &RendererBeginFrame;
	RendererFunctions.EndFrame = &RendererEndFrame;
    RendererFunctions.UnloadRenderer = &RendererDestroy;
#endif

	Renderer = RendererFunctions.LoadRenderer(&Limits);

	sokol_loaded_code GameCode = {};
	GameCode.DLLFullPath = SourceGameCodeDLLFullPath;
#if COMPILE_WINDOWS
    GameCode.TransientDLLName = "whitherthen_game_temp.dll";
    GameCode.LockFullPath = CodeLockFullPath;
#endif
	GameCode.FunctionCount = ArrayCount(SokolGameFunctionTableNames);
	GameCode.FunctionNames = SokolGameFunctionTableNames;
	GameCode.Functions = (void **)&Game;

	SokolLoadCode(State, &GameCode);

	stm_setup();

    GameMemory.PlatformAPI.OpenFile = SokolOpenFile;
    GameMemory.PlatformAPI.CloseFile = SokolCloseFile;
    GameMemory.PlatformAPI.ReadDataFromFile = SokolReadDataFromFile;
    GameMemory.PlatformAPI.AllocateMemory = SokolAllocateMemory;
    GameMemory.PlatformAPI.DeallocateMemory = SokolDeallocateMemory;

    GameMemory.TextureQueue = &Renderer->TextureQueue;
#ifdef COMPILE_INTERNAL
    GameMemory.PlatformAPI.Dev.CollapsingHeader = &DevUICollapsingHeader;
    GameMemory.PlatformAPI.Dev.SliderFloat2 = &DevUISliderFloat2;
    GameMemory.PlatformAPI.Dev.ColorEdit4 = &DevUIColorEdit4;
    GameMemory.PlatformAPI.Dev.Text = &DevUIText;
#endif
}

internal void SokolFrame(void)
{
	const double dt = stm_sec(stm_laptime(&LastTime));

    NewInput->dtForFrame = dt;

	if(NewInput->FKeyPressed[1])
	{
		ShowImgui = !ShowImgui;
	}

    game_render_commands* Frame = RendererFunctions.BeginFrame(Renderer, V2U(sapp_width(), sapp_height()), dt);

	if(Game.UpdateAndRender)
	{
		Game.UpdateAndRender(&GameMemory, NewInput, Frame);
		if(NewInput->QuitRequested)
		{
			sapp_quit();
		}
	}

	RendererFunctions.EndFrame(Renderer, Frame);

    game_input *Temp = NewInput;
    NewInput = OldInput;
    OldInput = Temp;

    // TODO(hugo)
    // Find out how to do this at the beginning of the frame
    // instead of the end. Because Sokol calls the event loop before the frame, so I cannot
    // clear the inputs at the beginning of the frame otherwise I would clean what
    // was just written...
    game_controller_input* OldKeyboardController = &OldInput->KeyboardController;
    game_controller_input* NewKeyboardController = &NewInput->KeyboardController;
    *NewKeyboardController = {};
    for(int ButtonIndex = 0;
            ButtonIndex < ArrayCount(NewKeyboardController->Buttons);
            ++ButtonIndex)
    {
        NewKeyboardController->Buttons[ButtonIndex].EndedDown =
            OldKeyboardController->Buttons[ButtonIndex].EndedDown;
    }
    for(u32 ButtonIndex = 0;
            ButtonIndex < PlatformMouseButton_Count;
            ++ButtonIndex)
    {
        NewInput->MouseButtons[ButtonIndex] = OldInput->MouseButtons[ButtonIndex];
        NewInput->MouseButtons[ButtonIndex].HalfTransitionCount = 0;
    }
	ZeroStruct(NewInput->FKeyPressed);
}

internal void SokolProcessKeyboardMessage(game_button_state* NewState, bool32 IsDown)
{
    if(NewState->EndedDown != IsDown)
    {
        NewState->EndedDown = IsDown;
        ++NewState->HalfTransitionCount;
    }
}

internal void SokolEvent(const sapp_event* Event)
{
	simgui_handle_event(Event);

	switch(Event->type)
	{
		case SAPP_EVENTTYPE_KEY_DOWN:
		case SAPP_EVENTTYPE_KEY_UP:
			{
                bool32 IsDown = Event->type == SAPP_EVENTTYPE_KEY_DOWN;
				if(!Event->key_repeat)
				{
					if(Event->key_code == SAPP_KEYCODE_LEFT)
					{
                        SokolProcessKeyboardMessage(&NewInput->KeyboardController.MoveLeft, IsDown);
					}
					else if(Event->key_code == SAPP_KEYCODE_RIGHT)
					{
                        SokolProcessKeyboardMessage(&NewInput->KeyboardController.MoveRight, IsDown);
					}
					else if(Event->key_code == SAPP_KEYCODE_UP)
					{
                        SokolProcessKeyboardMessage(&NewInput->KeyboardController.MoveUp, IsDown);
					}
					else if(Event->key_code == SAPP_KEYCODE_DOWN)
					{
                        SokolProcessKeyboardMessage(&NewInput->KeyboardController.MoveDown, IsDown);
					}
					else if(Event->key_code >= SAPP_KEYCODE_F1 && Event->key_code <= SAPP_KEYCODE_F12)
					{
						NewInput->FKeyPressed[Event->key_code + 1 - SAPP_KEYCODE_F1] = IsDown;
					}
				}
			} break;
        case SAPP_EVENTTYPE_MOUSE_MOVE:
            {
                f32 MouseX = Event->mouse_x;
                f32 MouseY = Event->mouse_y;
                NewInput->ClipSpaceMouseP.x = ClampBinormalMapToRange(0.0f, MouseX, sapp_width());
                NewInput->ClipSpaceMouseP.y = ClampBinormalMapToRange(0.0f, MouseY, sapp_height());
                NewInput->ClipSpaceMouseP.z = 0.0f;
            } break;
        case SAPP_EVENTTYPE_MOUSE_UP:
        case SAPP_EVENTTYPE_MOUSE_DOWN:
            {
                bool32 IsDown = Event->type == SAPP_EVENTTYPE_MOUSE_DOWN;
                SokolProcessKeyboardMessage(&NewInput->MouseButtons[Event->mouse_button], IsDown);

            } break;
		default:
			{
			} break;
	}
}

internal void SokolCleanup(void)
{
	RendererFunctions.UnloadRenderer(Renderer);
    Renderer = 0;
}

sapp_desc sokol_main(int ArgumentCount, char** Arguments)
{
	sapp_desc Desc = {};
	Desc.width = 640;
	Desc.height = 480;
	Desc.init_cb = SokolInit;
	Desc.cleanup_cb = SokolCleanup;
	Desc.event_cb = SokolEvent;
	Desc.frame_cb = SokolFrame;

	return(Desc);
}
