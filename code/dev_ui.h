#define DEVUI_COLLAPSING_HEADER(name) b32 name(const char* label)
typedef DEVUI_COLLAPSING_HEADER(collapsing_header);
#define DEVUI_SLIDER_FLOAT2(name) b32 name(const char* label, float v[2], float v_min, float v_max)
typedef DEVUI_SLIDER_FLOAT2(slider_float2);
#define DEVUI_COLOR_EDIT4(name) b32 name(const char* label, float col[4])
typedef DEVUI_COLOR_EDIT4(color_edit4);
#define DEVUI_TEXT(name) void name(const char* fmt, ...)
typedef DEVUI_TEXT(text);

typedef struct dev_ui
{
    collapsing_header* CollapsingHeader;
    slider_float2* SliderFloat2;
    color_edit4* ColorEdit4;
    text* Text;
} dev_ui;

#ifdef DEVUI_IMPLEMENTATION

#include "imgui.cpp"
#undef STB_TRUETYPE_IMPLEMENTATION
#undef __STB_INCLUDE_STB_TRUETYPE_H__
#include "imgui_draw.cpp"
#include "imgui_widgets.cpp"
#include "imgui_demo.cpp"

DEVUI_COLLAPSING_HEADER(DevUICollapsingHeader)
{
    return(ImGui::CollapsingHeader(label));
}

DEVUI_SLIDER_FLOAT2(DevUISliderFloat2)
{
    return(ImGui::SliderFloat2(label, v, v_min, v_max));
}

DEVUI_COLOR_EDIT4(DevUIColorEdit4)
{
    return(ImGui::ColorEdit4(label, col));
}

DEVUI_TEXT(DevUIText)
{
    va_list ArgList;
    va_start(ArgList, fmt);
    ImGui::TextV(fmt, ArgList);
    va_end(ArgList);
}

#endif
