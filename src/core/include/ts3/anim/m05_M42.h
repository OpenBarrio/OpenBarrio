#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m42_impl {
struct Entry { uint32_t lo,hi,a,b; };
struct RandomController {
    void* vtable; unsigned char opaque04[0x28];
    void* child; void* clip; void* retained;
    unsigned char opaque38[0x10];
    Entry *entries,*entriesEnd,*entriesCapacity; uint32_t entriesDescriptor;
    float *weights,*weightsEnd,*weightsCapacity; uint32_t weightsDescriptor;
    float *times,*timesEnd,*timesCapacity; uint32_t timesDescriptor;
    unsigned char opaque78[12]; uint8_t enabled;
};
struct Cursor { void* vtable; uint32_t opaque04; void* stream; uint32_t opaque0c; uint32_t mode; uint8_t good; };
TS3_X86_OFFSET(RandomController,child,0x2c);
TS3_X86_OFFSET(RandomController,entries,0x48);
TS3_X86_OFFSET(RandomController,enabled,0x84);
TS3_X86_OFFSET(Cursor,good,0x14);
// TS3W.exe 0x0056CC90: Destroy and optionally free; returns this even after free.
void* DeleteTransform(void*,uint32_t);
// TS3W.exe 0x0056CD60: Set notification latch to one; EAX unspecified.
void Enable(RandomController*);
// TS3W.exe 0x0056CD80: Forward slot 3 to the optional child, otherwise return zero.
uintptr_t Query(RandomController*,uint32_t);
// TS3W.exe 0x0056CDA0: Return enabled child time or the global disabled time.
float Time(RandomController*);
// TS3W.exe 0x0056CDD0: Retain replacement before releasing old and propagate to clip; EAX unspecified.
void SetRetained(RandomController*,void*);
// TS3W.exe 0x0056CE20: Serialize random controller state, stopping on the first stream failure.
bool Save(RandomController*,Cursor*);
}
namespace ts3::anim {
struct m42 : m42_impl::RandomController {
 void Enable() { m42_impl::Enable(this); }
 uintptr_t Query(uint32_t a) { return m42_impl::Query(this,a); }
 float Time() { return m42_impl::Time(this); }
 void SetRetained(void* p) { m42_impl::SetRetained(this,p); }
 bool Save(void* p) { return m42_impl::Save(this,static_cast<m42_impl::Cursor*>(p)); }
};
}
