#pragma once
#include "ts3/layout.h"
namespace ts3::ui::b04 {
struct Rect { float x,y,right,bottom; };
struct DrawContext { void** table; void* resource; uint32_t reserved,flags; };
struct Texture { void** table; unsigned char opaque[24]; int32_t width,height; };
struct CellMap { void** table; uint32_t opaque[2]; int32_t width,height; uint32_t* begin; uint32_t* end; };
struct Iterator { void** table; void* owner; uint32_t kind; void* cursor; };
TS3_X86_OFFSET(DrawContext,flags,0xc);
TS3_X86_OFFSET(Texture,width,0x1c);
TS3_X86_OFFSET(CellMap,begin,0x14);
// TS3W.exe 0x00509560: draw a queried texture at its native size with unit UVs.
void DrawNative(DrawContext*,float,float,void*);
// TS3W.exe 0x00509600: draw a queried texture at native size with supplied UVs.
void DrawNativeUv(DrawContext*,float,float,void*,const Rect*);
// TS3W.exe 0x00509680: draw a queried texture in supplied bounds and UVs.
void DrawRect(DrawContext*,const Rect*,void*,const Rect*);
// TS3W.exe 0x005096C0: query two textures in order and draw with native size and unit UVs.
void DrawPairNative(DrawContext*,float,float,void*,void*);
// TS3W.exe 0x00509790: query two textures and draw at native size with supplied UVs.
void DrawPairNativeUv(DrawContext*,float,float,void*,void*,const Rect*);
// TS3W.exe 0x00509830: query two textures and draw with supplied bounds and UVs.
void DrawPairRect(DrawContext*,const Rect*,void*,void*,const Rect*);
// TS3W.exe 0x00509A70: retain the UI service and initialize first-use rendering state.
uint8_t Initialize(void*);
// TS3W.exe 0x00509D30: allocate and construct a window, returning its adjusted interface.
void* MakeWindow(uint32_t,void*);
// TS3W.exe 0x00509D80: pop a renderable node under the shared mutex, allocating on empty.
void* AllocateRenderable();
// TS3W.exe 0x00509F10: pop a display node or allocate it and initialize only its embedded list.
void* AllocateDisplay();
// TS3W.exe 0x00509FA0: query the two supported cell-map interface IDs (verified query twin).
void* QueryMap(void*,uint32_t);
// TS3W.exe 0x00509FD0: bound-check a cell and return lower-bound insertion-index parity.
uint8_t CellParity(CellMap*,const int32_t*);
// TS3W.exe 0x0050A190: query the two supported drawing interface IDs (verified query twin).
void* QueryDraw(void*,uint32_t);
// TS3W.exe 0x0050A230: query texture IDs, adjusting the auxiliary interface by 1C.
void* QueryTexture(Texture*,uint32_t);
// TS3W.exe 0x0050A270: initialize a borrowed iterator and return the owner's cursor.
void* BeginIterator(void*,Iterator*);
}

#if defined(_M_IX86)
namespace ts3::ui::b04 {
inline void __fastcall HookDrawNative(DrawContext* s,void*,float x,float y,void* a) { return DrawNative(s,x,y,a); }
inline void __fastcall HookDrawNativeUv(DrawContext* s,void*,float x,float y,void* a,const Rect* uv) { return DrawNativeUv(s,x,y,a,uv); }
inline void __fastcall HookDrawRect(DrawContext* s,void*,const Rect* r,void* a,const Rect* uv) { return DrawRect(s,r,a,uv); }
inline void __fastcall HookDrawPairNative(DrawContext* s,void*,float x,float y,void* a,void* b) { return DrawPairNative(s,x,y,a,b); }
inline void __fastcall HookDrawPairNativeUv(DrawContext* s,void*,float x,float y,void* a,void* b,const Rect* uv) { return DrawPairNativeUv(s,x,y,a,b,uv); }
inline void __fastcall HookDrawPairRect(DrawContext* s,void*,const Rect* r,void* a,void* b,const Rect* uv) { return DrawPairRect(s,r,a,b,uv); }
inline void* __stdcall HookCreateWindow(uint32_t ignored,void* a) { return MakeWindow(ignored,a); }
inline void* __fastcall HookQueryMap(void* s,void*,uint32_t id) { return QueryMap(s,id); }
inline uint8_t __fastcall HookCellParity(CellMap* s,void*,const int32_t* cell) { return CellParity(s,cell); }
inline void* __fastcall HookQueryDraw(void* s,void*,uint32_t id) { return QueryDraw(s,id); }
inline void* __fastcall HookQueryTexture(Texture* s,void*,uint32_t id) { return QueryTexture(s,id); }
inline void* __fastcall HookBeginIterator(void* s,void*,Iterator* out) { return BeginIterator(s,out); }
}
#endif
