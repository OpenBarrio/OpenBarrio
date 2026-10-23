#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::audio::b10 {
struct Retained { void** vtable; int32_t references; };
struct Node { Node* right; Node* left; Node* parent; uint32_t color; uint32_t key; Retained* value; };
// The sentinel occupies +4..+0f; key/value are not part of the header.
struct Map { void* allocator; Node* maximum; Node* minimum; Node* root; uint8_t color; uint8_t colorPadding[3]; uint32_t count; };
struct InsertResult { Node* node; uint8_t inserted; };
struct VectorView { void* allocator; void* begin; uintptr_t capacity; uint32_t size; };
struct Manager {
    void** vtable; void** secondary;
    uint8_t reserved08[0xF8-8];
    void* properties;
    uint8_t callback[0x27BD0-0xFC];
    VectorView symbols;
    uint8_t gapBE0[0x10];
    void* listener;
    void* service;
    VectorView objects;
    uint8_t gapC08[0x10];
    uint8_t mutex1[0x20];
    uint8_t mutex2[0x20];
    uint8_t gapC58[4];
    void* scratch;
    uint8_t gapC60[0x50];
    uint32_t triples[7][3];
    void* subscription;
    uint8_t enabled;
    uint8_t padD09[3];
    VectorView pending;
    uint8_t gapD1C[0x10];
    Map arrays;
};
TS3_X86_OFFSET(Manager, properties, 0xF8);
TS3_X86_OFFSET(Manager, symbols, 0x27BD0);
TS3_X86_OFFSET(Manager, scratch, 0x27C5C);
TS3_X86_OFFSET(Manager, triples, 0x27CB0);
TS3_X86_OFFSET(Manager, arrays, 0x27D2C);
// TS3W.exe 0x00867040: Destroy a tree in right-subtree/left-spine order, resetting last references before deletion.
void DestroyTree(Map*, Node*);
// TS3W.exe 0x008670A0: Insert a key only when absent and report whether it was inserted.
InsertResult* InsertUnique(Map*, InsertResult*, const uint32_t*, uint32_t);
// TS3W.exe 0x00867D50: Register manager callbacks and load the seven validated property triples.
uint8_t Initialize(Manager*);
// TS3W.exe 0x00868090: Erase a matching map key, returning one only when found.
uint32_t Erase(Map*, const uint32_t*);
// TS3W.exe 0x008680D0: Insert using a hint when the strict neighboring-key checks permit it.
Node** InsertHint(Map*, Node**, Node*, const uint32_t*, uint32_t);
// TS3W.exe 0x00868190: Tear down manager-owned resources in the original callback-visible order.
uintptr_t Destruct(Manager*);
// TS3W.exe 0x00868300: Return the fixed unsupported-operation result, ignoring all three arguments.
uint32_t Unsupported(Manager*, uintptr_t, uintptr_t, uintptr_t);
// TS3W.exe 0x00868310: Adjust the secondary interface and run the deleting destructor.
Manager* DeleteSecondary(void*, uint32_t);
// TS3W.exe 0x00868320: Clear the sound-array map and invoke base shutdown.
uintptr_t Shutdown(Manager*);
// TS3W.exe 0x00868650: Destroy and optionally free the manager; return its original address.
Manager* Delete(Manager*, uint32_t);
// TS3W.exe 0x00868670: Return the mapped value, inserting a zero value when the key is missing.
Retained** GetOrInsert(Map*, const uint32_t*);
// TS3W.exe 0x008686F0: Decode and enqueue a sound-array command, retaining across callbacks.
uint8_t QueueMessage(Manager*, void*);
// TS3W.exe 0x00868820: Stop, erase and unregister the selected sound array.
uint8_t RemoveMessage(Manager*, void*);
// TS3W.exe 0x008688E0: Forward an optional float property to the retained sound array.
uint8_t FloatMessage(Manager*, void*);

#if defined(_M_IX86)
// ABI-compatible fastcall adapters: EDX is the unused thiscall bridge word.
inline InsertResult* __fastcall HookInsertUnique(Map* s, void*, InsertResult* o, const uint32_t* k, uint32_t u) { return InsertUnique(s,o,k,u); }
inline uint8_t __fastcall HookInitialize(Manager* s, void*) { return Initialize(s); }
inline uint32_t __fastcall HookErase(Map* s, void*, const uint32_t* k) { return Erase(s,k); }
inline Node** __fastcall HookInsertHint(Map* s, void*, Node** o, Node* h, const uint32_t* k, uint32_t u) { return InsertHint(s,o,h,k,u); }
inline uintptr_t __fastcall HookDestruct(Manager* s, void*) { return Destruct(s); }
inline uint32_t __fastcall HookUnsupported(Manager* s, void*, uintptr_t a, uintptr_t b, uintptr_t c) { return Unsupported(s,a,b,c); }
inline uintptr_t __fastcall HookShutdown(Manager* s, void*) { return Shutdown(s); }
inline Manager* __fastcall HookDelete(Manager* s, void*, uint32_t f) { return Delete(s,f); }
inline Retained** __fastcall HookGetOrInsert(Map* s, void*, const uint32_t* k) { return GetOrInsert(s,k); }
inline uint8_t __fastcall HookQueueMessage(Manager* s, void*, void* m) { return QueueMessage(s,m); }
inline uint8_t __fastcall HookRemoveMessage(Manager* s, void*, void* m) { return RemoveMessage(s,m); }
inline uint8_t __fastcall HookFloatMessage(Manager* s, void*, void* m) { return FloatMessage(s,m); }
#endif
}
