#pragma once
#include <cstdint>
#include "ts3/layout.h"
namespace ts3::ui::b09 {
struct RenderNode { RenderNode* next; RenderNode* previous; void* renderable; void* controller; };
struct RenderBuffer {
    void** table; void* pending; void* owner; int32_t index;
    uint8_t flag,pad11[3];float rect[4];void* p24;void* p28;void* p2c;const char* tag;
};
// The embedded list head is only a next/previous pair.
struct RenderHead { RenderNode* next;RenderNode* previous; };
struct BufferedRenderer {
    uint8_t opaque00[0x40];void* active;void* fallback;uint32_t untouched48,zero4c;int32_t index;
    void* saved;RenderHead head;RenderBuffer buffer;uint8_t direct;
};
TS3_X86_OFFSET(BufferedRenderer,active,0x40);
TS3_X86_OFFSET(BufferedRenderer,head,0x58);
TS3_X86_OFFSET(BufferedRenderer,buffer,0x60);
TS3_X86_OFFSET(BufferedRenderer,direct,0x94);
// TS3W.exe 0x0050E710: drain the pending renderable directly or append a retained list node; clear the direct byte on every path.
uintptr_t __fastcall Flush(BufferedRenderer*,void*);
// TS3W.exe 0x0050E7D0: initialize the renderer's sparse fields and embedded original buffer, returning self.
BufferedRenderer* __fastcall InitRenderer(BufferedRenderer*,void*,void* owner);
// TS3W.exe 0x0050E850: flush before changing the active controller, store the complete direct byte, and return the embedded buffer.
RenderBuffer* __fastcall SelectRenderer(BufferedRenderer*,void*,void* controller,uint8_t direct);
struct ValueType { uint8_t opaque00[8];uint32_t size; };
struct PropertyValue { ValueType* type;void* data;uint32_t count; };
struct PropertySource { void* opaque00;uint8_t* data; };
struct Property;
using PropertyCallback=uint8_t(__cdecl*)(PropertyValue*,PropertySource*,Property*,void*);
struct PropertyMethods { void* allocate;PropertyCallback direct;PropertyCallback prepare; };
struct Property { uint32_t opaque00;PropertyMethods* methods;uint32_t opaque08;int16_t kind,offset;uint32_t count; };
TS3_X86_OFFSET(Property,kind,0xc);
TS3_X86_OFFSET(Property,count,0x10);
// TS3W.exe 0x005116B0: resolve a property, honoring direct callbacks and compacting kind-18 data from a sixteen-byte source stride.
uint8_t __fastcall ReadProperty(Property*,void*,PropertyValue*,PropertySource*,void* allocator);
// TS3W.exe 0x00511790: prepare property storage or a borrowed source view, preserving signed-kind count relaxation and zero-count success.
uint8_t __fastcall PrepareProperty(Property*,void*,PropertyValue*,PropertySource*,uint32_t count,void* allocator,uint8_t force);
struct alignas(16) DrawGeometry { float matrix[16];uint32_t flags;uint8_t opaque44[12];uint32_t color;uint8_t opaque54[12]; };
struct DrawTreeNode {
    DrawTreeNode* next;DrawTreeNode* previous;uint8_t opaque08[6];uint8_t selected;uint8_t opaque0f[3];uint16_t level;uint8_t opaque14[8];void* effect;
    RenderHead children;uint8_t opaque28[8];alignas(16) DrawGeometry geometry[2];
};
struct alignas(16) TraversalRenderer {
    void* geometry;uint32_t color;float bounds[4];uint8_t opaque18[8];alignas(16) float translation[4],scale[4];
    void* active;void* fallback;DrawTreeNode* list;void* transform;uint32_t tint;void* saved;
};
TS3_X86_OFFSET(DrawTreeNode,effect,0x1c);
TS3_X86_OFFSET(DrawTreeNode,children,0x20);
TS3_X86_OFFSET(DrawTreeNode,geometry,0x30);
TS3_X86_OFFSET(TraversalRenderer,list,0x48);
TS3_X86_OFFSET(TraversalRenderer,saved,0x54);
// TS3W.exe 0x0050EB20: traverse the draw tree, transform borrowed geometry, switch controllers, and retain EA's alpha threshold and level cutoff.
uintptr_t __fastcall Traverse(TraversalRenderer*,void*,DrawTreeNode* list,float* bounds);
// Only the two integer dimensions are consumed; the preceding bytes are opaque.
struct TileDescriptor { uint8_t opaque[0x1c]; int32_t width,height; };
TS3_X86_OFFSET(TileDescriptor,width,0x1c);
TS3_X86_OFFSET(TileDescriptor,height,0x20);
// TS3W.exe 0x0050EDF0: interpolate packed color bytes, truncating and wrapping rather than clamping.
uint32_t __cdecl BlendColor(uint32_t a,uint32_t b,float t);
// TS3W.exe 0x0050EEF0: clip a quad and generate unit UVs, keeping pointer and LAHF-modified incidental EAX returns on rejected spans.
uintptr_t __cdecl ClippedQuad(void* renderer,float* bounds,uint32_t color,float* clip,int mode);
// TS3W.exe 0x00510F00: emit eight border triangles; the x86 entry preserves incoming EAX except AH on degenerate rectangles.
uint32_t __cdecl BorderTriangles(void* renderer,float* outer,float* inner,uint32_t* edge,uint32_t* center);
// TS3W.exe 0x00511250: shade a three-pixel bevel, set its center color, and return the fill virtual's EAX.
uint32_t __cdecl DrawBevel(void* renderer,float* bounds,uint32_t color,uint8_t flags);
// TS3W.exe 0x00511510: draw two feathered border bands and return the second triangle helper's EAX.
uint32_t __cdecl DrawFeather(void* renderer,float* bounds,uint32_t color,float thickness);
// TS3W.exe 0x0050F900: tile both axes with source-scaled dimensions and x87 phase wrapping.
void __cdecl TileBoth(void* renderer,float* bounds,float* source,TileDescriptor*,float sx,float sy,float ox,float oy);
// TS3W.exe 0x0050FB10: tile horizontally; dimensions below one suppress drawing, and exact phase multiples remain zero.
void __cdecl TileHorizontal(void* renderer,float* bounds,float* source,TileDescriptor*,float scale,float phase,int mode);
// TS3W.exe 0x0050FC30: tile vertically with source-scaled dimensions and original phase wrapping.
void __cdecl TileVertical(void* renderer,float* bounds,float* source,TileDescriptor*,float scale,float phase,int mode);
}
