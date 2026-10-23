#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::m81 {
struct Range { unsigned char* begin; unsigned char* end; unsigned char* capacity; };
struct JointRange : Range { void* descriptor; };
// Opaque original IK records are consumed only by original-game bridges.
struct Group { uint32_t prefix[2]; Range records; void* descriptor; unsigned char storage[0xaa0]; };
TS3_X86_OFFSET(Group,records,8);
TS3_X86_SIZE(Group,0xab8);
// TS3W.exe 0x00578870: Resize the 0x70-byte record range, retaining the helper's EAX on growth.
uint32_t Resize(JointRange*,uint32_t);
// TS3W.exe 0x00578910: Forward four arguments to IK binding with a zero inserted before the last pair.
bool Bind(ClipIkInfo*,uint32_t,uint32_t,void*,void*);
// TS3W.exe 0x00578A00: Restore the IK vtable and free hash, namespace and slot arrays in that order.
void Destruct(ClipIkInfo*);
// TS3W.exe 0x00578B80: Exchange two IK record ranges through an inline temporary, including self-exchange.
void Exchange(Range*,Range*);
// TS3W.exe 0x00578C60: Exchange groups then visit every record, accumulating low-byte success without short circuiting.
bool ResolveGroups(Group*,Group*);
// TS3W.exe 0x00578D60: Lazily resolve two group pairs; the visited bit remains set even after failure.
bool Resolve(ClipIkInfo*);
}
#if defined(_M_IX86)
namespace ts3::anim::m81 {
inline uint32_t __fastcall HookResize(JointRange* s,void*,uint32_t n) { return Resize(s,n); }
inline bool __fastcall HookBind(ClipIkInfo* s,void*,uint32_t a,uint32_t b,void* c,void* d) { return Bind(s,a,b,c,d); }
inline void __fastcall HookDestruct(ClipIkInfo* s,void*) { Destruct(s); }
inline bool __fastcall HookResolveGroups(Group* a,void*,Group* b) { return ResolveGroups(a,b); }
inline bool __fastcall HookResolve(ClipIkInfo* s,void*) { return Resolve(s); }
}
#endif
