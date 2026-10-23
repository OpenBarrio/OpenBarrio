#pragma once
#include "ts3/layout.h"
namespace ts3::audio::b43impl {
struct Value {
    uint32_t header[2];
    uint8_t flags, pad;
    uint16_t type;
    uint32_t reserved;
    union { uint32_t words[4]; struct { void* data; uint32_t unused, count; } array; } payload;
};
struct OwnerView { void** vtable; void* first; void* reserved; void* owner; };
struct Scene {
    void* vtable; void* secondary; uint32_t state; void* storage;
    uint8_t gap[12]; void* inlineStorage;
    uint8_t tail[0x36c-0x20]; void* resource;
};
TS3_X86_OFFSET(Value, payload, 0x10);
TS3_X86_OFFSET(Scene, resource, 0x36c);
// TS3W.exe 0x008A71E0: Release scene resource/storage and optionally link the scene into its free list.
Scene* Destroy(Scene*, uint32_t flags);
// TS3W.exe 0x008A7240: Return positive floating zero, ignoring both arguments.
float Zero(void*, uint32_t, uint32_t);
// TS3W.exe 0x008A7250: Return the static empty name.
const char* EmptyName(void*, uint32_t);
// TS3W.exe 0x008A7260: Read four words from a type-1000 property.
bool ReadFour(void*, uint32_t, uint32_t*);
// TS3W.exe 0x008A72C0: Read two words from a type-1000 property.
bool ReadTwo(void*, uint32_t, uint32_t*);
// TS3W.exe 0x008A7320: Read words zero, one and three from a type-1000 property.
bool ReadSplit(void*, uint32_t, uint32_t*, uint32_t*);
// TS3W.exe 0x008A7370: Read a type-13 array; re-read flags after publishing count.
bool Read13(void*, uint32_t, uint32_t*, void**);
// TS3W.exe 0x008A73E0: Read a type-10 array; re-read flags after publishing count.
bool Read10(void*, uint32_t, uint32_t*, void**);
// TS3W.exe 0x008A7450: Read a type-1000 array; re-read flags after publishing count.
bool Read1000(void*, uint32_t, uint32_t*, void**);
// TS3W.exe 0x008A74C0: Test the supported type conversion pairs.
uint32_t Compatible(void*, uint32_t, uint32_t);
// TS3W.exe 0x008A7500: Return the resource configuration class ID.
uint32_t ClassId(void*);
// TS3W.exe 0x008A7510: Enumerate three supported types, with all-or-nothing capacity handling.
uint32_t Types(void*, uint32_t*, uint32_t);
// TS3W.exe 0x008A7540: Store the adjusted owner then forward to the shared binding method.
void* Bind(OwnerView*, void*, void*);
}
// ABI-only member view for the generated thiscall hook adapters. No native data is
// accessed through this class; the portable implementation uses typed views above.
namespace ts3::audio {
class b43 {
public:
    void* Destroy(uint32_t flags) { return b43impl::Destroy(reinterpret_cast<b43impl::Scene*>(this),flags); }
    void* EmptyName(uint32_t ignored) { return const_cast<char*>(b43impl::EmptyName(this,ignored)); }
    bool ReadFour(uint32_t k,void* p) { return b43impl::ReadFour(this,k,static_cast<uint32_t*>(p)); }
    bool ReadTwo(uint32_t k,void* p) { return b43impl::ReadTwo(this,k,static_cast<uint32_t*>(p)); }
    bool ReadSplit(uint32_t k,void* p,void* q) { return b43impl::ReadSplit(this,k,static_cast<uint32_t*>(p),static_cast<uint32_t*>(q)); }
    bool Read13(uint32_t k,void* p,void* q) { return b43impl::Read13(this,k,static_cast<uint32_t*>(p),static_cast<void**>(q)); }
    bool Read10(uint32_t k,void* p,void* q) { return b43impl::Read10(this,k,static_cast<uint32_t*>(p),static_cast<void**>(q)); }
    bool Read1000(uint32_t k,void* p,void* q) { return b43impl::Read1000(this,k,static_cast<uint32_t*>(p),static_cast<void**>(q)); }
    uint32_t Compatible(uint32_t a,uint32_t b) { return b43impl::Compatible(this,a,b); }
    uint32_t ClassId() { return b43impl::ClassId(this); }
    uint32_t Types(void* p,uint32_t n) { return b43impl::Types(this,static_cast<uint32_t*>(p),n); }
    void* Bind(void* p,void* q) { return b43impl::Bind(reinterpret_cast<b43impl::OwnerView*>(this),p,q); }
};
}
