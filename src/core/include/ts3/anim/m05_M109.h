#pragma once
#include "ts3/layout.h"
#include "ts3/math/float_values.h"
namespace ts3::anim::m109 {
using ts3::math::Float4Value;
struct Key { uint32_t word[4]; };
struct NodeKey { Key key; unsigned char opaque[0x18]; NodeKey* next; };
struct NodeLarge { uint32_t key; unsigned char opaque[0xf90]; NodeLarge* next; };
struct NodeSmall { uint32_t key; unsigned char opaque[0x60]; NodeSmall* next; };
struct Pair { void* node; void** bucket; };
struct Table {
    uint32_t functors; void** buckets; uint32_t count;
    // TS3W.exe 0x0083B8A0: Find an integer key in the large-node table, returning the end sentinel on a miss.
    Pair* FindLarge(Pair*,const uint32_t*);
    // TS3W.exe 0x0083B900: Find an integer key in the small-node table, returning the end sentinel on a miss.
    Pair* FindSmall(Pair*,const uint32_t*);
    // TS3W.exe 0x0083B960: Find a four-word key hashed by word0 XOR word3.
    Pair* FindKey(Pair*,const Key*);
};
// TS3W.exe 0x0083B4D0: Search a chain by four-word key, ignoring the supplied cached hash.
NodeKey* __stdcall Chain(NodeKey*,const Key*,uint32_t);
struct Callback {
    void* vtable; uint32_t opaque; uint8_t fired; unsigned char pad[3];
    void* context;
#if defined(_M_IX86)
    uintptr_t (__thiscall* call)(void*,uint32_t);
#else
    uintptr_t (*call)(void*,uint32_t);
#endif
    uint32_t* argument;
    // TS3W.exe 0x0083BA00: Invoke the bound callback, then set its fired latch, preserving its result.
    uintptr_t Invoke();
};
struct alignas(16) Shape { void** vtable; unsigned char opaque[0x60-sizeof(void*)]; Float4Value extent; };
struct Entity { void** vtable; void* opaque; void** secondary; };
// TS3W.exe 0x0083B230: Transfer source shape dimensions and entity transform to the destination shape; only AL is boolean.
bool Transfer(Entity*,Entity*);
TS3_X86_OFFSET(NodeLarge,next,0xf94);
TS3_X86_OFFSET(NodeSmall,next,0x64);
TS3_X86_OFFSET(NodeKey,next,0x28);
TS3_X86_OFFSET(Table,buckets,4);
TS3_X86_OFFSET(Callback,argument,0x14);
}
namespace ts3::anim {
struct m109_hooks {
    uint32_t FindLarge(void* out,void* key) { return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(reinterpret_cast<m109::Table*>(this)->FindLarge(static_cast<m109::Pair*>(out),static_cast<uint32_t*>(key)))); }
    uint32_t FindSmall(void* out,void* key) { return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(reinterpret_cast<m109::Table*>(this)->FindSmall(static_cast<m109::Pair*>(out),static_cast<uint32_t*>(key)))); }
    uint32_t FindKey(void* out,void* key) { return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(reinterpret_cast<m109::Table*>(this)->FindKey(static_cast<m109::Pair*>(out),static_cast<m109::Key*>(key)))); }
    uint32_t Invoke() { return static_cast<uint32_t>(reinterpret_cast<m109::Callback*>(this)->Invoke()); }
};
}
