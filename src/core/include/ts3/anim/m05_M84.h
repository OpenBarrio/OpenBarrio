#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::m84impl {
struct BindingMap { void* header; void* sentinel[2]; void* allocation; uintptr_t rest[3]; };
struct Clip { ResourcePrefix base; uint8_t loaded; uint8_t padding[3]; BindingMap map; void* ik; void* view; void* raw; void* events[4]; };
TS3_X86_OFFSET(Clip,map,0x2c);
TS3_X86_OFFSET(Clip,view,0x4c);
// TS3W.exe 0x0057ACE0: Allocate and construct a clip resource, returning null on allocation failure.
void* __fastcall CreateClip(void*,void*);
// TS3W.exe 0x0057AD10: Destroy clip members in order and optionally free, returning this.
void* __fastcall DeleteClip(Clip*,void*,uint32_t);
// TS3W.exe 0x0057ADF0: Look up or generate clip bindings under the global lock; repeats lookup.
void* __fastcall BindClip(Clip*,void*,void*);
// TS3W.exe 0x0057B300: Construct the rig factory base and install its vtable.
void* __fastcall ConstructFactory(void*,void*);
// TS3W.exe 0x0057B3B0: Resolve named rig joints, preserving unvisited cached indices.
int32_t __fastcall ResolveRig(RigResource*,void*);
// TS3W.exe 0x0057B590: Destroy the rig factory and optionally free, returning this even after free.
void* __fastcall DeleteFactory(void*,void*,uint32_t);
}

namespace ts3::anim {
// ABI facade used by generated thiscall hook adapters; free fastcall entries above
// are also callable directly by differential tests.
struct m84 {
 uintptr_t CreateClip() { return reinterpret_cast<uintptr_t>(m84impl::CreateClip(this,nullptr)); }
 uintptr_t DeleteClip(uint32_t flags) { return reinterpret_cast<uintptr_t>(m84impl::DeleteClip(reinterpret_cast<m84impl::Clip*>(this),nullptr,flags)); }
 uintptr_t BindClip(void* key) { return reinterpret_cast<uintptr_t>(m84impl::BindClip(reinterpret_cast<m84impl::Clip*>(this),nullptr,key)); }
 uintptr_t ConstructFactory() { return reinterpret_cast<uintptr_t>(m84impl::ConstructFactory(this,nullptr)); }
 uint32_t ResolveRig() { return static_cast<uint32_t>(m84impl::ResolveRig(reinterpret_cast<RigResource*>(this),nullptr)); }
 uintptr_t DeleteFactory(uint32_t flags) { return reinterpret_cast<uintptr_t>(m84impl::DeleteFactory(this,nullptr,flags)); }
};
}
