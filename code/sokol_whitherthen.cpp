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

internal PLATFORM_FILE_ERROR(SokolFileError)
{
    Handle->NoErrors = false;
}

internal PLATFORM_OPEN_FILE(SokolOpenFile)
{
    platform_file_handle Result = {};
    FILE* fp = fopen(FileName, "rb");
    Result.NoErrors = (fp != 0);
    Result.Platform = fp;

    // NOTE(hugo): Here I diverge from HandmadeHero's implementation.
    // They get the FileInfo from previous call.
    // Here, the FileInfo is part of the result.
    if(fp)
    {
        fseek(fp, 0, SEEK_END);
        Info->FileSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);
    }

    return(Result);
}

internal PLATFORM_CLOSE_FILE(SokolCloseFile)
{
    FILE* SokolHandle = (FILE *)Handle->Platform;
    if(SokolHandle != 0)
    {
        fclose(SokolHandle);
    }
}

internal PLATFORM_READ_DATA_FROM_FILE(SokolReadDataFromFile)
{
    if(PlatformNoFileErrors(Handle))
    {
        FILE* SokolHandle = (FILE *)Handle->Platform;
        uint32 FileSize32 = SafeTruncateToU32(Size);
        size_t BytesRead = fread(Dest, 1, Size, SokolHandle);
        if(BytesRead == FileSize32)
        {
        }
        else
        {
            SokolFileError(Handle, "Read file failed.");
        }
    }
}

struct sokol_loaded_code
{
    b32x IsValid;
    char *DLLFullPath;
    
#ifdef COMPILE_WINDOWS
    u32 TempDLLNumber;
    char* LockFullPath;
    char* TransientDLLName;
    HMODULE DLL;
    FILETIME DLLLastWriteTime;
#elif COMPILE_LINUX
    void *DLL;
    ino_t DLLFileID;
#endif
    
    u32 FunctionCount;
    char **FunctionNames;
    void **Functions;
};

#if COMPILE_LINUX
internal inline ino_t
LinuxFileId(char *FileName)
{
    struct stat Attr = {};
    if (stat(FileName, &Attr))
    {
        Attr.st_ino = 0;
    }
    
    return Attr.st_ino;
}
#endif

internal void* SokolLoadFunction(void *DLLHandle, const char *Name)
{
#ifdef COMPILE_WINDOWS
	NotImplemented
#elif COMPILE_LINUX
    void *Symbol = dlsym(DLLHandle, Name);
    if (!Symbol)
    {
        fprintf(stderr, "dlsym failed: %s\n", dlerror());
    }
    // TODO(michiel): Check if lib with underscore exists?!
    return Symbol;
#endif
}

internal void* SokolLoadLibrary(const char *Name)
{
#if COMPILE_WINDOWS
	NotImplemented
#elif COMPILE_LINUX
    void *Handle = NULL;
    
    Handle = dlopen(Name, RTLD_NOW | RTLD_LOCAL);
    if (!Handle)
    {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
    }
    return Handle;
#endif
}

internal void SokolUnloadLibrary(void *Handle)
{
#if COMPILE_WINDOWS
	NotImplemented
#elif COMPILE_LINUX
    if (Handle != NULL)
    {
        dlclose(Handle);
        Handle = NULL;
    }
#endif
}

internal void SokolUnloadCode(sokol_loaded_code *Loaded)
{
#if COMPILE_WINDOWS
    if(Loaded->DLL)
    {
        // TODO(casey): Currently, we never unload libraries, because
        // we may still be pointing to strings that are inside them
        // (despite our best efforts).  Should we just make "never unload"
        // be the policy?
        
        // FreeLibrary(GameCode->GameCodeDLL);
        Loaded->DLL = 0;
    }
    
    Loaded->IsValid = false;
    ZeroArray(Loaded->FunctionCount, Loaded->Functions);
#elif COMPILE_LINUX
    if (Loaded->DLL)
    {
        // TODO(casey): Currently, we never unload libraries, because
        // we may still be pointing to strings that are inside them
        // (despite our best efforts).  Should we just make "never unload"
        // be the policy?
        //LinuxUnloadLibrary(Loaded->Library);
        Loaded->DLL = 0;
    }
    Loaded->DLLFileID = 0;
    Loaded->IsValid = false;
    ZeroArray(Loaded->FunctionCount, Loaded->Functions);
#endif
}

#if COMPILE_WINDOWS
inline FILETIME SokolGetLastWriteTime(char *Filename)
{
    FILETIME LastWriteTime = {};
    
    WIN32_FILE_ATTRIBUTE_DATA Data;
    if(GetFileAttributesExA(Filename, GetFileExInfoStandard, &Data))
    {
        LastWriteTime = Data.ftLastWriteTime;
    }
    
    return(LastWriteTime);
}
#endif

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

internal void SokolLoadCode(sokol_state* State, sokol_loaded_code *Loaded)
{
#if COMPILE_WINDOWS
    char *SourceDLLName = Loaded->DLLFullPath;
    char *LockFileName = Loaded->LockFullPath;
    
    char TempDLLName[SOKOL_STATE_FILE_NAME_COUNT];
    
    WIN32_FILE_ATTRIBUTE_DATA Ignored;
    if(!GetFileAttributesExA(LockFileName, GetFileExInfoStandard, &Ignored))
    {
        Loaded->DLLLastWriteTime = SokolGetLastWriteTime(SourceDLLName);
        
        for(u32 AttemptIndex = 0;
            AttemptIndex < 128;
            ++AttemptIndex)
        {
            SokolBuildEXEPathFileName(State, Loaded->TransientDLLName, Loaded->TempDLLNumber,
                                      sizeof(TempDLLName), TempDLLName);
            if(++Loaded->TempDLLNumber >= 1024)
            {
                Loaded->TempDLLNumber = 0;
            }
            
            if(CopyFile(SourceDLLName, TempDLLName, FALSE))
            {
                break;
            }
        }
        
        Loaded->DLL = LoadLibraryA(TempDLLName);
        if(Loaded->DLL)
        {
            Loaded->IsValid = true;
            for(u32 FunctionIndex = 0;
                FunctionIndex < Loaded->FunctionCount;
                ++FunctionIndex)
            {
                void *Function = GetProcAddress(Loaded->DLL, Loaded->FunctionNames[FunctionIndex]);
                if(Function)
                {
                    Loaded->Functions[FunctionIndex] = Function;
                }
                else
                {
                    Loaded->IsValid = false;
                }
            }
        }
    }
    
    if(!Loaded->IsValid)
    {
        SokolUnloadCode(Loaded);
    }
#elif COMPILE_LINUX
   ino_t FileID = LinuxFileId(Loaded->DLLFullPath);
    
    if (Loaded->DLLFileID != FileID)
    {
        // NOTE(michiel): Create temp file, copy the library and load.
        // dlopen uses a caching mechanism based on the library name.
        char TempFileName[SOKOL_STATE_FILE_NAME_COUNT];
        FormatString(SOKOL_STATE_FILE_NAME_COUNT, TempFileName, "%sXXXXXX",
                     Loaded->DLLFullPath);
        s32 FD = mkstemp(TempFileName);
        s32 OrigFile = open(Loaded->DLLFullPath, O_RDONLY);
        
        if ((FD >=0) && (OrigFile >= 0))
        {
            char ReadBuffer[4096];
            ssize_t ReadCount = read(OrigFile, ReadBuffer, sizeof(ReadBuffer));
            
            while (ReadCount > 0)
            {
                char *WriteBuffer = ReadBuffer;
                ssize_t WriteCount;
                do {
                    WriteCount = write(FD, WriteBuffer, ReadCount);
                    
                    if (WriteCount >= 0)
                    {
                        ReadCount -= WriteCount;
                        WriteBuffer += WriteCount;
                    }
                    else if (errno != EINTR)
                    {
                        Assert(!"Could not copy shared library while loading.");
                    }
                } while (ReadCount > 0);
                ReadCount = read(OrigFile, ReadBuffer, sizeof(ReadBuffer));
            }
            if (ReadCount == 0)
            {
                close(OrigFile);
                close(FD);
            }
            else
            {
                Assert(!"Could not copy whole shared library while loading.");
            }
        }
        else
        {
            Assert(!"Could not open shared library for copying.");
        }
        
        //LinuxUnloadLibrary(Loaded->Library);
        Loaded->DLLFileID = FileID;
        Loaded->IsValid = false;
        
        Loaded->DLL = SokolLoadLibrary(TempFileName);
        if (Loaded->DLL)
        {
            Loaded->IsValid = true;
            for (u32 FunctionIndex = 0;
                 FunctionIndex < Loaded->FunctionCount;
                 ++FunctionIndex)
            {
                void *Function = SokolLoadFunction(Loaded->DLL, Loaded->FunctionNames[FunctionIndex]);
                if (Function)
                {
                    Loaded->Functions[FunctionIndex] = Function;
                }
                else
                {
                    Loaded->IsValid = false;
                }
            }
        }
    }
    
    if(!Loaded->IsValid)
    {
        SokolUnloadCode(Loaded);
    }
#endif
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

#define TEXTURE_COUNT 128
#define TEXTURE_TRANSFER_BUFFER_SIZE (128*1024*1024)

internal void SokolInit(void)
{
    sokol_state* State = &GlobalSokolState;
    State->MemorySentinel.Prev = &State->MemorySentinel;
    State->MemorySentinel.Next = &State->MemorySentinel;

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
	RendererFunctions.LoadRenderer = &SokolInitGFX;
	RendererFunctions.BeginFrame = &RendererBeginFrame;
	RendererFunctions.EndFrame = &RendererEndFrame;
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
	//GameMemory.ImGuiContext = ImGui::GetCurrentContext();
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
	simgui_shutdown();
	RendererDestroy((sokol_gfx *)Renderer);
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
