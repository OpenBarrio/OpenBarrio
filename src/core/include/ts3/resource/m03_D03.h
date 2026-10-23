#pragma once
#include <cstdint>
#include <cstddef>
namespace ts3::resource::d03 {
struct Payload { uint32_t key[2]; void* data; uint32_t flags; void* resource; uint32_t tag; };
struct Node { Node* next; Node* prev; Payload value; };
struct Locale {
    void* vtable; uint32_t base[3]; void* allocator;
    uint32_t reserved14, state[4]; char16_t locale[6];
    void* mapVtable; uint32_t type, one, reserved40, reserved44;
    Node* next; Node* prev; void* tag; uint32_t reserved54;
};
struct HashNode { HashNode* next; uint32_t unused; uint32_t key[2]; };
struct Table { HashNode* buckets[7919]; HashNode* sentinel; };
struct Iterator { HashNode* node; HashNode** bucket; };
#if defined(_M_IX86)
static_assert(offsetof(Locale, next)==0x48 && sizeof(Locale)==0x58);
static_assert(offsetof(Node,value.resource)==0x18 && sizeof(Node)==0x20);
static_assert(offsetof(Table,sentinel)==0x7bbc);
#endif
// TS3W.exe 0x005986B0: store the low byte of the locale flag.
void SetFlag(uint32_t flag);
// TS3W.exe 0x0059ABA0: find a locale name, returning zero for null/unknown.
uint64_t FindLocale(Locale*, const char16_t*);
// TS3W.exe 0x0059AC00: resolve five aliases before searching locale names.
uint64_t FindAlias(Locale*, const char16_t*);
// TS3W.exe 0x0059AC70: return the locale service type-name pointer.
const char16_t* TypeName(Locale*);
// TS3W.exe 0x0059AC80: select the CSS locale font resource.
uintptr_t SelectFont(Locale*);
// TS3W.exe 0x0059AF00: release resources, then free list nodes, then reset links.
void Clear(Locale*);
// TS3W.exe 0x0059AF50: find an unsigned 64-bit key in the 7919-bucket table.
Iterator* Find(Table*, Iterator*, const uint32_t*);
// TS3W.exe 0x0059AFC0: construct and publish the locale service.
Locale* Construct(Locale*, void* allocator);
// TS3W.exe 0x0059B030: destroy and optionally free, returning this.
Locale* Delete(Locale*, uint32_t);
// TS3W.exe 0x0059B0C0: register the locale service if absent (low-byte presence).
uintptr_t Register(void* allocator);
// TS3W.exe 0x0059B120: copy a payload and insert before a list position; allocation failure still faults.
Node** Insert(void* list, Node** out, Node* before, const Payload*);
// TS3W.exe 0x0059C400: clear and unregister the locale update callback.
uintptr_t Stop(Locale*);
// TS3W.exe 0x0059C800: parse a resource key then request its localized string.
uintptr_t Lookup(Locale*, const char16_t*);
#if defined(_M_IX86)
inline uint64_t __fastcall HookFindLocale(Locale* s,void*,const char16_t* n) { return FindLocale(s,n); }
inline uint64_t __fastcall HookFindAlias(Locale* s,void*,const char16_t* n) { return FindAlias(s,n); }
inline const char16_t* __fastcall HookTypeName(Locale* s,void*) { return TypeName(s); }
inline uintptr_t __fastcall HookSelectFont(Locale* s,void*) { return SelectFont(s); }
inline void __fastcall HookClear(Locale* s,void*) { return Clear(s); }
inline Iterator* __fastcall HookFind(Table* s,void*,Iterator* o,const uint32_t* k) { return Find(s,o,k); }
inline Locale* __fastcall HookConstruct(Locale* s,void*,void* a) { return Construct(s,a); }
inline Locale* __fastcall HookDelete(Locale* s,void*,uint32_t f) { return Delete(s,f); }
inline Node** __fastcall HookInsert(void* s,void*,Node** o,Node* b,const Payload* p) { return Insert(s,o,b,p); }
inline uintptr_t __fastcall HookStop(Locale* s,void*) { return Stop(s); }
inline uintptr_t __fastcall HookLookup(Locale* s,void*,const char16_t* t) { return Lookup(s,t); }
#endif
}
