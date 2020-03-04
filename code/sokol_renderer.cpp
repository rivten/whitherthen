#define SOKOL_GL_IMPL
#include "sokol_gl.h"
#define SOKOL_FONTSTASH_IMPL
#include "sokol_fontstash.h"

#define SOKOL_IMGUI_IMPL
#include "sokol_imgui.h"

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

#define GFX_INIT_BUFFER_SIZE (1 << 15)

internal platform_renderer* RendererInit(platform_renderer_limits* Limits)
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
    ImageDesc.min_filter = SG_FILTER_NEAREST;
    ImageDesc.mag_filter = SG_FILTER_NEAREST;
    ImageDesc.wrap_u = SG_WRAP_MIRRORED_REPEAT;
    ImageDesc.wrap_v = SG_WRAP_MIRRORED_REPEAT;
    ImageDesc.wrap_w = SG_WRAP_MIRRORED_REPEAT;
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

		smooth out vec2 FragUV;
		smooth out vec4 FragColor;
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
		smooth in vec2 FragUV;
		smooth in vec4 FragColor;
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

	simgui_desc_t ImguiDesc = {};
	simgui_setup(&ImguiDesc);

	return((platform_renderer *)GFX);
}

internal void RendererDestroy(platform_renderer* Renderer)
{
	simgui_shutdown();
    sokol_gfx* GFX = (sokol_gfx *)Renderer;
    sfons_destroy(GFX->FontContext);
    sgl_shutdown();
	sg_shutdown();
}

game_render_commands* RendererBeginFrame(platform_renderer* Renderer, v2u OSWindowDim, f32 dt)
{
	sokol_gfx* GFX = (sokol_gfx *)Renderer;
	simgui_new_frame(OSWindowDim.x, OSWindowDim.y, dt);

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

void RendererEndFrame(platform_renderer* Renderer, game_render_commands* Commands)
{
	sokol_gfx* GFX = (sokol_gfx *)Renderer;

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

