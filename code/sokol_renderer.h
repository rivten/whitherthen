
#define SOKOL_LOAD_RENDERER(name) platform_renderer *name(platform_renderer_limits *Limits)
typedef SOKOL_LOAD_RENDERER(sokol_load_renderer);
#define SOKOL_LOAD_RENDERER_ENTRY() SOKOL_LOAD_RENDERER(SokolLoadRenderer)

struct sokol_renderer_function_table
{
    sokol_load_renderer *LoadRenderer;
    renderer_begin_frame *BeginFrame;
    renderer_end_frame *EndFrame;
};
global char *SokolRendererFunctionTableNames[] = 
{
    "SokolLoadRenderer",
    "SokolBeginFrame",
    "SokolEndFrame",
};
