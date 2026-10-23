#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::m58 {
// TS3W.exe 0x00570E70: Allocate 0x88 bytes and construct the scratch controller, returning null on allocation failure.
void* CreateScratch(void* host);
// TS3W.exe 0x00570EA0: Allocate 0x60 bytes and construct an identity transform controller.
void* CreateTransform(void* host);
// TS3W.exe 0x00570ED0: Allocate 0x70 bytes and construct a sequence controller with mode -1.
void* CreateSequence(void* host);
// TS3W.exe 0x00570F10: Allocate 0x2C bytes and construct the animation controller.
void* CreateAnimation(void* host);
// TS3W.exe 0x00570F40: Construct the base controller and replace its three interface tables, returning this.
Controller* Construct(Controller*,void* host);
// TS3W.exe 0x00570F70: Return the constant secondary-interface type ID.
uint32_t Type(void*);
#if defined(_M_IX86)
namespace hooks {
inline void* __fastcall CreateScratch(void*,void*,void* h) { return m58::CreateScratch(h); }
inline void* __fastcall CreateTransform(void*,void*,void* h) { return m58::CreateTransform(h); }
inline void* __fastcall CreateSequence(void*,void*,void* h) { return m58::CreateSequence(h); }
inline void* __fastcall CreateAnimation(void*,void*,void* h) { return m58::CreateAnimation(h); }
inline Controller* __fastcall Construct(Controller* p,void*,void* h) { return m58::Construct(p,h); }
inline uint32_t __fastcall Type(void* p,void*) { return m58::Type(p); }
}
#endif
}
