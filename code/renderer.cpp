
internal renderer_texture ReferToTexture(u32 Index, u32 Width, u32 Height)
{
    renderer_texture Result;
    
    Result.Index = Index;
    Result.Width = (u16)Width;
    Result.Height = (u16)Height;
    
    Assert(Result.Index == Index);
    Assert(Result.Width == Width);
    Assert(Result.Height == Height);
    
    return(Result);
}

internal texture_op* BeginTextureOp(renderer_texture_queue* Queue, u32 SizeRequested)
{
    texture_op* Result = 0;
    if(Queue->OpCount < ArrayCount(Queue->Ops))
    {
        u32 SizeAvailable = 0;

        u32 MemoryAt = Queue->TransferMemoryLastUsed;
        if(Queue->TransferMemoryLastUsed == Queue->TransferMemoryFirstUsed)
        {
            // NOTE(casey): The used space is either the entire buffer or none of the buffer,
            // and we disambiguate between those two by just checking if there are any
            // ops outstanding
            if(Queue->OpCount == 0)
            {
                MemoryAt = 0;
                SizeAvailable = Queue->TransferMemoryCount;
            }
        }
        else if(Queue->TransferMemoryLastUsed < Queue->TransferMemoryFirstUsed)
        {
            // NOTE(casey): The used space wraps around, one continuous usable space
            SizeAvailable = (Queue->TransferMemoryFirstUsed - MemoryAt);
        }
        else
        {
            // NOTE(casey): The used space doesn't wrap, two usable spaces on either side
            SizeAvailable = (Queue->TransferMemoryCount - MemoryAt);
            if(SizeAvailable < SizeRequested)
            {
                MemoryAt = 0;
                SizeAvailable = (Queue->TransferMemoryFirstUsed - MemoryAt);
            }
        }

        if(SizeRequested <= SizeAvailable)
        {
            u32 OpIndex = Queue->FirstOpIndex + Queue->OpCount++;
            Result = Queue->Ops + (OpIndex % ArrayCount(Queue->Ops));
            
            Result->Data = Queue->TransferMemory + MemoryAt;
            Result->TransferMemoryLastUsed = MemoryAt + SizeRequested;
            Result->State = TextureOp_PendingLoad;
            
#if 0
            for(u32 Index = 0;
                Index < Result->TransferMemoryCount;
                ++Index)
            {
                ((u8 *)Result->Data)[Index] = 0xFF;
            }
#endif
            
            Queue->TransferMemoryLastUsed = Result->TransferMemoryLastUsed;
            
            Assert(MemoryAt < Queue->TransferMemoryCount);
            Assert(Result->TransferMemoryLastUsed <= Queue->TransferMemoryCount);
        }
    }

    return(Result);
}

internal void CompleteTextureOp(renderer_texture_queue *Queue, texture_op *Op)
{
    Op->State = TextureOp_ReadyToTransfer;
}

internal void CancelTextureOp(renderer_texture_queue *Queue, texture_op *Op)
{
    Op->State = TextureOp_Empty;
}

internal v2 GetUVScaleForTexture(renderer_texture Texture)
{
#if 0
    v2 Result =
    {
        (f32)Texture.Width / (f32)TEXTURE_ARRAY_DIM,
        (f32)Texture.Height / (f32)TEXTURE_ARRAY_DIM
    };
#else
    // TODO(hugo): Need to make this more rigourous...
    v2 Result =
    {
        1.0f / (f32)TEXTURE_ARRAY_DIM,
        1.0f / (f32)TEXTURE_ARRAY_DIM
    };
#endif
    
    return(Result);
}

inline void PushQuad(game_render_commands* Commands, renderer_texture Texture,
		v4 P0, v2 UV0, u32 C0,
		v4 P1, v2 UV1, u32 C1,
		v4 P2, v2 UV2, u32 C2,
		v4 P3, v2 UV3, u32 C3)
{
	++Commands->QuadCount;

    u32 VertIndex = Commands->VertexCount;
    u32 IndexIndex = Commands->IndexCount;

    Commands->VertexCount += 4;
    Commands->IndexCount += 6;
    Assert(Commands->VertexCount <= Commands->MaxVertexCount);
    Assert(Commands->IndexCount <= Commands->MaxIndexCount);

    textured_vertex *Vert = Commands->VertexArray + VertIndex;
    u16 *Index = Commands->IndexArray + IndexIndex;

    v2 InvUV = GetUVScaleForTexture(Texture);

    UV0 = Hadamard(InvUV, UV0);
    UV1 = Hadamard(InvUV, UV1);
    UV2 = Hadamard(InvUV, UV2);
    UV3 = Hadamard(InvUV, UV3);

    u32 TextureIndex32 = TextureIndexFrom(Texture);
    u16 TextureIndex = (u16)TextureIndex32;
    Assert(TextureIndex == TextureIndex32);

    Vert[0].P = P3;
    Vert[0].UV = UV3;
    Vert[0].Color = C3;
    Vert[0].TextureIndex = TextureIndex;
    
    Vert[1].P = P0;
    Vert[1].UV = UV0;
    Vert[1].Color = C0;
    Vert[1].TextureIndex = TextureIndex;
    
    Vert[2].P = P2;
    Vert[2].UV = UV2;
    Vert[2].Color = C2;
    Vert[2].TextureIndex = TextureIndex;
    
    Vert[3].P = P1;
    Vert[3].UV = UV1;
    Vert[3].Color = C1;
    Vert[3].TextureIndex = TextureIndex;
    
    u32 BaseIndex = VertIndex;
    u16 VI = (u16)BaseIndex;
    Assert((u32)VI == BaseIndex);
    
    Index[0] = VI + 0;
    Index[1] = VI + 1;
    Index[2] = VI + 2;
    Index[3] = VI + 1;
    Index[4] = VI + 3;
    Index[5] = VI + 2;
}

inline void PushQuad(game_render_commands* Commands, renderer_texture Texture,
		v4 P0, v2 UV0, v4 C0,
		v4 P1, v2 UV1, v4 C1,
		v4 P2, v2 UV2, v4 C2,
		v4 P3, v2 UV3, v4 C3)
{
	PushQuad(Commands, Texture,
			P0, UV0, RGBAPack4x8(255.0f * C0),
			P1, UV1, RGBAPack4x8(255.0f * C1),
			P2, UV2, RGBAPack4x8(255.0f * C2),
			P3, UV3, RGBAPack4x8(255.0f * C3));
}

inline void PushBitmap(game_render_commands* Commands, renderer_texture Texture,
		rectangle2 SourceRect, rectangle2 DestRect, v4 Color)
{
	v4 P0 = V4(DestRect.Min.x / (float)Commands->OSWindowDim.x, DestRect.Min.y / (float)Commands->OSWindowDim.y, 0.0f, 1.0f);
	v2 UV0 = V2(SourceRect.Min.x, SourceRect.Max.y);
	v4 C0 = Color;

	v4 P1 = V4(DestRect.Max.x / (float)Commands->OSWindowDim.x, DestRect.Min.y / (float)Commands->OSWindowDim.y, 0.0f, 1.0f); 
	v2 UV1 = V2(SourceRect.Max.x, SourceRect.Max.y); 
	v4 C1 = Color;

	v4 P2 = V4(DestRect.Max.x / (float)Commands->OSWindowDim.x, DestRect.Max.y / (float)Commands->OSWindowDim.y, 0.0f, 1.0f);
	v2 UV2 = V2(SourceRect.Max.x, SourceRect.Min.y);
	v4 C2 = Color;

	v4 P3 = V4(DestRect.Min.x / (float)Commands->OSWindowDim.x, DestRect.Max.y / (float)Commands->OSWindowDim.y, 0.0f, 1.0f);
	v2 UV3 = V2(SourceRect.Min.x, SourceRect.Min.y);
	v4 C3 = Color;

	PushQuad(Commands, Texture,
			P0, UV0, C0,
			P1, UV1, C1,
			P2, UV2, C2,
			P3, UV3, C3);
}

internal void PushText(game_render_commands* Commands, v2 P, v4 Color, f32 Size, char* Text)
{
	Assert(Commands->RenderTextCommandCount < ArrayCount(Commands->RenderTextCommands));
	render_text_command* Command = Commands->RenderTextCommands + Commands->RenderTextCommandCount++;
	Command->P = P;
	Command->Color = RGBAPack4x8(255.0f * Color);
	Command->Size = Size;

	memory_index StringSize = StringLength(Text);
	Assert(StringSize < ArrayCount(Command->Text));
	Copy(StringSize, Text, Command->Text);
}

#define TILE_SIZE_IN_PIXELS 16
internal void PushTile(game_render_commands* RenderCommands, renderer_texture Texture, v2 Pos, v2 TileSetPos, v4 Color)
{
    rectangle2 DestRect = RectCenterHalfDim(2.0f * TILE_SIZE_IN_PIXELS * Pos, V2(TILE_SIZE_IN_PIXELS, TILE_SIZE_IN_PIXELS));
    rectangle2 SourceRect = RectMinDim(TILE_SIZE_IN_PIXELS * TileSetPos, V2(TILE_SIZE_IN_PIXELS, TILE_SIZE_IN_PIXELS));
	PushBitmap(RenderCommands, Texture, SourceRect, DestRect, Color);
}

#define DEFAULT_TILE_SET_POS V2(1, 0) // NOTE(hugo): This is a white square

global renderer_texture DefaultTexture = {};

inline void PushRect(game_render_commands* RenderCommands, v2 P, v2 Dim, v4 Color = V4(1, 1, 1, 1))
{
	u32 C = RGBAPack4x8(255.0f*Color);

	rectangle2 DestRect = RectMinDim(P, Dim);
	rectangle2 SourceRect = RectMinDim(TILE_SIZE_IN_PIXELS * DEFAULT_TILE_SET_POS, V2(TILE_SIZE_IN_PIXELS, TILE_SIZE_IN_PIXELS));
	PushBitmap(RenderCommands, DefaultTexture, SourceRect, DestRect, Color);
}
