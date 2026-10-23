#pragma once
#include "ts3/resource/compressed_index.h"
#include "ts3/layout.h"
namespace ts3::resource::m03::d04 {
using Key = ts3::resource::ResourceKey;
struct KeyList { Key *begin, *end, *capacity; const char* name; };
struct Archive { void *root, *owner, *stream; uint32_t endian, mode; uint8_t good, mapped; };
TS3_X86_OFFSET(Archive,stream,8);
TS3_X86_OFFSET(Archive,good,0x14);
// External boundary injection only; absent leaves bridge to originals on x86.
struct Leaves {
 bool (*enabled)()=nullptr;
 void* (*manager)()=nullptr;
 void* (*scripting)()=nullptr;
 void* (*cache)()=nullptr;
 uint64_t (*hash)(const char16_t*)=nullptr;
 void (*free)(void*)=nullptr;
 void (*construct)(Archive*,void*,unsigned)=nullptr;
 uint8_t (*read64)(void*,uint64_t*,unsigned)=nullptr;
 uint8_t (*read32)(void*,uint32_t*,unsigned)=nullptr;
 void (*destroy)(Archive*)=nullptr;
 uintptr_t (*allowed)(void*,uintptr_t,void*)=nullptr;
 uintptr_t (*read)(void*,uintptr_t)=nullptr;
 uintptr_t (*checksum)(void*,uintptr_t,uintptr_t,unsigned)=nullptr;
 uintptr_t (*write)(void*,uintptr_t,uintptr_t)=nullptr;
 uintptr_t (*shutdown)(void*)=nullptr;
 const void* bindings=nullptr;
 const void* classId=nullptr;
};
extern Leaves leaves;
// TS3W.exe 0x0073C650: Return CacheManager registration class identifier (donor immediate adapted).
uintptr_t __fastcall ClassId(void*,void*);
// TS3W.exe 0x0073C6C0: Clear a named cache by reverse key removal; compare returned database identity.
uint8_t ClearCache(uintptr_t name);
// TS3W.exe 0x0073C7C0: Open cached stream, deserialize three outputs and return pre-cleanup archive status.
uint8_t ReadStream(uintptr_t name,uint64_t* first,uint64_t* second,uint32_t* third);
// TS3W.exe 0x0073CE80: Dispatch caching query when singleton exists; only AL defined on null path.
uint8_t IsCachingAllowed(uintptr_t name,void* out);
// TS3W.exe 0x0073CEA0: Dispatch cache read or return null.
uintptr_t ReadCachedData(uintptr_t name);
// TS3W.exe 0x0073CEC0: Dispatch checksum read; null receiver leaves EAX unspecified.
uintptr_t ReadChecksum(uintptr_t name,uintptr_t out,unsigned size);
// TS3W.exe 0x0073CEE0: Dispatch cache write; null receiver leaves EAX unspecified.
uintptr_t WriteCachedData(uintptr_t name,uintptr_t data);
// TS3W.exe 0x0073CF00: Dispatch shutdown; null receiver leaves EAX unspecified.
uintptr_t Shutdown();
// TS3W.exe 0x0073CF10: Store native binding table and return the caller's output pointer.
const void** __fastcall BindingTable(void*,void*,const void** out);
// TS3W.exe 0x0073FF10: Forward four words to Scripting slot95's factory slot13.
uintptr_t FactoryCreate(uintptr_t a,uintptr_t b,uintptr_t c,uintptr_t d);
// TS3W.exe 0x0073FF40: Forward prototype get to factory slot14.
uintptr_t GetPrototype(uintptr_t a);
// TS3W.exe 0x0073FF60: Forward data-class get to factory slot15.
uintptr_t GetDataClass(uintptr_t a);
// TS3W.exe 0x0073FF80: Forward prototype release to factory slot16, retaining cleanup return.
uintptr_t ReleasePrototype(uintptr_t a);
// TS3W.exe 0x0073FFA0: Forward data-class release to factory slot17, retaining cleanup return.
uintptr_t ReleaseDataClass(uintptr_t a);
}
