#pragma once
#include "ts3/eastl_game.h"

namespace ts3::resource::m03::b01 {
// Same bounded four-pointer string payload used by the original name helpers.
struct String { void *begin,*end,*cap,*allocName; };
struct Pair { void* object; uintptr_t flags; };
struct Vector { Pair *begin,*end,*capacity; GameAllocator allocator{}; };
struct Node { uint32_t key; Vector values; Node* next; };
struct Hash { uint32_t unused; Node** buckets; uint32_t count,size; uint32_t policy[3]; GameAllocator allocator{}; };
struct Result { Node* node; Node** cell; uint8_t inserted; uint8_t padding[sizeof(void*)-1]; };
struct Record { void* vtable; uintptr_t opaque04,opaque08; void* stream; uintptr_t opaque10[4]; uint8_t open,owned,pending; };
TS3_X86_OFFSET(Node,next,0x1c);
TS3_X86_SIZE(Node,32);
TS3_X86_OFFSET(Hash,allocator,0x1c);
TS3_X86_OFFSET(Record,pending,0x22);
// TS3W.exe 0x004B1DD0: find or insert an empty type name and return its string.
String* __fastcall TypeName(Hash*,void*,const uint32_t*);
// TS3W.exe 0x004B1E80: find or insert an empty resource key name and return its string.
String* __fastcall KeyName(Hash*,void*,const uint32_t*);
// TS3W.exe 0x004B1F50: insert adjusted-interface owning pair; in-place EAX is last cleanup result.
uintptr_t __fastcall InsertAdjusted(Vector*,void*,Pair*,const Pair*);
// TS3W.exe 0x004B20E0: insert primary-interface owning pair, retaining before releasing.
uintptr_t __fastcall InsertPrimary(Vector*,void*,Pair*,const Pair*);
// TS3W.exe 0x004B2260: destroy bucket chains and clear each bucket, returning count.
uint32_t __fastcall Clear(Hash*,void*,Node**,uint32_t);
// TS3W.exe 0x004B2320: insert unique type/factory vector; preserve output padding and policy AL.
Result* __fastcall InsertHash(Hash*,void*,Result*,const Node*,uintptr_t);
// TS3W.exe 0x004B2400: lock and assign a resource key name, preserving unlock EAX high bytes.
uint32_t __fastcall SetKeyName(void*,void*,const uint32_t*,const void*);
// TS3W.exe 0x004B24B0: insert type name or compare existing name, returning raw comparison AL.
uint32_t __fastcall SetTypeName(void*,void*,uint32_t,const void*);
// TS3W.exe 0x004B25B0: append adjusted pair or insert and return relocated position.
Pair* __fastcall AppendAdjusted(Vector*,void*,Pair*,const Pair*);
// TS3W.exe 0x004B2620: insert inline interlocked-ref pair; growth returns last deallocator EAX.
uintptr_t __fastcall InsertInline(Vector*,void*,Pair*,const Pair*);
// TS3W.exe 0x004B3040: append inline-ref pair or insert and return relocated position.
Pair* __fastcall AppendInline(Vector*,void*,Pair*,const Pair*);
// TS3W.exe 0x004B35C0: clear pending before closing owned stream; retains last virtual EAX above AL; only AL defined on no-call paths.
uint32_t __fastcall Finish(Record*,void*);
}
