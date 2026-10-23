#pragma once
#include "ts3/layout.h"
namespace ts3::resource::m03::d02 {
struct MapNode { const char16_t* key; void* value; MapNode* next; };
struct SetNode { const char16_t* key; SetNode* next; };
template<class N> struct Hash { uintptr_t unused; N** buckets; uint32_t count,size; uint32_t policy[3]; const char* allocName; };
template<class N> struct Result { N* node; N** cell; uint8_t inserted; uint8_t padding[sizeof(void*)-1]; };
struct String { char16_t *begin,*end,*capacity; const char* allocName; void* value; };
TS3_X86_OFFSET(Hash<MapNode>,allocName,0x1c);
TS3_X86_SIZE(String,20);
// TS3W.exe 0x00734AF0: acquire a record and query its interface; release on failure, preserving EAX above AL.
uintptr_t __fastcall Query(void*,void*,const void*,const void*,void**);
// TS3W.exe 0x00734B50: open and register a directory or packed database, preserving failure cleanup order.
void* __fastcall Open(void*,void*,const char16_t*,uint32_t);
// TS3W.exe 0x007356D0: insert UTF16 pointer-key/value into a hash, using pointer identity rather than text equality.
Result<MapNode>* __fastcall InsertMap(Hash<MapNode>*,void*,Result<MapNode>*,const MapNode*,uintptr_t);
// TS3W.exe 0x007357F0: insert unique UTF16 text into a hash using the original text lookup helper.
Result<SetNode>* __fastcall InsertSet(Hash<SetNode>*,void*,Result<SetNode>*,const SetNode*,uintptr_t);
// TS3W.exe 0x00735A20: uninitialized-copy a range of owning strings and payloads, returning the output cursor address.
String** __cdecl CopyStrings(String**,const String*,const String*,String*);
}
