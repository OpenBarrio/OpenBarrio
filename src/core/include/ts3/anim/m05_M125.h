#pragma once
#include "ts3/layout.h"
#include "ts3/math/float_values.h"
namespace ts3::anim::m125_impl {
using ts3::math::Float4Value;
struct Link { Link* next; Link* previous; };
struct StyleList { void** vtable; uintptr_t opaque[2]; Link children; };
struct StyleNode { void** vtable; Link link; };
struct Entity { void** vtable; volatile int32_t references; };
TS3_X86_OFFSET(StyleList, children, 0x0c);
TS3_X86_OFFSET(StyleNode, link, 4);
TS3_X86_OFFSET(Entity, references, 4);
// TS3W.exe 0x008468C0: Find the routing level at a position; only the predicate's low byte is tested.
int32_t PositionLevel(const Float4Value* position);
// TS3W.exe 0x008469B0: Query list children for interface 0x065E0C92 and return the first match.
void* FindStyle(StyleList* self);
// TS3W.exe 0x008469F0: Accumulate parent height differences with per-step float rounding and EA ref transfers.
double HeightDifference(Entity* entity, uint32_t idLow, uint32_t idHigh);
// TS3W.exe 0x00846AE0: Overwrite the output with each ancestor's vector, consuming the input reference.
Float4Value* AncestorPosition(Float4Value* out, Entity* entity);
// TS3W.exe 0x00846B70: Find the first footprint or nearby body obstruction, returning its full EDX:EAX ID.
uint64_t FindObstruction(Entity*,void*,const Float4Value*,const uint64_t*,int);
// TS3W.exe 0x00847390: Update the host obstruction state, preserving reference consumption and EAX residue.
uintptr_t UpdateObstruction(void*,int);
}

namespace ts3::anim {
// ABI-only facade: registration maps thiscall to this member; the other entries are cdecl.
struct m125 {
    uintptr_t UpdateObstruction(int unused) { return m125_impl::UpdateObstruction(this,unused); }
    static uint64_t FindObstruction(m125_impl::Entity* e,void* b,const math::Float4Value* p,const uint64_t* ids,int n) { return m125_impl::FindObstruction(e,b,p,ids,n); }
    void* FindStyle() { return m125_impl::FindStyle(reinterpret_cast<m125_impl::StyleList*>(this)); }
    static int32_t PositionLevel(const math::Float4Value* p) { return m125_impl::PositionLevel(p); }
    static double HeightDifference(m125_impl::Entity* p,uint32_t lo,uint32_t hi) { return m125_impl::HeightDifference(p,lo,hi); }
    static math::Float4Value* AncestorPosition(math::Float4Value* out,m125_impl::Entity* p) { return m125_impl::AncestorPosition(out,p); }
};
}
