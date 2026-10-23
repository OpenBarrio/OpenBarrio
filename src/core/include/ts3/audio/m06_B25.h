#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::audio::b25 {
struct Payload { void* vtable; int32_t refs; uint32_t tag; void* data; uint32_t size; };
struct RefValue { void* object; uint8_t flag; };
struct Entry { uint32_t key; void* object; uint8_t flag; };
struct Node { Node* right; Node* left; Node* parent; uint32_t color; uint32_t key; void* object; };
struct Tree { uint32_t unused; Node* first; Node* last; Node* root; uint8_t flag; uint8_t pad[3]; uint32_t count; Node* free; };
struct Group { uint32_t key[2]; Tree tree; uint8_t gap[0xb8-8-28]; Group* next; };
struct Table { uint32_t unused; Group** buckets; uint32_t count; };
struct Iterator { Group* node; Group** bucket; };
struct Queue { uint8_t prefix[12]; Group** buckets; uint32_t count; uint8_t gap[0x38-20]; uint8_t mutex[0x30]; uint32_t active; uint8_t gap2[8]; float update; };
TS3_X86_OFFSET(Payload,data,12); TS3_X86_OFFSET(Node,object,20);
TS3_X86_OFFSET(Tree,free,24); TS3_X86_OFFSET(Group,next,0xb8);
TS3_X86_OFFSET(Queue,mutex,0x38); TS3_X86_OFFSET(Queue,active,0x68); TS3_X86_OFFSET(Queue,update,0x74);
// TS3W.exe 0x008711C0: return the fixed sound interface name (argument ignored).
const void* SoundName(void* self, uint32_t ignored);
// TS3W.exe 0x008711D0: read the raw sound flag byte, without canonicalizing it.
uint8_t GetFlag(void* self);
// TS3W.exe 0x008711E0: store and return the low byte of the argument.
uint8_t SetFlag(void* self, uint32_t value);
// TS3W.exe 0x00871200: adjust the third sound interface to its deleting destructor.
void* DeleteInterface(void* self, uint32_t flags);
// TS3W.exe 0x00871210: construct a retained payload and copy bytes; allocation failure clears tag and size too.
Payload* Construct(Payload* self, uint32_t tag, const void* data, uint32_t size);
// TS3W.exe 0x00871280: destroy payload storage and optionally the object; zero-sized storage is not freed.
Payload* Destroy(Payload* self, uint32_t flags);
// TS3W.exe 0x008712E0: copy a key/ref/flag entry, retaining the object before publishing its flag.
Entry* CopyEntry(Entry* out, const uint32_t* key, const RefValue* value);
// TS3W.exe 0x00871310: under the recursive lock, search each group's unsigned-key tree.
uint8_t Contains(Queue* self, uint32_t key);
// TS3W.exe 0x008713D0: update flagged sounds under lock with a temporary retain per tree entry.
void Update(Queue* self, float value);
// TS3W.exe 0x008714D0: find a two-word key and return its bucket iterator or the end iterator.
Iterator* Find(Table* self, Iterator* out, const uint32_t* key);
// TS3W.exe 0x00871540: set the special scalar/UTF16 filename properties, otherwise delegate to the base.
uintptr_t SetProperty(void* self, uint32_t key, uintptr_t value, uint32_t extra);
// TS3W.exe 0x008715A0: recycle the tree, reset its intrusive sentinel, and return zero.
int32_t Clear(Tree* self);
// TS3W.exe 0x008715D0: release objects in right-recursive/left-iterative order and push nodes onto the pool.
void Recycle(Tree* self, Node* node);
// TS3W.exe 0x00871610: pop a pool node, retain its value, link it and increment count; empty pool still faults.
Node** Insert(Tree* self, Node** out, Node* parent, const Entry* value, uint8_t forceLeft);
#if defined(_M_IX86)
const void* __fastcall HookSoundName(void* s, void*, uint32_t a);
uint8_t __fastcall HookGetFlag(void* s, void*);
uint8_t __fastcall HookSetFlag(void* s, void*, uint32_t a);
void* __fastcall HookDeleteInterface(void* s, void*, uint32_t a);
Payload* __fastcall HookConstruct(Payload* s, void*, uint32_t a, const void* b, uint32_t c);
Payload* __fastcall HookDestroy(Payload* s, void*, uint32_t a);
uint8_t __fastcall HookContains(Queue* s, void*, uint32_t a);
void __fastcall HookUpdate(Queue* s, void*, float a);
Iterator* __fastcall HookFind(Table* s, void*, Iterator* a, const uint32_t* b);
uintptr_t __fastcall HookSetProperty(void* s, void*, uint32_t a, uintptr_t b, uint32_t c);
int32_t __fastcall HookClear(Tree* s, void*);
void __fastcall HookRecycle(Tree* s, void*, Node* a);
Node** __fastcall HookInsert(Tree* s, void*, Node** a, Node* b, const Entry* c, uint8_t d);
#endif
}
