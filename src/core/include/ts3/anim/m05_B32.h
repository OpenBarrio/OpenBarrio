#pragma once
#include "ts3/layout.h"
namespace ts3::anim::b32 {
struct Ref { void* primary; void** deleting; uint32_t refs; };
struct Entity { void** vtable; void* opaque[2]; int32_t count; Entity** begin; Entity** end; Entity** capacity; void* descriptor; uint8_t flag; };
struct Owner { unsigned char opaque[0x28]; Ref* value; };
struct Link { Link* next; Link* previous; };
struct Linked { unsigned char opaque[0x40]; Link link; };
struct Text { const uint16_t* begin; const uint16_t* end; };
TS3_X86_OFFSET(Ref, refs, 8);
TS3_X86_OFFSET(Entity, count, 0xc);
TS3_X86_OFFSET(Entity, begin, 0x10);
TS3_X86_OFFSET(Entity, flag, 0x20);
TS3_X86_OFFSET(Owner, value, 0x28);
TS3_X86_OFFSET(Linked, link, 0x40);
// TS3W.exe 0x00839CA0: Return a child only within both signed bounds.
Entity* Child(Entity*, int32_t);
// TS3W.exe 0x00839CD0: Collect child identifiers, zeroing null entries.
void Identifiers(Entity*, uint64_t*);
// TS3W.exe 0x00839D20: Assign an intrusive reference and return the destination.
Ref** Assign(Ref**, Ref*);
// TS3W.exe 0x00839D60: Assign the owner's reference, restoring one before deletion.
void SetOwner(Owner*, Ref*);
// TS3W.exe 0x0083A1B0: Select the strict maximum scored entity and consume the scorer reference.
Entity* Best(Entity*, void*);
// TS3W.exe 0x0083A650: Replace the child range, or clear count and flag on invalid input.
void SetChildren(Entity*, Entity**, int32_t, uint8_t);
// TS3W.exe 0x0083A6D0: Advance the external iterator until its terminal token.
void Advance(void*);
// TS3W.exe 0x0083AB40: Insert an unlinked child at the head of the intrusive list.
void Attach(Linked*, Linked*);
struct Handle { void* opaque; void* object; };
// TS3W.exe 0x0083AC20: Follow parent identifiers to test ancestry; returns only AL.
bool Ancestor(const Handle*, const Handle*);
struct NodeLarge { uint32_t key; unsigned char opaque[0xf90]; NodeLarge* next; };
struct NodeSmall { uint32_t key; unsigned char opaque[0x60]; NodeSmall* next; };
struct NodePair { uint32_t key; unsigned char a[8]; uint32_t other; unsigned char b[0x18]; NodePair* next; };
template<class N> struct Table { void* opaque; N** buckets; uint32_t count; unsigned char pad[0x10]; void* descriptor; };
TS3_X86_OFFSET(NodeLarge, next, 0xf94);
TS3_X86_OFFSET(NodeSmall, next, 0x64);
TS3_X86_OFFSET(NodePair, next, 0x28);
TS3_X86_OFFSET(Table<NodeLarge>, descriptor, 0x1c);
// TS3W.exe 0x0083AC90: Rehash large nodes, reversing traversal order within buckets.
void RehashLarge(Table<NodeLarge>*, uint32_t);
// TS3W.exe 0x0083AD50: Rehash small nodes, freeing old buckets only when count exceeds one.
void RehashSmall(Table<NodeSmall>*, uint32_t);
// TS3W.exe 0x0083AE00: Rehash pair keys using XOR, preserving the trailing -1 sentinel.
void RehashPair(Table<NodePair>*, uint32_t);
// TS3W.exe 0x0083AEC0: Reverse UTF16 search, returning first for an empty range or needle.
uintptr_t Reverse(const uint16_t*, const uint16_t*, const uint16_t*, const uint16_t*);
// TS3W.exe 0x0083B510: Reverse substring index, clamping the starting position.
uint32_t Find(Text*, const uint16_t*, uint32_t, uint32_t);
// TS3W.exe 0x0083B9D0: Measure a zero-terminated needle and forward to reverse index search.
uint32_t FindZ(Text*, const uint16_t*, uint32_t);
}

namespace ts3::anim::b32 {
// TS3W.exe 0x00839CA0: ABI entry for Child.
Entity* __fastcall HookChild(Entity* s, void*, int32_t i);
// TS3W.exe 0x00839CD0: ABI entry for Identifiers.
void __fastcall HookIdentifiers(Entity* s, void*, uint64_t* out);
// TS3W.exe 0x00839D20: ABI entry for Assign.
Ref** __fastcall HookAssign(Ref** s, void*, Ref* p);
// TS3W.exe 0x00839D60: ABI entry for SetOwner.
void __fastcall HookSetOwner(Owner* s, void*, Ref* p);
// TS3W.exe 0x0083A1B0: ABI entry for Best.
Entity* __fastcall HookBest(Entity* s, void*, void* p);
// TS3W.exe 0x0083A650: ABI entry for SetChildren.
void __fastcall HookSetChildren(Entity* s, void*, Entity** p, int32_t n, uint8_t f);
// TS3W.exe 0x0083A6D0: ABI entry for Advance.
void __fastcall HookAdvance(void*, void*, void* p);
// TS3W.exe 0x0083AB40: ABI entry for Attach.
void __fastcall HookAttach(Linked* s, void*, Linked* p);
// TS3W.exe 0x0083AC20: ABI entry for Ancestor.
bool __cdecl HookAncestor(const Handle* a, const Handle* b);
// TS3W.exe 0x0083AC90: ABI entry for RehashLarge.
void __fastcall HookRehashLarge(Table<NodeLarge>* s, void*, uint32_t n);
// TS3W.exe 0x0083AD50: ABI entry for RehashSmall.
void __fastcall HookRehashSmall(Table<NodeSmall>* s, void*, uint32_t n);
// TS3W.exe 0x0083AE00: ABI entry for RehashPair.
void __fastcall HookRehashPair(Table<NodePair>* s, void*, uint32_t n);
// TS3W.exe 0x0083AEC0: ABI entry for Reverse.
uintptr_t __cdecl HookReverse(const uint16_t* a, const uint16_t* b, const uint16_t* c, const uint16_t* d);
// TS3W.exe 0x0083B510: ABI entry for Find.
uint32_t __fastcall HookFind(Text* s, void*, const uint16_t* p, uint32_t pos, uint32_t n);
// TS3W.exe 0x0083B9D0: ABI entry for FindZ.
uint32_t __fastcall HookFindZ(Text* s, void*, const uint16_t* p, uint32_t pos);
}
