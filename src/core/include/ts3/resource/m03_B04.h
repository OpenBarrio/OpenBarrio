#pragma once
#include "ts3/layout.h"
namespace ts3::resource::m03::b04 {
struct Node { uintptr_t payload[4]; Node* next; };
struct Map { void* allocator; Node** buckets; uint32_t bucketCount, count; uint32_t policy[4]; };
struct Vector { Map* begin; Map* end; Map* capacity; const char* name; };
TS3_X86_SIZE(Map,32);
// TS3W.exe 0x0059C850: hash UTF16 text then request a localized string.
uintptr_t Lookup(void* self,const char16_t* text);
// TS3W.exe 0x0059C870: update the locale singleton, if present; EAX is unspecified when absent.
void SetLocale(const char16_t* text);
// TS3W.exe 0x0059C890: export current locale through the scripting string adapter.
uint32_t Locale();
// TS3W.exe 0x0059C960: export the first derived locale through the scripting adapter.
uint32_t DerivedLocale();
// TS3W.exe 0x0059CA30: export the second derived locale through the scripting adapter.
uint32_t AlternateLocale();
// TS3W.exe 0x0059CE20: reserve then insert a hash iterator range; retain low-byte insert flag behavior.
uintptr_t InsertRange(Map*,Node*,Node**,Node*,Node**);
// TS3W.exe 0x0059CEC0: clear and copy a map, except on self-assignment; return destination.
Map* Assign(Map*,const Map*);
// TS3W.exe 0x0059CF20: construct and publish the NameGuidMap service.
void* Construct(void*);
// TS3W.exe 0x0059CF60: register NameGuidMap unless already registered; test only AL.
uintptr_t Ensure();
// TS3W.exe 0x0059CFE0: copy maps backwards, permitting overlapping ranges.
Map* Backward(const Map*,const Map*,Map*);
// TS3W.exe 0x0059D0C0: insert a map into a vector, preserving alias adjustment and growth returns.
uintptr_t Insert(Vector*,Map*,const Map*);
// TS3W.exe 0x0059F5E0: TableFactory deleting destructor; free with resource allocator iff flags bit zero is set.
void* DestroyFactory(void*,uint32_t);
}

#if defined(_M_IX86)
namespace ts3::resource::m03::b04 {
uintptr_t __fastcall HookLookup(void* p,void*,const char16_t* text);
uintptr_t __fastcall HookInsertRange(Map* p,void*,Node* a,Node** b,Node* c,Node** d);
Map* __fastcall HookAssign(Map* p,void*,const Map* s);
void* __fastcall HookConstruct(void* p,void*);
uintptr_t __fastcall HookInsert(Vector* p,void*,Map* pos,const Map* value);
void* __fastcall HookDestroyFactory(void* p,void*,uint32_t flags);
}
#endif
