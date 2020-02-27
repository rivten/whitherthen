
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

struct sokol_gfx
{
    platform_renderer Header;

	sg_pipeline Pipeline;
	sg_bindings Bindings;
	sg_pass_action PassAction;

	sg_buffer VertexBuffer;
	sg_buffer IndexBuffer;

    sg_image TextureArray;

    u32 MaxQuadTextureCount;
    u32 MaxTextureCount;
    u32 MaxVertexCount;
    u32 MaxIndexCount;

    textured_vertex *VertexArray;
    u16 *IndexArray;

	FONScontext* FontContext;
    game_render_commands RenderCommands;
};

#define TEXTURE_ARRAY_DIM 512
#define TEXTURE_SPECIAL_BIT 0x80000000

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

internal u32 TextureIndexFrom(renderer_texture Texture)
{
    u32 Result = (Texture.Index & ~TEXTURE_SPECIAL_BIT);
    return(Result);
}

internal void SokolGFXManageTextures(sokol_gfx* GFX, renderer_texture_queue* Queue)
{
    while(Queue->OpCount)
    {
        texture_op* Op = Queue->Ops + Queue->FirstOpIndex;
        if(Op->State == TextureOp_PendingLoad)
        {
            break;
        }
        else if(Op->State == TextureOp_ReadyToTransfer)
        {
            renderer_texture Texture = Op->Texture;
            void* Data = Op->Data;
            u32 TextureIndex = TextureIndexFrom(Texture);
            Assert(TextureIndex < GFX->MaxTextureCount);
            sg_image_content ImageContent = {};
            ImageContent.subimage[0][0].size = Op->Texture.Width * Op->Texture.Height * 4;
            ImageContent.subimage[0][0].ptr = Op->Data;

            sg_update_image(GFX->TextureArray, &ImageContent);
        }
        else
        {
            Assert(Op->State == TextureOp_Empty);
        }

        Queue->TransferMemoryFirstUsed = Op->TransferMemoryLastUsed;
        --Queue->OpCount;
        ++Queue->FirstOpIndex;
        if(Queue->FirstOpIndex >= ArrayCount(Queue->Ops))
        {
            Queue->FirstOpIndex = 0;
        }
    }
}

internal void* SokolRendererAlloc(umm Size)
{
	void* Result = malloc(Size);
	memset(Result, 0, Size);
	return(Result);
}

internal void InitTextureQueue(renderer_texture_queue *Queue, u32 RequestTransferBufferSize, void *Memory)
{
    Queue->TransferMemoryCount = RequestTransferBufferSize;
    Queue->TransferMemoryFirstUsed = 0;
    Queue->TransferMemoryLastUsed = 0;
    Queue->TransferMemory = (u8 *)Memory;
    
    Queue->OpCount = 0;
    Queue->FirstOpIndex = 0;
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

#define GFX_INIT_BUFFER_SIZE (1 << 15)

internal sokol_gfx* SokolInitGFX(platform_renderer_limits* Limits)
{
	sokol_gfx* GFX = (sokol_gfx *)SokolRendererAlloc(sizeof(sokol_gfx));
    InitTextureQueue(&GFX->Header.TextureQueue, Limits->TextureTransferBufferSize, SokolRendererAlloc(Limits->TextureTransferBufferSize));
	u32 MaxVertexCount = Limits->MaxQuadCountPerFrame*4;
	u32 MaxIndexCount = Limits->MaxQuadCountPerFrame*6;
    GFX->MaxTextureCount = Limits->MaxTextureCount;
	GFX->MaxVertexCount = MaxVertexCount;
	GFX->MaxIndexCount = MaxIndexCount;
	GFX->VertexArray = (textured_vertex *)SokolRendererAlloc(MaxVertexCount * sizeof(textured_vertex));
	GFX->IndexArray = (u16 *)SokolRendererAlloc(MaxIndexCount * sizeof(u16));

	sg_desc SokolGFXDesc = {};
	sg_setup(&SokolGFXDesc);

    sgl_desc_t SokolGLDesc = {};
    sgl_setup(&SokolGLDesc);

    // NOTE(hugo): Setup projection matrix for font rendering
    // Weirdly, this does not seem to work if I put this inside the Frame function...
    sgl_matrix_mode_projection();
    sgl_ortho(-0.5f * sapp_width(), 0.5f * sapp_width(), -0.5f * sapp_height(), 0.5f * sapp_height(), -1.0f, 1.0f);

	sg_buffer_desc VertexBufferDesc = {};
	VertexBufferDesc.size = GFX_INIT_BUFFER_SIZE;
	VertexBufferDesc.content = 0;
	VertexBufferDesc.type = SG_BUFFERTYPE_VERTEXBUFFER;
	VertexBufferDesc.usage = SG_USAGE_STREAM;
	VertexBufferDesc.label = "VertexBuffer";
	GFX->VertexBuffer = sg_make_buffer(&VertexBufferDesc);

	sg_buffer_desc IndexBufferDesc = {};
	IndexBufferDesc.size = GFX_INIT_BUFFER_SIZE;
	IndexBufferDesc.content = 0;
	IndexBufferDesc.type = SG_BUFFERTYPE_INDEXBUFFER;
	IndexBufferDesc.usage = SG_USAGE_STREAM;
	IndexBufferDesc.label = "IndexBuffer";
	GFX->IndexBuffer = sg_make_buffer(&IndexBufferDesc);

    sg_image_desc ImageDesc = {};
    ImageDesc.width = TEXTURE_ARRAY_DIM;
    ImageDesc.height = TEXTURE_ARRAY_DIM;
    ImageDesc.layers = GFX->MaxTextureCount;
    ImageDesc.usage = SG_USAGE_STREAM;
    ImageDesc.type = SG_IMAGETYPE_ARRAY;
    ImageDesc.content = {};
    GFX->TextureArray = sg_make_image(&ImageDesc);

	GFX->Bindings.vertex_buffers[0] = GFX->VertexBuffer;
	GFX->Bindings.index_buffer = GFX->IndexBuffer;
	GFX->Bindings.fs_images[0] = GFX->TextureArray;
		
	sg_shader_desc ShaderDesc = {};
	ShaderDesc.attrs[0].name = "VertP";
	ShaderDesc.attrs[1].name = "VertUV";
	ShaderDesc.attrs[2].name = "VertColor";
	ShaderDesc.attrs[3].name = "VertTexIndex";
	ShaderDesc.vs.source = R"FOO(
		#version 330 core

		in vec4 VertP;
		in vec2 VertUV;
		in vec4 VertColor;
        in int VertTexIndex;

		out vec2 FragUV;
		out vec4 FragColor;
        flat out int FragTexIndex;

		void main()
		{
			gl_Position = VertP;
			FragUV = VertUV;
			FragColor = VertColor;
            FragTexIndex = VertTexIndex;
		}
	)FOO";

	ShaderDesc.fs.source = R"FOO(
		#version 330 core
		in vec2 FragUV;
		in vec4 FragColor;
        flat in int FragTexIndex;

		uniform sampler2DArray TextureSampler;

		out vec4 FinalColor;

		void main()
		{
            vec3 ArrayUV = vec3(FragUV.x, FragUV.y, float(FragTexIndex));
            float textureA = texture(TextureSampler, ArrayUV).a;
			FinalColor = FragColor * textureA;
		}
	)FOO";
	ShaderDesc.fs.images[0].type = SG_IMAGETYPE_ARRAY;
	ShaderDesc.fs.images[0].name = "TextureSampler";
	sg_shader Shader = sg_make_shader(&ShaderDesc);

	sg_pipeline_desc PipelineDesc = {};
	PipelineDesc.shader = Shader,
	PipelineDesc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT4;
	PipelineDesc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT2;
	PipelineDesc.layout.attrs[2].format = SG_VERTEXFORMAT_UBYTE4N;
	PipelineDesc.layout.attrs[3].format = SG_VERTEXFORMAT_UBYTE4;
	PipelineDesc.index_type = SG_INDEXTYPE_UINT16;
    PipelineDesc.blend.enabled = true;
    PipelineDesc.blend.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
    PipelineDesc.blend.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;

	PipelineDesc.label = "QuadPipeline";

	GFX->Pipeline = sg_make_pipeline(&PipelineDesc);
	GFX->PassAction = {};
	GFX->PassAction.colors[0].action = SG_ACTION_CLEAR;
	GFX->PassAction.colors[0].val[0] = 0.0f;
	GFX->PassAction.colors[0].val[1] = 0.0f;
	GFX->PassAction.colors[0].val[2] = 0.0f;
	GFX->PassAction.colors[0].val[3] = 1.0f;

    s32 AtlasWidth = 512;
    s32 AtlasHeight = 512;
    GFX->FontContext = sfons_create(AtlasWidth, AtlasHeight, FONS_ZERO_BOTTOMLEFT);
    int IosevkaFont = fonsAddFont(GFX->FontContext, "Iosevka", "../data/iosevka-term-regular_2.ttf");
    Assert(IosevkaFont != FONS_INVALID);
    fonsSetFont(GFX->FontContext, IosevkaFont);

	return(GFX);
}

internal void RendererDestroy(sokol_gfx* GFX)
{
    sfons_destroy(GFX->FontContext);
    sgl_shutdown();
	sg_shutdown();
}

game_render_commands* RendererBeginFrame(sokol_gfx* GFX, v2u OSWindowDim)
{
	game_render_commands* Commands = &GFX->RenderCommands;
	Commands->MaxVertexCount = GFX->MaxVertexCount;
	Commands->VertexCount = 0;
	Commands->MaxIndexCount = GFX->MaxIndexCount;
	Commands->IndexCount = 0;
	Commands->VertexArray = GFX->VertexArray;
	Commands->IndexArray = GFX->IndexArray;
	Commands->QuadCount = 0;

	Commands->OSWindowDim = OSWindowDim;
	Commands->RenderTextCommandCount = 0;

	return(Commands);
}

void RendererEndFrame(sokol_gfx* GFX, game_render_commands* Commands)
{
	Assert(sg_query_image_info(GFX->Bindings.fs_images[0]).slot.state != SG_RESOURCESTATE_FAILED);
	Assert(Commands->VertexCount * sizeof(textured_vertex) <= GFX_INIT_BUFFER_SIZE);
	Assert(Commands->IndexCount * sizeof(u16) <= GFX_INIT_BUFFER_SIZE);
	sg_update_buffer(GFX->VertexBuffer, Commands->VertexArray, Commands->VertexCount * sizeof(textured_vertex));
	sg_update_buffer(GFX->IndexBuffer, Commands->IndexArray, Commands->IndexCount * sizeof(u16));

    SokolGFXManageTextures(GFX, &GFX->Header.TextureQueue);

	sg_begin_default_pass(&GFX->PassAction, sapp_width(), sapp_height());
	sg_apply_pipeline(GFX->Pipeline);
	sg_apply_bindings(&GFX->Bindings);
	sg_draw(0, 6 * Commands->QuadCount, 1);

	for(u32 CommandIndex = 0; CommandIndex < Commands->RenderTextCommandCount; ++CommandIndex)
	{
		render_text_command* Command = Commands->RenderTextCommands + CommandIndex;
		fonsSetSize(GFX->FontContext, Command->Size);
		fonsSetColor(GFX->FontContext, Command->Color);
		fonsDrawText(GFX->FontContext, Command->P.x, Command->P.y, Command->Text, nullptr);
	}
    sfons_flush(GFX->FontContext);
	sgl_draw();

	simgui_render();

	sg_end_pass();

	sg_commit();
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


