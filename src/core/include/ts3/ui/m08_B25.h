#pragma once
#include "ts3/layout.h"
namespace ts3::ui::b25 {
template<class R,class T,class... A> using BridgeThis=R (__fastcall*)(T*,void*,A...);
template<class R,class... A> using BridgeCdecl=R (__cdecl*)(A...);
struct Text { char16_t *begin,*end,*capacity; void* allocator; };
// Sparse native views. Unknown spans are not owned storage or inferred class tails.
struct Combo {
    void* primary; void* secondary; uint8_t unknown08[0x28]; void* manager;
    uint32_t unknown34,source38,source3C; float bounds[4];
    uint8_t unknown50[0x3C]; uint32_t fontKey; uint8_t unknown90[0x16C];
    uint32_t color1FC,color200; void* skin; uint8_t unknown208[0x48];
    void* choices; void* popupInterface; int32_t hover,pressed; uint32_t selected;
    uint32_t unknown264,color268; uint8_t unknown26C[0x14]; uint32_t disabledColor;
    uint32_t unknown284,flags,orientation; uint8_t dirty,opened; uint8_t unknown292[2];
    Text *begin,*end,*capacity; uint32_t unknown2A0[3]; float rect[3][4];
    uint8_t unknown2DC[0x14]; float inset[4]; void* popup; void* observer;
};
struct Drawable { void* primary; void* secondary; void* unknown08; void* tertiary; void* unknown10; void* texture[2]; };
struct Descriptor { void* table; Drawable* owner; uint32_t count,value; };
struct Message { void *unknown0,*unknown4; uint32_t kind,source,value; uint32_t unknown14[2]; };
TS3_X86_OFFSET(Combo,manager,0x30);
TS3_X86_OFFSET(Combo,skin,0x204);
TS3_X86_OFFSET(Combo,choices,0x250);
TS3_X86_OFFSET(Combo,selected,0x260);
TS3_X86_OFFSET(Combo,flags,0x288);
TS3_X86_OFFSET(Combo,begin,0x294);
TS3_X86_OFFSET(Combo,rect,0x2AC);
TS3_X86_OFFSET(Combo,popup,0x300);
TS3_X86_OFFSET(Drawable,texture,0x14);
struct Dependencies {
    BridgeThis<void*,void,uint32_t,uint32_t> renderer=nullptr;
    BridgeCdecl<void*,uint32_t> fontService=nullptr,otherService=nullptr;
    BridgeThis<void*,void,uint32_t,uint32_t> lookupFont=nullptr;
    BridgeCdecl<uintptr_t,void*,const float*,uint32_t,uint32_t,float> frame=nullptr;
    BridgeThis<void*,Drawable,uint32_t> query=nullptr,destruct=nullptr;
};
Dependencies& Bridges();
// TS3W.exe 0x0051BD20: Draw combo layers and selected text, returning AL=true.
uint8_t __fastcall Draw(Combo*,void*,void*);
// TS3W.exe 0x0051BF90: Draw a skin layer or the unskinned background.
void __fastcall DrawLayer(Combo*,void*,void*,uint32_t);
// TS3W.exe 0x0051C0B0: Hit-test half-open rectangles in arrow/text/background priority.
int32_t __fastcall Hit(Combo*,void*,float,float);
// TS3W.exe 0x0051C160: Recalculate dirty skin rectangles, preserving EA's fallback sizing.
void __fastcall Layout(Combo*,void*);
// TS3W.exe 0x0051C3F0: Mark the popup open once and dispatch its borrowed notification.
void __fastcall Open(void*,void*,void*);
// TS3W.exe 0x0051C450: Close the popup and synchronously detach observers/focus/parent.
void __fastcall Close(void*,void*,void*,uint32_t);
// TS3W.exe 0x0051C620: Return a choice's UTF16 begin pointer, or null for an invalid index.
char16_t* __fastcall Item(void*,void*,uint32_t);
// TS3W.exe 0x0051C650: Return the signed vector element count.
int32_t __fastcall Count(void*,void*);
// TS3W.exe 0x0051C660: Clamp selection and optionally notify; same-index skips even clamping.
void __fastcall Select(void*,void*,uint32_t,uint8_t);
// TS3W.exe 0x0051C710: Fill a drawable descriptor and return secondary slot 5's value.
uint32_t __fastcall Describe(void*,void*,Descriptor*);
// TS3W.exe 0x0051C740: Draw a texture slice or the default frame/triangle.
uintptr_t __fastcall DrawPart(Drawable*,void*,void*,const float*,const uint32_t*);
// TS3W.exe 0x0051C9F0: Adjust tertiary receiver to primary and forward query.
void* __fastcall Query12(void*,void*,uint32_t);
// TS3W.exe 0x0051CA00: Adjust secondary receiver to primary and forward deleting destructor.
void* __fastcall Delete4(void*,void*,uint32_t);
// TS3W.exe 0x0051CA10: Adjust tertiary receiver to primary and forward deleting destructor.
void* __fastcall Delete12(void*,void*,uint32_t);
// TS3W.exe 0x0051CA20: Adjust secondary receiver to primary and forward query.
void* __fastcall Query4(void*,void*,uint32_t);
}
