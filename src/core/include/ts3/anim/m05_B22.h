#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::b22 {
struct String { char* begin; char* end; char* capacity; void* descriptor; };
struct PackedClip { uint32_t signature; PackedClip* next; uint32_t unknown08; void* data0c; void* groups; void* data14; void* data18; uint32_t unknown1c,unknown20; void* data24; };
// TS3W.exe 0x00578A50: Destroy the IK arrays in reverse order and optionally free this; returns this.
ClipIkInfo* Destroy(ClipIkInfo*,uint32_t);
// TS3W.exe 0x00578AB0: Initialize IK metadata without clearing opaque bytes.
ClipIkInfo* Construct(ClipIkInfo*);
// TS3W.exe 0x00579690: Relocate a linked serialized clip payload in place.
bool Relocate(PackedClip*);
// TS3W.exe 0x00579790: Look up the namespace byte in the sentinel-terminated table.
uint8_t NamespaceByte(uint32_t);
// TS3W.exe 0x005797D0: Pack two namespace bytes into bits 48 through 63.
uint64_t NamespaceKey(uint32_t,uint32_t);
// TS3W.exe 0x00579810: XOR two keys and apply namespace mask only for a nonzero second key.
uint64_t Combine(uint64_t,uint64_t);
// TS3W.exe 0x00579840: Find the last byte in a range, returning end if absent.
const char* FindByte(const char*,const char*,uint8_t);
// TS3W.exe 0x005798F0: Hash a name, clear namespace bits, and optionally combine with a key.
uint64_t HashName(const char*,uint64_t,uint64_t*);
// TS3W.exe 0x00579950: Reverse substring search; empty pattern returns begin (EA quirk).
const char* FindLast(const char*,const char*,const char*,const char*);
// TS3W.exe 0x00579A10: Reverse substring search with bounded starting position.
uint32_t RFind(String*,const char*,uint32_t,uint32_t);
// TS3W.exe 0x00579A80: Construct a prefix string or copy the whole source.
String* Prefix(String*,String*,uint32_t);
// TS3W.exe 0x00579AD0: Construct a suffix string or copy the whole source.
String* Suffix(String*,String*,uint32_t);
// TS3W.exe 0x00579B40: Construct the concatenation of two strings.
String* Concat(String*,const String*,const String*);
// TS3W.exe 0x00579BB0: Parse a short namespace pair; names longer than five bytes produce zero.
uint16_t Parse(String*);
// TS3W.exe 0x0057A4B0: Return this for three supported interface IDs, otherwise null.
void* Query(void*,uint32_t);
}

namespace ts3::anim::b22 {
inline ClipIkInfo* __fastcall HookDestroy(ClipIkInfo* s, void*, uint32_t flags) { return Destroy(s,flags); }
inline ClipIkInfo* __fastcall HookConstruct(ClipIkInfo* s, void*) { return Construct(s); }
inline uint32_t __fastcall HookRFind(String* s, void*, const char* p,uint32_t pos,uint32_t n) { return RFind(s,p,pos,n); }
inline String* __fastcall HookPrefix(String* s,void*,String* out,uint32_t n) { return Prefix(s,out,n); }
inline String* __fastcall HookSuffix(String* s,void*,String* out,uint32_t n) { return Suffix(s,out,n); }
inline void* __fastcall HookQuery(void* s,void*,uint32_t id) { return Query(s,id); }
}
