#pragma once
#include "ts3/anim/m05_layouts.h"
#include "ts3/anim/m05_B01.h"
#include "ts3/anim/m05_B09.h"
namespace ts3::anim::m38 {
struct Blend { Controller base; Controller* first; Controller* second; uint32_t value30; float weight; };
TS3_X86_OFFSET(Blend,first,0x28);
TS3_X86_OFFSET(Blend,second,0x2c);
TS3_X86_OFFSET(Blend,weight,0x34);
// TS3W.exe 0x0056BE20: Adjust secondary-interface this by -0x0c and delete the blend.
Blend* DeleteSecondary(void*,uint32_t);
// TS3W.exe 0x0056BE30: Release second then first, restoring last refs to one before virtual deletion.
void Destroy(Blend*);
// TS3W.exe 0x0056C0B0: Evaluate positive-weight children and combine their nodes in first/second order.
b01::Node* Evaluate(Blend*,void*);
// TS3W.exe 0x0056C220: Load versions zero or one, two retained controllers and two scalar fields.
uint8_t Load(Blend*,b09impl::Reader*);
// TS3W.exe 0x0056C2E0: Destroy and optionally free, returning this even after free.
Blend* Delete(Blend*,uint32_t);
// TS3W.exe 0x0056C550: Adjust secondary-interface this by -0x0c and forward to the original deleting destructor.
void* DeleteOtherSecondary(void*,uint32_t);
}
#if defined(_M_IX86)
namespace ts3::anim::m38::hooks {
Blend* __fastcall DeleteSecondary(void*,void*,uint32_t);
void __fastcall Destroy(Blend*,void*);
b01::Node* __fastcall Evaluate(Blend*,void*,void*);
uint8_t __fastcall Load(Blend*,void*,b09impl::Reader*);
Blend* __fastcall Delete(Blend*,void*,uint32_t);
void* __fastcall DeleteOtherSecondary(void*,void*,uint32_t);
}
#endif
