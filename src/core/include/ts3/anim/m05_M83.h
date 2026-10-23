#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::m83_detail {
// The scaffold stops at +60; this constructor also initializes the +64 allocator word.
struct Map { void* left; void* right; uint32_t count; uint32_t color; void* extra; void* descriptor; };
struct Clip { ResourcePrefix base; uint8_t loaded; uint8_t padding[3]; uint32_t opaque2c; Map map; ClipIkInfo* ik; void* view; void* rawBuffer; void* eventBegin; void* eventEnd; void* eventCapacity; void* eventDescriptor; };
struct Resource { Clip clip; int32_t allocator; };
static_assert(sizeof(void*)!=4 || sizeof(Resource)==0x68);
// TS3W.exe 0x0057A470: Convert a nullable UTF-16 string to UTF-8 then calculate the identifier pair.
uint64_t Identifier(const char16_t* text, uint32_t seed);
// TS3W.exe 0x0057A4E0: Construct the clip resource factory base and install its vtable.
void* Factory(void* self);
// TS3W.exe 0x0057A540: Reload the clip buffer, IK metadata and appended event list.
uintptr_t Load(Clip* self, void* stream);
// TS3W.exe 0x0057A6B0: Destroy the factory and optionally free it, returning this even after free.
void* DeleteFactory(void* self, uint32_t flags);
// TS3W.exe 0x0057A6E0: Retain an opened stream, load the clip and release the stream.
bool Open(void* source, Clip* self);
// TS3W.exe 0x0057AB10: Initialize the resource without touching padding or the opaque +2C word.
Resource* Construct(Resource* self);
}

namespace ts3::anim {
// ABI facade used by the mechanically generated thiscall adapters.
struct m83 {
 static uint64_t Identifier(const char16_t* t,uint32_t s) { return m83_detail::Identifier(t,s); }
 uint32_t Factory() { return uint32_t(reinterpret_cast<uintptr_t>(m83_detail::Factory(this))); }
 uint32_t DeleteFactory(uint32_t f) { return uint32_t(reinterpret_cast<uintptr_t>(m83_detail::DeleteFactory(this,f))); }
 uint32_t Construct() { return uint32_t(reinterpret_cast<uintptr_t>(m83_detail::Construct(reinterpret_cast<m83_detail::Resource*>(this)))); }
 uint32_t Load(void* stream) { return uint32_t(m83_detail::Load(reinterpret_cast<m83_detail::Clip*>(this),stream)); }
 static bool Open(void* source,void* resource) { return m83_detail::Open(source,static_cast<m83_detail::Clip*>(resource)); }
};
}
