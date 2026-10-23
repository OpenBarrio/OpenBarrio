#pragma once
#include "ts3/anim/m05_B01.h"
namespace ts3::anim::m17 {
struct alignas(16) Entry { Float4Value translation,rotation; int32_t bone; unsigned char opaque[12]; };
struct Trans { b01::Node base; Entry *begin,*end,*capacity; void* descriptor; };
struct alignas(16) Constant { b01::Node base; unsigned char opaque[12]; Float4Value translation,rotation; };
struct Blend { b01::Node base; void *a,*b; uint32_t untouched; void *c,*d; float f38,f3c,f40,f44,f48; void *e,*f; uint8_t flag54,flag55; uint16_t untouched56; float f58; };
struct Context { unsigned char opaque[0x54]; uint8_t flags; };
TS3_X86_OFFSET(Trans,begin,0x24);
TS3_X86_OFFSET(Constant,translation,0x30);
TS3_X86_OFFSET(Blend,f58,0x58);
// TS3W.exe 0x0055D150: Evaluate first child (or reset pose), then overwrite selected translations and rotations.
void Evaluate(Trans*,Context*,RigPose*);
// TS3W.exe 0x0055D3B0: Construct a translation-list evaluator without initializing sibling links.
Trans* ConstructTrans(Trans*,void*);
// TS3W.exe 0x0055D420: Release translation storage, reset base and optionally free; return this even after free.
Trans* DestroyTrans(Trans*,uint32_t);
// TS3W.exe 0x0055D790: Construct a constant transform evaluator with identity quaternion.
Constant* ConstructConstant(Constant*,void*);
// TS3W.exe 0x0055D7F0: Reset base and optionally free the constant evaluator, returning this.
Constant* DestroyConstant(Constant*,uint32_t);
// TS3W.exe 0x0055DED0: Construct blend state, preserving uninitialized holes.
Blend* ConstructBlend(Blend*,void*);
}

#if defined(_M_IX86)
namespace ts3::anim::m17::hooks {
void __fastcall Evaluate(Trans* n,void*,Context* c,RigPose* p);
Trans* __fastcall ConstructTrans(Trans* n,void*,void* h);
Trans* __fastcall DestroyTrans(Trans* n,void*,uint32_t f);
Constant* __fastcall ConstructConstant(Constant* n,void*,void* h);
Constant* __fastcall DestroyConstant(Constant* n,void*,uint32_t f);
Blend* __fastcall ConstructBlend(Blend* n,void*,void* h);
}
#endif
