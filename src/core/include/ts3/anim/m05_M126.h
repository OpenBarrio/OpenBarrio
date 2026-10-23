#pragma once
#include <cstdint>
#include <cstddef>
#include "ts3/layout.h"
namespace ts3::anim::m126_impl {
struct Link { Link* next; Link* previous; };
struct Style { void* table; void* opaque[2]; Link link; };
#if defined(_M_IX86)
static_assert(offsetof(Style,link)==0x0c);
#endif
// TS3W.exe 0x00847710: constructs the style interface using the base constructor.
Style* Construct(Style*, Link*);
// TS3W.exe 0x00847730: returns the style interface identifier.
uint32_t Type(Style*);
// TS3W.exe 0x00847740: returns this for the base or style interface identifier.
void* Query(Style*, uint32_t);
// TS3W.exe 0x00847770: resets the vtable and tail-calls the base destructor, passing EAX through.
uintptr_t Destroy(Style*);
// TS3W.exe 0x00847780: destroys and optionally frees this, returning this even after free.
Style* Delete(Style*, uint32_t);
// TS3W.exe 0x008477B0: returns the first nonnull query result in the intrusive list.
void* Find(Style*);
}
namespace ts3::anim {
// ABI member facade for the generated thiscall adapters.
struct m126 {
    void* Construct(void* p) { return m126_impl::Construct(reinterpret_cast<m126_impl::Style*>(this),static_cast<m126_impl::Link*>(p)); }
    uint32_t Type() { return m126_impl::Type(reinterpret_cast<m126_impl::Style*>(this)); }
    void* Query(uint32_t id) { return m126_impl::Query(reinterpret_cast<m126_impl::Style*>(this),id); }
    uint32_t Destroy() { return static_cast<uint32_t>(m126_impl::Destroy(reinterpret_cast<m126_impl::Style*>(this))); }
    void* Delete(uint32_t flags) { return m126_impl::Delete(reinterpret_cast<m126_impl::Style*>(this),flags); }
    void* Find() { return m126_impl::Find(reinterpret_cast<m126_impl::Style*>(this)); }
};
}
