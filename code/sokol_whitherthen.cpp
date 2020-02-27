#define SOKOL_ASSERT(Expression) if(!(Expression)) {*(volatile int *)0 = 0;}
#define SOKOL_IMPL
#define SOKOL_GLCORE33
#include "sokol_app.h"
#include "sokol_gfx.h"

#include "sokol_fetch.h"
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

#define SOKOL_GL_IMPL
#include "sokol_gl.h"
#define SOKOL_FONTSTASH_IMPL
#include "sokol_fontstash.h"

#include "imgui.cpp"
#undef STB_TRUETYPE_IMPLEMENTATION
#undef __STB_INCLUDE_STB_TRUETYPE_H__
#include "imgui_draw.cpp"
#include "imgui_widgets.cpp"
#include "imgui_demo.cpp"
#define SOKOL_IMGUI_IMPL
#include "sokol_imgui.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "platform.h"
#include "intrinsics.h"
#include "math.h"
#include "shared.h"

#include "memory.h"

global uint64_t LastTime = 0;
global bool ShowImgui = false;

#include "renderer.cpp"

global game_input GlobalInput[2] = {};
global game_input* NewInput = nullptr;
global game_input* OldInput = nullptr;
global game_memory GameMemory = {};
global platform_renderer* Renderer = nullptr;

internal void FetchCallback(const sfetch_response_t* Response)
{
	if(Response->fetched)
	{
		int PngWidth = 0;
		int PngHeight = 0;
		int NumChannels = 0;
		const int DesiredChannels = 4;
		stbi_uc* Pixels = stbi_load_from_memory((stbi_uc* const)Response->buffer_ptr, (int)Response->fetched_size, &PngWidth, &PngHeight, &NumChannels, DesiredChannels);
		if(Pixels)
		{
			sg_image_desc ImageDesc = {};
			ImageDesc.width = PngWidth;
			ImageDesc.height = PngHeight;
			ImageDesc.layers = 1;
            ImageDesc.type = SG_IMAGETYPE_ARRAY;
			ImageDesc.pixel_format = SG_PIXELFORMAT_RGBA8;
			ImageDesc.min_filter = SG_FILTER_LINEAR;
			ImageDesc.mag_filter = SG_FILTER_LINEAR;
			ImageDesc.content.subimage[0][0].ptr = Pixels;
			ImageDesc.content.subimage[0][0].size = PngWidth * PngHeight * DesiredChannels;
			// TODO(hugo): fix this hack
			sokol_gfx* GFX = (sokol_gfx *)Renderer;
			sg_init_image(GFX->Bindings.fs_images[0], &ImageDesc);
			stbi_image_free(Pixels);
		}
	}
}

#define TILE_SIZE_IN_PIXELS 16

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
};

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

#define TEXTURE_COUNT 128
#define TEXTURE_TRANSFER_BUFFER_SIZE (128*1024*1024)

internal void SokolInit(void)
{
    sokol_state* State = &GlobalSokolState;
    State->MemorySentinel.Prev = &State->MemorySentinel;
    State->MemorySentinel.Next = &State->MemorySentinel;

    NewInput = &GlobalInput[0];
    OldInput = &GlobalInput[1];

	sfetch_desc_t FetchDesc = {};
	FetchDesc.max_requests = 1;
	FetchDesc.num_channels = 1;
	FetchDesc.num_lanes = 1;
	sfetch_setup(&FetchDesc);

	u32 MaxQuadCountPerFrame = (1 << 20);
	platform_renderer_limits Limits = {};
	Limits.MaxQuadCountPerFrame = MaxQuadCountPerFrame;
    Limits.MaxTextureCount = TEXTURE_COUNT;
    Limits.TextureTransferBufferSize = TEXTURE_TRANSFER_BUFFER_SIZE;
	Renderer = (platform_renderer *)SokolInitGFX(&Limits);

	stm_setup();

	simgui_desc_t ImguiDesc = {};
	simgui_setup(&ImguiDesc);

#if 0
	sfetch_request_t FetchRequest = {};
	FetchRequest.path = "../data/roguelike_tileset.png";
	FetchRequest.callback = FetchCallback;
	FetchRequest.buffer_ptr = FileBuffer;
	FetchRequest.buffer_size = sizeof(FileBuffer);
	sfetch_send(&FetchRequest);
#endif

    GameMemory.PlatformAPI.OpenFile = SokolOpenFile;
    GameMemory.PlatformAPI.CloseFile = SokolCloseFile;
    GameMemory.PlatformAPI.ReadDataFromFile = SokolReadDataFromFile;
    GameMemory.PlatformAPI.AllocateMemory = SokolAllocateMemory;
    GameMemory.PlatformAPI.DeallocateMemory = SokolDeallocateMemory;

    GameMemory.TextureQueue = &Renderer->TextureQueue;
#ifdef COMPILE_INTERNAL
	GameMemory.ImGuiContext = ImGui::GetCurrentContext();
#endif
}

#include "whitherthen.cpp"

internal void SokolFrame(void)
{
	const double dt = stm_sec(stm_laptime(&LastTime));
	simgui_new_frame(sapp_width(), sapp_height(), dt);

    NewInput->dtForFrame = dt;

	if(NewInput->FKeyPressed[1])
	{
		ShowImgui = !ShowImgui;
	}

	sfetch_dowork();

    game_render_commands* Frame = RendererBeginFrame((sokol_gfx *)Renderer, V2U(sapp_width(), sapp_height()));
    GameUpdateAndRender(&GameMemory, NewInput, Frame);
    if(NewInput->QuitRequested)
    {
        sapp_quit();
    }

	if(ShowImgui)
    {
        //ImGui::ShowDemoWindow();
        ImGui::Text("dt = %.4f", dt);

#if 0
		ImGui::Text("Vertex Count : %u", Frame->VertexCount);
		ImGui::Text("Index Count : %u", Frame->IndexCount);
		for(u32 VertexIndex = 0; VertexIndex < Frame->VertexCount; ++VertexIndex)
		{
			textured_vertex* V = Frame->VertexArray + VertexIndex;
			ImGui::Text("Vertex : P(%f, %f, %f, %f), UV(%f, %f), C(%u)", V->P.x, V->P.y, V->P.z, V->P.w, V->UV.x, V->UV.y, V->Color);
		}
		ImGui::Text("%u", sg_query_image_info(((sokol_gfx *)Renderer)->Bindings.fs_images[0]).slot.state);
#endif
#if 0
		for(u32 IndexIndex = 0; IndexIndex < Frame->IndexCount; ++IndexIndex)
		{
			ImGui::Text("Index : %u", Frame->IndexArray[IndexIndex]);
		}
#endif
        for(u32 CommandIndex = 0; CommandIndex < Frame->RenderTextCommandCount; ++CommandIndex)
        {
            render_text_command* Command = Frame->RenderTextCommands + CommandIndex;
            ImGui::Text("Size : %.2f", Command->Size);
        }
    }

	RendererEndFrame((sokol_gfx *)Renderer, Frame);

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
