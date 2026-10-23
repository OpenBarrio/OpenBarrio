#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::audio::b04 {
struct Pair { uint32_t id; void* object; };
struct PairVector { Pair *begin,*end,*capacity; const char* label; Pair storage[21]; };
struct Node { Node* next; uint32_t reserved; uint32_t key[2]; uint32_t flags,kind; uint32_t pad[2]; alignas(16) float position[4]; };
struct Table { Node* buckets[53]; Node* sentinel; uint32_t count; };
struct Iterator { Node* node; Node** bucket; };
struct PtrVector { Node **begin,**end,**capacity; const char* label; };
struct Owner {
 void* primary; void* secondary; void* third; uint32_t reserved; void* fourth;
 uint8_t pad[0x94]; uint8_t flags[4]; uint32_t more[2];
 PairVector pairs; uint32_t kind; float radius; PtrVector selected;
};
TS3_X86_OFFSET(Owner,flags,0xa8); TS3_X86_OFFSET(Owner,pairs,0xb4); TS3_X86_OFFSET(Owner,selected,0x174);
struct TextVector { const char16_t* begin; const char16_t* end; const char16_t* capacity; };
struct Lookup { void* pad[4]; void* service; };
// TS3W.exe 0x00861FB0: initialize the derived ambience interfaces.
Owner* Construct(Owner*);
// TS3W.exe 0x00861FE0: return the ambience type identifier.
uint32_t Type(void*);
// TS3W.exe 0x00861FF0: adjust the fourth interface before deleting.
Owner* DeleteAdjusted(void*,uint32_t);
// TS3W.exe 0x00862000: stop active ambience, destroy retained entries and optionally free.
Owner* Delete(Owner*,uint32_t);
// TS3W.exe 0x00862060: insert a retained pair, including aliased source and growth.
uintptr_t Insert(PairVector*,Pair*,const Pair*);
// TS3W.exe 0x008622D0: append or combine an ambience delegate; upper EAX bits are retained.
uintptr_t Add(Owner*,uint32_t,void*);
// TS3W.exe 0x008624A0: select unflagged nodes strictly inside the squared radius.
void Select(Owner*,Table*,const float*);
// TS3W.exe 0x008625B0: grow a pool with signed capacity and a 340 element cap.
uintptr_t Grow(void*);
// TS3W.exe 0x00862610: resolve concatenated UTF16 subset names, preserving enumeration order.
uintptr_t Resolve(Lookup*,const TextVector*,const char16_t*);
// TS3W.exe 0x008627C0: copy an aligned vector and return the source pointer.
const void* CopyVector(void*,const void*);
// TS3W.exe 0x008627E0: unlink a hash node and return its successor iterator.
Iterator* Unlink(Table*,Iterator*,Node*,Node**);
// TS3W.exe 0x00862850: copy 500-byte text records to uninitialized storage.
void** CopyRecords(void**,const void*,const void*,void*);
// TS3W.exe 0x008628A0: find a two-word key in 53 buckets, returning the sentinel on miss.
Iterator* Find(Table*,Iterator*,const uint32_t*);
}

namespace ts3::audio::b04 {
inline Owner* __fastcall HookConstruct(Owner* s,void*) { return Construct(s); }
inline uint32_t __fastcall HookType(void* s,void*) { return Type(s); }
inline Owner* __fastcall HookDeleteAdjusted(void* s,void*,uint32_t f) { return DeleteAdjusted(s,f); }
inline Owner* __fastcall HookDelete(Owner* s,void*,uint32_t f) { return Delete(s,f); }
inline uintptr_t __fastcall HookInsert(PairVector* s,void*,Pair* p,const Pair* v) { return Insert(s,p,v); }
inline uintptr_t __fastcall HookAdd(Owner* s,void*,uint32_t id,void* p) { return Add(s,id,p); }
inline void __fastcall HookSelect(Owner* s,void*,Table* t,const float* p) { return Select(s,t,p); }
inline uintptr_t __fastcall HookGrow(void* s,void*) { return Grow(s); }
inline uintptr_t __fastcall HookResolve(Lookup* s,void*,const TextVector* v,const char16_t* p) { return Resolve(s,v,p); }
inline const void* __fastcall HookCopyVector(void* s,void*,const void* p) { return CopyVector(s,p); }
inline Iterator* __fastcall HookUnlink(Table* s,void*,Iterator* i,Node* n,Node** b) { return Unlink(s,i,n,b); }
inline Iterator* __fastcall HookFind(Table* s,void*,Iterator* i,const uint32_t* k) { return Find(s,i,k); }
}
