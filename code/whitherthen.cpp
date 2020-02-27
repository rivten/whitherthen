
#include "whitherthen.h"
#include "color_palette.h"

#include "renderer.cpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

enum entity_type
{
	EntityType_Player,
	EntityType_Wall,
	EntityType_Ground,

	EntityType_Count,
};

global v2 TileSetPosFromType[]
{
	V2(0, 13), // EntityType_Player,
	V2(15, 0), // EntityType_Wall,
	V2(8, 0), // EntityType_Ground,
};

enum entity_flag
{
	EntityFlag_None = 0,
	EntityFlag_RoomConnection = 1 << 0,
};

struct entity
{
	v2 P;
	v2 TileSetP;
	v4 Color;
	entity_type Type;
	u32 Flags;
};

struct game_state
{
    memory_arena TotalArena;

    v2 PlayerPos;
	f32 Timer;

	u32 EntityCount;
	entity Entities[256];

	f32 JustSteppedOnConnectionTimer;
    f32 TextSize;
    renderer_texture TileSetTexture;
};

void PushEntity(game_state* GameState, v2 P, v4 C, entity_type Type, u32 Flags = EntityFlag_None)
{
	Assert(GameState->EntityCount < ArrayCount(GameState->Entities));
	entity* Entity = GameState->Entities + GameState->EntityCount++;
	Entity->P = P;
	Entity->Color = C;
	Entity->Type = Type;
	// NOTE(hugo): Do I need to store this ? Or could I compute it each render iteration ?
	Entity->TileSetP = TileSetPosFromType[Type];
	Entity->Flags = Flags;
}

void PushPlayer(game_state* GameState, v2 P, v4 C)
{
	PushEntity(GameState, P, C, EntityType_Player);
}

void PushWall(game_state* GameState, v2 P, v4 C)
{
	PushEntity(GameState, P, C, EntityType_Wall);
}

void PushGround(game_state* GameState, v2 P, v4 C, u32 Flags = EntityFlag_None)
{
	PushEntity(GameState, P, C, EntityType_Ground, Flags);
}

enum room_flag
{
	RoomFlag_None = 0,
	RoomFlag_LeftHole = 1 << 0,
	RoomFlag_RightHole = 1 << 1,
};

void CreateRoom(game_state* GameState, rectangle2i Rect, v4 WallC, v4 GroundC, u32 Flags = RoomFlag_None)
{
	for(s32 X = Rect.MinX; X <= Rect.MaxX; ++X)
	{
		PushWall(GameState, V2(X, Rect.MinY), WallC);
		PushWall(GameState, V2(X, Rect.MaxY), WallC);
		for(s32 Y = Rect.MinY + 1; Y < Rect.MaxY; ++Y)
		{
			if(X == Rect.MinX || X == Rect.MaxX)
			{
				b32 IsLeftHole = ((Flags & RoomFlag_LeftHole) != 0) && (X == Rect.MinX) && (Y == (Rect.MinY + Rect.MaxY)/2);
				b32 IsRightHole = ((Flags & RoomFlag_RightHole) != 0) && (X == Rect.MaxX) && (Y == (Rect.MinY + Rect.MaxY)/2);
				b32 IsHole = IsLeftHole || IsRightHole;

				if(IsHole)
				{
					PushGround(GameState, V2(X, Y), GroundC, EntityFlag_RoomConnection);
				}
				else
				{
					PushWall(GameState, V2(X, Y), WallC);
				}
			}
			else
			{
				PushGround(GameState, V2(X, Y), GroundC);
			}
		}
	}
}

internal b32 MoveAllowed(game_state* GameState, v2 WantedP)
{
	for(u32 EntityIndex = 0; EntityIndex < GameState->EntityCount; ++EntityIndex)
	{
		entity* Entity = GameState->Entities + EntityIndex;
		if(Entity->Type == EntityType_Wall && WantedP.x == Entity->P.x && WantedP.y == Entity->P.y)
		{
			return(false);
		}
	}
	return(true);
}

internal entity* GetUnderlyingEntity(game_state* GameState, v2 P)
{
	for(u32 EntityIndex = 0; EntityIndex < GameState->EntityCount; ++EntityIndex)
	{
		entity* Entity = GameState->Entities + EntityIndex;
		if(Entity->Type != EntityType_Player && P.x == Entity->P.x && P.y == Entity->P.y)
		{
			return(Entity);
		}
	}
	return(nullptr);
}

platform_api Platform;

internal renderer_texture LoadBitmap(renderer_texture_queue* TextureOpQueue, char* FileName)
{
    u8 FileBuffer[256*1024] = {};
    platform_file_info FileInfo = {};
    platform_file_handle FileHandle = Platform.OpenFile(FileName, &FileInfo);
    Platform.ReadDataFromFile(&FileHandle, 0, FileInfo.FileSize, FileBuffer);
    Assert(PlatformNoFileErrors(&FileHandle));
    Platform.CloseFile(&FileHandle);

    int PngWidth = 0;
    int PngHeight = 0;
    int NumChannels = 0;
    const int DesiredChannels = 4;
    stbi_uc* Pixels = stbi_load_from_memory(FileBuffer, FileInfo.FileSize, &PngWidth, &PngHeight, &NumChannels, DesiredChannels);
    Assert(Pixels);

    u32 SizeRequested = PngWidth*PngHeight*4;
    u32 TextureHandle = 0; // TODO(hugo)
    renderer_texture Texture = ReferToTexture(TextureHandle, PngWidth, PngHeight);
    texture_op* TextureOp = BeginTextureOp(TextureOpQueue, SizeRequested);
    Assert(TextureOp);
    TextureOp->Texture = Texture;
    Copy(SizeRequested, Pixels, TextureOp->Data);
    CompleteTextureOp(TextureOpQueue, TextureOp);
    stbi_image_free(Pixels);

    return(Texture);
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Platform = Memory->PlatformAPI;
    Assert((&Input->KeyboardController.Terminator - &Input->KeyboardController.Buttons[0]) ==
           (ArrayCount(Input->KeyboardController.Buttons)));

    game_state* GameState = Memory->GameState;
    if(!GameState)
    {
        GameState = Memory->GameState = BootstrapPushStruct(game_state, TotalArena);
#if COMPILE_INTERNAL
		//ImGui::SetCurrentContext(Memory->ImGuiContext);
#endif

		v4 WallC = ColorPalette0[0];
		v4 GroundC = ColorPalette0[1];
		v4 PlayerC = ColorPalette0[2];
		CreateRoom(GameState, RectMinMax(-5, -5, 5, 5), WallC, GroundC, RoomFlag_RightHole);
		CreateRoom(GameState, RectMinMax(6, -5, 16, 5), GroundC, WallC, RoomFlag_LeftHole);
		PushPlayer(GameState, V2(-2.0f, -1.0f), PlayerC);

        GameState->TextSize = 100.0f;
        GameState->TileSetTexture = LoadBitmap(Memory->TextureQueue, "../data/roguelike_tileset.png");
		DefaultTexture = GameState->TileSetTexture;
    }

	GameState->Timer += Input->dtForFrame;

	
	for(u32 EntityIndex = 0; EntityIndex < GameState->EntityCount; ++EntityIndex)
	{
		entity* Entity = GameState->Entities + EntityIndex;
#if 0
		if(ShowImgui)
		{
			char Buffer[256];
			FormatString(ArrayCount(Buffer), Buffer, "Entity %u", EntityIndex);
			if(ImGui::CollapsingHeader(Buffer))
			{
				ImGui::SliderFloat2("P", &Entity->P.x, 0.0f, 100.0f);
				ImGui::SliderFloat2("TileSetP", &Entity->TileSetP.x, 0.0f, 100.0f);
				ImGui::ColorEdit4("Color", &Entity->Color.x);
				ImGui::Text("Type %u", Entity->Type);
			}
		}
#endif

		if(Entity->Type == EntityType_Player)
		{
			v2 WantedP = Entity->P;
			if(WasPressed(Input->KeyboardController.MoveLeft))
			{
				WantedP.x -= 1.0f;
			}
			if(WasPressed(Input->KeyboardController.MoveRight))
			{
				WantedP.x += 1.0f;
			}
			if(WasPressed(Input->KeyboardController.MoveUp))
			{
				WantedP.y += 1.0f;
			}
			if(WasPressed(Input->KeyboardController.MoveDown))
			{
				WantedP.y -= 1.0f;
			}

			if(MoveAllowed(GameState, WantedP))
			{
				Entity->P = WantedP;

				entity* UnderlyingEntity = GetUnderlyingEntity(GameState, WantedP);
				if(UnderlyingEntity)
				{
					if((UnderlyingEntity->Flags & EntityFlag_RoomConnection) != 0)
					{
						if(GameState->JustSteppedOnConnectionTimer <= -1.0f)
						{
							GameState->JustSteppedOnConnectionTimer = 5.0f;
						}
					}
					else
					{
						if(GameState->JustSteppedOnConnectionTimer <= 0.0f)
						{
							GameState->JustSteppedOnConnectionTimer = -1.0f;
						}
					}
				}
			}
		}
		PushTile(RenderCommands, GameState->TileSetTexture, Entity->P, Entity->TileSetP, Entity->Color);
	}

	if(GameState->JustSteppedOnConnectionTimer > 0.0f)
	{
		GameState->JustSteppedOnConnectionTimer -= Input->dtForFrame;
		f32 Alpha = Clamp01MapToRange(0.0f, GameState->JustSteppedOnConnectionTimer, 5.0f);
		v4 TextC = ColorPalette0[4];
		TextC.a = Alpha;
		PushText(RenderCommands, V2(0.0f, 200.0f), TextC, GameState->TextSize, "NEW ROOM");
	}

	PushRect(RenderCommands, V2(0.0f, 0.0f), V2(100.0f, 100.0f), V4(1.0f, 0.0f, 0.0f, 0.5f));
#if 0
    ImGui::Text("%f", GameState->JustSteppedOnConnectionTimer);
    ImGui::SliderFloat("Text Size", &GameState->TextSize, 0.0f, 500.0f);
    if(ImGui::Button("TIME"))
    {
        GameState->JustSteppedOnConnectionTimer = 5.0f;
    }
#endif
}

