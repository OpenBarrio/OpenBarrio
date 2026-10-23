#pragma once
#include "ts3/layout.h"
namespace ts3::resource::m03::c04 {
struct Node { Node* next; uint32_t reserved; uint32_t low,high; const char16_t* text; };
struct Hash { Node* buckets[7919]; Node* sentinel; uint32_t count; };
struct InsertResult { Node* node; Node** bucket; uint8_t inserted; };
struct Table { void* const* vtable; uint8_t prefix[0x24]; Hash hash; uint32_t last; void* data; uint32_t dataSize; };
TS3_X86_OFFSET(Node,low,8);
TS3_X86_OFFSET(Table,hash,0x28);
TS3_X86_OFFSET(Hash,count,0x7bc0);
// TS3W.exe 0x0059F610: query four supported resource interface IDs.
void* __fastcall Query(Table*,void*,uint32_t);
// TS3W.exe 0x0059F650: allocate and register the string-table factory, even on allocation failure.
uint8_t __cdecl RegisterFactory();
// TS3W.exe 0x0059F730: destroy table contents and the resource base.
uintptr_t __fastcall Destroy(Table*,void*);
// TS3W.exe 0x0059F750: insert a node unless its full 64-bit key already exists; third argument is unused.
InsertResult* __fastcall Insert(Hash*,void*,InsertResult*,Node*,uint32_t);
// TS3W.exe 0x0059F830: destroy and optionally free a table, returning the original pointer.
Table* __fastcall Delete(Table*,void*,uint32_t);
// TS3W.exe 0x0059F860: serialize an STBL header and bucket-ordered UTF16 strings.
uint8_t __fastcall Write(Table*,void*,void*);
// TS3W.exe 0x0059F9F0: allocate and initialize a table resource (adapted from verified 007D5FB0).
Table* __fastcall Create(void*,void*);
// TS3W.exe 0x0059FC90: read flags/count and invoke the table-entry parser; later read failures are ignored.
uint8_t __fastcall ReadBody(Table*,void*,void*,uint32_t);
// TS3W.exe 0x0059FD00: skip four unchecked magic bytes, read version, and select the body encoding.
uint8_t __fastcall Read(Table*,void*,void*);
// TS3W.exe 0x0059FF50: query the resource interface and read the input stream into it.
uint8_t __fastcall Load(void*,void*,void*,void*);
// TS3W.exe 0x006922F0: apply locale to an existing CSS service or save its borrowed pointer.
void __cdecl CssLocale(const char16_t*);
// TS3W.exe 0x007D0430: query the installed locale registry value, returning the caller's buffer on success.
char* __cdecl RegistryLocale(char*,uint32_t);
}
