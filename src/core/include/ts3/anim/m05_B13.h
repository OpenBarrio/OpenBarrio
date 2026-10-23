#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::b13_impl {
struct Range { unsigned char* begin; unsigned char* end; unsigned char* capacity; void* descriptor; };
struct Records { Controller base; void* secondary; Range first, middle, last; };
struct Stream { void* vtable; uint32_t unknown4; void* stream; uint32_t valueC, endian; uint8_t good; };
struct Node { uint32_t key; unsigned char opaque4[12]; unsigned char payload[64]; Node* next; };
struct Table { void* unknown; Node** buckets; uint32_t bucketCount, count; unsigned char policy[12]; void* descriptor; };
struct InsertResult { Node* node; Node** bucket; uint8_t inserted; };
struct Object { Controller base; Table table; };
TS3_X86_OFFSET(Records,first,0x2c);
TS3_X86_OFFSET(Records,middle,0x3c);
TS3_X86_OFFSET(Records,last,0x4c);
TS3_X86_OFFSET(Stream,good,0x14);
TS3_X86_OFFSET(Node,next,0x50);
TS3_X86_OFFSET(Table,descriptor,0x1c);
// TS3W.exe 0x0056A750: SaveVersion; preserves incidental EAX / partial AL returns.
uintptr_t SaveVersion(void* self, Stream* s);
// TS3W.exe 0x0056A780: SavePair; preserves incidental EAX / partial AL returns.
uintptr_t SavePair(void* self, Stream* s);
// TS3W.exe 0x0056A7D0: LoadVersion; preserves incidental EAX / partial AL returns.
uintptr_t LoadVersion(void* self, Stream* s);
// TS3W.exe 0x0056A850: CountFirst; preserves incidental EAX / partial AL returns.
uintptr_t CountFirst(Records* self);
// TS3W.exe 0x0056A860: SetFirst; preserves incidental EAX / partial AL returns.
uintptr_t SetFirst(Records* self, const void* v);
// TS3W.exe 0x0056A890: FindFirst; preserves incidental EAX / partial AL returns.
uintptr_t FindFirst(Records* self, uint32_t key, const void* v);
// TS3W.exe 0x0056A8D0: CountLast; preserves incidental EAX / partial AL returns.
uintptr_t CountLast(Records* self);
// TS3W.exe 0x0056A8E0: SetTime; preserves incidental EAX / partial AL returns.
uintptr_t SetTime(Records* self, float v);
// TS3W.exe 0x0056A900: SetWeight; preserves incidental EAX / partial AL returns.
uintptr_t SetWeight(Records* self, float v);
// TS3W.exe 0x0056A920: FindTime; preserves incidental EAX / partial AL returns.
uintptr_t FindTime(Records* self, uint32_t key, float v);
// TS3W.exe 0x0056A960: SetLast; preserves incidental EAX / partial AL returns.
uintptr_t SetLast(Records* self, const void* v);
// TS3W.exe 0x0056A990: FindLast; preserves incidental EAX / partial AL returns.
uintptr_t FindLast(Records* self, uint32_t key, const void* v);
// TS3W.exe 0x0056B440: Clear; preserves incidental EAX / partial AL returns.
uintptr_t Clear(Table* self);
// TS3W.exe 0x0056B860: Insert; preserves incidental EAX / partial AL returns.
uintptr_t Insert(Table* self, InsertResult* out, const uint32_t* key, uint32_t ignored);
// TS3W.exe 0x0056BAF0: Load; preserves incidental EAX / partial AL returns.
uintptr_t Load(Object* self, Stream* s);
}
namespace ts3::anim { struct b13 {
uint32_t SaveVersion(void* s) { return static_cast<uint32_t>(b13_impl::SaveVersion(reinterpret_cast<void*>(this), static_cast<b13_impl::Stream*>(s))); }
uint32_t SavePair(void* s) { return static_cast<uint32_t>(b13_impl::SavePair(reinterpret_cast<void*>(this), static_cast<b13_impl::Stream*>(s))); }
uint32_t LoadVersion(void* s) { return static_cast<uint32_t>(b13_impl::LoadVersion(reinterpret_cast<void*>(this), static_cast<b13_impl::Stream*>(s))); }
uint32_t CountFirst() { return static_cast<uint32_t>(b13_impl::CountFirst(reinterpret_cast<b13_impl::Records*>(this))); }
uint32_t SetFirst(void* v) { return static_cast<uint32_t>(b13_impl::SetFirst(reinterpret_cast<b13_impl::Records*>(this), static_cast<const void*>(v))); }
uint32_t FindFirst(uint32_t key, void* v) { return static_cast<uint32_t>(b13_impl::FindFirst(reinterpret_cast<b13_impl::Records*>(this), key, static_cast<const void*>(v))); }
uint32_t CountLast() { return static_cast<uint32_t>(b13_impl::CountLast(reinterpret_cast<b13_impl::Records*>(this))); }
uint32_t SetTime(float v) { return static_cast<uint32_t>(b13_impl::SetTime(reinterpret_cast<b13_impl::Records*>(this), v)); }
uint32_t SetWeight(float v) { return static_cast<uint32_t>(b13_impl::SetWeight(reinterpret_cast<b13_impl::Records*>(this), v)); }
uint32_t FindTime(uint32_t key, float v) { return static_cast<uint32_t>(b13_impl::FindTime(reinterpret_cast<b13_impl::Records*>(this), key, v)); }
uint32_t SetLast(void* v) { return static_cast<uint32_t>(b13_impl::SetLast(reinterpret_cast<b13_impl::Records*>(this), static_cast<const void*>(v))); }
uint32_t FindLast(uint32_t key, void* v) { return static_cast<uint32_t>(b13_impl::FindLast(reinterpret_cast<b13_impl::Records*>(this), key, static_cast<const void*>(v))); }
uint32_t Clear() { return static_cast<uint32_t>(b13_impl::Clear(reinterpret_cast<b13_impl::Table*>(this))); }
uint32_t Insert(void* out, void* key, uint32_t ignored) { return static_cast<uint32_t>(b13_impl::Insert(reinterpret_cast<b13_impl::Table*>(this), static_cast<b13_impl::InsertResult*>(out), static_cast<const uint32_t*>(key), ignored)); }
uint32_t Load(void* s) { return static_cast<uint32_t>(b13_impl::Load(reinterpret_cast<b13_impl::Object*>(this), static_cast<b13_impl::Stream*>(s))); }
}; }
