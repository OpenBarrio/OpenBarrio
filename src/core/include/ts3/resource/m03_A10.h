#pragma once
#include "ts3/resource/m03_catalog_driver_contracts.h"
#include "ts3/resourcesystem/clu_4a3640_ResourceSystem.h"
#include <array>
namespace ts3::resource::m03::a10 {
using Driver=catalog_driver::DriverView;
using Path=resourcesystem::PathString;
struct Mapping { uint32_t scalar[2]; Path path; std::array<uint32_t,4> identifier; };
struct Directory { const void* table; Mapping* begin; Mapping* end; Mapping* capacity; };
struct Published;
struct DriverPool { uint8_t reserved[0x10]; Published* head; uint8_t tail[0x10]; };
struct StreamDriver { Driver base; void* argument; DriverPool pool; Mutex mutex; };
TS3_X86_SIZE(DriverPool,0x24);
struct Published { Published* next; uintptr_t unused; void* handle; void* record; };
TS3_X86_SIZE(Mapping,0x28);
TS3_X86_OFFSET(StreamDriver,pool,0x14);
TS3_X86_OFFSET(StreamDriver,mutex,0x38);
// TS3W.exe 0x0072E200: destroys a nullable request wrapper, then closes its storage; EAX incidental.
void ReleaseWrapper(void*);
// TS3W.exe 0x0072E220: constructs the named resman driver and returns self.
Driver* ConstructName(Driver*,const char*);
// TS3W.exe 0x0072E240: constructs a stream driver, pool and mutex, returning self.
StreamDriver* ConstructStream(StreamDriver*,const char*,void*);
// TS3W.exe 0x0072E290: configures the pool, normalizing AL only and preserving high EAX.
uint32_t ConfigurePool(StreamDriver*);
// TS3W.exe 0x0072E2C0: schedules handle disposal, releases the record, and publishes under lock.
void Publish(StreamDriver*,Published*);
// TS3W.exe 0x0072E340: destroys the resman base, freeing self iff flags bit zero is set.
Driver* DeleteName(Driver*,uint32_t);
// TS3W.exe 0x0072E370: parses a path and opens a referenced database, returning the owned result.
void* Open(Driver*,const char*,uint32_t,uint32_t);
// TS3W.exe 0x0072E470: destroys mutex, pool, base, optionally freeing self, and returns self.
StreamDriver* DeleteStream(StreamDriver*,uint32_t);
// TS3W.exe 0x0072E790: copies a bounded category identifier; only AL is defined.
uint8_t Category(uint32_t,uint32_t,void*);
// TS3W.exe 0x0072E7F0: scans both category tables in order; only AL is defined.
uint8_t IsCategory(const void*);
// TS3W.exe 0x0072E8E0: frees owned paths in a mapping range; uses signed even-byte capacity.
void __stdcall DestroyRange(Mapping*,Mapping*);
// TS3W.exe 0x0072E920: constructs mapping copies and advances a cursor cell; returns that cell.
Mapping** CopyRange(Mapping**,const Mapping*,const Mapping*,Mapping*,uintptr_t);
// TS3W.exe 0x0072EA00: backward-assigns mappings and returns the destination begin.
Mapping* AssignRange(const Mapping*,const Mapping*,Mapping*);
// TS3W.exe 0x0072EC70: clears mappings and backing storage before optional core-family delete.
Directory* DeleteDirectory(Directory*,uint32_t);
#if defined(_M_IX86)
inline Driver* __fastcall HookConstructName(Driver* p,void*,const char* name) { return ts3::resource::m03::a10::ConstructName(p,name); }
inline StreamDriver* __fastcall HookConstructStream(StreamDriver* p,void*,const char* n,void* a) { return ConstructStream(p,n,a); }
inline uint32_t __fastcall HookConfigurePool(StreamDriver* p,void*) { return ConfigurePool(p); }
inline void __fastcall HookPublish(StreamDriver* p,void*,Published* r) { Publish(p,r); }
inline Driver* __fastcall HookDeleteName(Driver* p,void*,uint32_t f) { return DeleteName(p,f); }
inline void* __fastcall HookOpen(Driver* p,void*,const char* n,uint32_t f,uint32_t u) { return ts3::resource::m03::a10::Open(p,n,f,u); }
inline StreamDriver* __fastcall HookDeleteStream(StreamDriver* p,void*,uint32_t f) { return DeleteStream(p,f); }
inline uint8_t __stdcall HookCategory(uint32_t t,uint32_t i,void* out) { return Category(t,i,out); }
inline uint8_t __stdcall HookIsCategory(const void* p) { return IsCategory(p); }
inline Directory* __fastcall HookDeleteDirectory(Directory* p,void*,uint32_t f) { return DeleteDirectory(p,f); }
#endif
}
