#pragma once
#include "ts3/layout.h"
#include <cstdint>
#ifdef DrawText
#undef DrawText
#endif
namespace ts3::ui::b02 {
struct Ref { void** table; };
struct Entry { Ref* object; uintptr_t opaque; };
struct Selector { void** table; uintptr_t pad; Ref* selected; uint32_t value; uintptr_t pad10; Entry* begin; Entry* end; uintptr_t rest[19]; void* fallback; void* overrideValue; };
TS3_X86_OFFSET(Selector,begin,0x14);
TS3_X86_OFFSET(Selector,fallback,0x68);
struct Transfer { void** table; void* value; };
struct Buffer { void** table; void* renderable; void* owner; uint32_t color; uint8_t clipping; uint8_t padding[3]; float rect[4]; void* words[3]; const char* label; };
TS3_X86_OFFSET(Buffer,rect,0x14);
TS3_X86_OFFSET(Buffer,label,0x30);
struct Text { uint8_t pad[0x34]; char16_t* begin; char16_t* end; uintptr_t tail[10]; void* style; };
TS3_X86_OFFSET(Text,style,0x64);
struct StackRecord { Ref* first; Ref* second; uintptr_t opaque; uint32_t count; uint32_t tail; };
struct Stack { uintptr_t pad[8]; StackRecord* end; };
struct Pool { Ref** begin; Ref** end; uintptr_t capacity; uintptr_t tag; uint32_t active; uintptr_t primary[4]; uintptr_t secondary[4]; uintptr_t pad34; uintptr_t lock[6]; };
TS3_X86_OFFSET(Pool,lock,0x38);
// TS3W.exe 0x00505620: choose the first accepting interface and deactivate the others (AL predicate).
uintptr_t Select(Selector*);
// TS3W.exe 0x00505690: retain the new global interface before releasing the previous one.
Ref* SetCurrent(Ref*);
// TS3W.exe 0x005056D0: read the global interface without a null guard.
Ref* Current();
// TS3W.exe 0x005057F0: store the override and rerun selection.
uintptr_t Override(Selector*,void*);
// TS3W.exe 0x00505B80: transfer a stored pointer and clear the slot.
void* Take(Transfer*);
// TS3W.exe 0x00505DA0: perform guarded one-time font configuration through original services.
void Initialize();
// TS3W.exe 0x00505EA0: return the borrowed singleton (exact twin of 004D8C80).
void* Singleton();
// TS3W.exe 0x00505EB0: adjust the top record count and release/pop it only at zero.
void Adjust(Stack*,uint32_t);
// TS3W.exe 0x00506190: copy clipping coordinates and return the output address with AL replaced by the flag.
uintptr_t GetClip(Buffer*,float*);
// TS3W.exe 0x005061C0: store AL; if nonzero copy coordinates and return their pointer, otherwise only AL is defined.
uintptr_t SetClip(Buffer*,uint32_t,const float*);
// TS3W.exe 0x005061F0: intersect the clipping rectangle (unordered comparisons remain fall-through).
const float* Intersect(Buffer*,const float*);
// TS3W.exe 0x005062C0: forward UTF16 begin/style/signed code-unit length to slot 12.
uintptr_t DrawText(Buffer*,Text*);
// TS3W.exe 0x005064F0: initialize the sparse GDEBuffer without touching padding.
Buffer* Construct(Buffer*,void*);
// TS3W.exe 0x005065B0: acquire a renderable from the pool under its original lock.
Ref* Acquire(Pool*);
// TS3W.exe 0x00506A30: draw four border strips using the clipped thickness.
void Border(Buffer*,float,float,float,float,float);
}

namespace ts3::ui::b02::hooks {
uintptr_t __fastcall Select(Selector* s,void*);
uintptr_t __fastcall Override(Selector* s,void*,void* p);
void* __fastcall Take(Transfer* s,void*);
void __fastcall Adjust(Stack* s,void*,uint32_t d);
uintptr_t __fastcall GetClip(Buffer* s,void*,float* p);
uintptr_t __fastcall SetClip(Buffer* s,void*,uint32_t f,const float* p);
const float* __fastcall Intersect(Buffer* s,void*,const float* p);
uintptr_t __fastcall DrawText(Buffer* s,void*,Text* t);
Buffer* __fastcall Construct(Buffer* s,void*,void* p);
Ref* __fastcall Acquire(Pool* s,void*);
void __fastcall Border(Buffer* s,void*,float a,float b,float c,float d,float t);
}
