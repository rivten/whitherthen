struct game_render_commands;
struct platform_renderer;
#define RENDERER_BEGIN_FRAME(name) game_render_commands *name(platform_renderer *Renderer, v2u OSWindowDim, f32 dt)
#define RENDERER_END_FRAME(name) void name(platform_renderer *Renderer, game_render_commands *Frame)

typedef RENDERER_BEGIN_FRAME(renderer_begin_frame);
typedef RENDERER_END_FRAME(renderer_end_frame);

#define TEXTURE_ARRAY_DIM 512
#define TEXTURE_SPECIAL_BIT 0x80000000

struct platform_renderer_limits
{
    u32 MaxQuadCountPerFrame;
    u32 MaxTextureCount;
    u32 TextureTransferBufferSize;
};

struct textured_vertex
{
	v4 P;
	v2 UV;
	u32 Color;
    u32 TextureIndex;
};

struct render_text_command
{
	v2 P;
	u32 Color;
	f32 Size;
	char Text[128];
};

union renderer_texture
{
    u64 Packed;
    struct
    {
        // NOTE(casey): You could pack an offset in here if you wanted!  Just use
        // a 16-bit index.
        u32 Index;
        u16 Width;
        u16 Height;
    };
};

enum texture_op_state
{
    TextureOp_Empty,
    TextureOp_PendingLoad,
    TextureOp_ReadyToTransfer,
};

struct texture_op
{
    renderer_texture Texture;
    void* Data;
    u32 TransferMemoryLastUsed;
    volatile texture_op_state State;
};

struct renderer_texture_queue
{
    u32 TransferMemoryCount;
    u32 TransferMemoryFirstUsed;
    u32 TransferMemoryLastUsed;
    u8* TransferMemory;

    u32 OpCount;
    u32 FirstOpIndex;
    texture_op Ops[256];
};

struct platform_renderer
{
    renderer_texture_queue TextureQueue;
    umm TotalTextureMemory;
    umm TotalFramebufferMemory;

    void* Platform;
};

struct game_render_commands
{
	v2u OSWindowDim;
#if 0
	rectangle2i OSDrawRegion;
	u32 MaxPushBufferSize;
	u8* PushBufferBase;
	u8* PushBufferAt;
#endif

	u32 MaxVertexCount;
	u32 VertexCount;
	textured_vertex* VertexArray;

	u32 MaxIndexCount;
	u32 IndexCount;
	u16* IndexArray;

	u32 QuadCount;

	u32 RenderTextCommandCount;
	render_text_command RenderTextCommands[128];
};

internal u32 TextureIndexFrom(renderer_texture Texture)
{
    u32 Result = (Texture.Index & ~TEXTURE_SPECIAL_BIT);
    return(Result);
}

