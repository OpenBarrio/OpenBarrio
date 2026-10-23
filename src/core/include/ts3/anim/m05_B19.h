#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::anim::b19 {
struct Ref { void** vtable; uint32_t refs; unsigned char pad[8]; uint8_t dirty; };
struct Entry { Ref* value; uint32_t stamp; };
struct Cache { unsigned char opaque[16]; Entry entries[2]; };
struct DirtyState { unsigned char opaque[0x24]; Ref* auxiliary; unsigned char gap[12]; uint32_t flags; };
struct CoherentOwner { unsigned char opaque[0x34]; uintptr_t tagged; };
struct alignas(16) Coherent { unsigned char bytes[0x80]; };
struct alignas(16) Record { unsigned char bytes[64]; };
struct Range { Record* begin; Record* end; Record* capacity; };
struct alignas(16) Group { Range range; alignas(16) Record storage[11]; unsigned char vectors[32]; float weight, contribution; uint32_t flags; };
struct alignas(16) Groups { Group* begin; Group* end; Group* capacity; alignas(16) Group storage[16]; float total; };
TS3_X86_OFFSET(DirtyState,flags,0x34);
TS3_X86_OFFSET(Group,vectors,0x2d0);
TS3_X86_SIZE(Group,0x300);
TS3_X86_OFFSET(Groups,total,0x3010);
// TS3W.exe 0x00573720: Consume the state and auxiliary dirty bits.
uint8_t ConsumeDirty(DirtyState*);
// TS3W.exe 0x00573740: Initialize only the observed coherent-state fields.
Coherent* InitCoherent(Coherent*);
// TS3W.exe 0x00573D40: Obtain or lazily allocate coherent state, retaining low tag bits.
Coherent* GetCoherent(CoherentOwner*,bool);
// TS3W.exe 0x00573DA0: Expire old entries and look up a frame stamp.
Ref* Find(Cache*,uint32_t);
// TS3W.exe 0x00573E20: Replace the last entry tied for oldest stamp.
void Put(Cache*,Ref*,uint32_t);
// TS3W.exe 0x00573E80: Clear both cached references in order.
void Clear(Cache*);
// TS3W.exe 0x00574620: Construct record range, leaving twelve padding bytes untouched and quieting scalar sNaNs.
Record** ConstructRecords(Record**,const Record*,const Record*,Record*,uint32_t);
// TS3W.exe 0x005746F0: Assign record range and return the destination end, preserving x87 scalar-copy behavior.
Record* CopyRecords(const Record*,const Record*,Record*);
// TS3W.exe 0x00574B30: Assign a record vector; overflow discards storage without allocation.
void AssignRange(Range*,const Record*,const Record*,uint32_t);
// TS3W.exe 0x00575140: Assign a group including its scalar and vector tail, quieting scalar sNaNs.
Group* AssignGroup(Group*,const Group*);
// TS3W.exe 0x005751C0: Assign a group range backwards.
Group* CopyBackward(const Group*,const Group*,Group*);
// TS3W.exe 0x00575200: Copy construct a group with inline storage, quieting scalar sNaNs.
Group* ConstructGroup(Group*,const Group*);
// TS3W.exe 0x00575280: Construct groups into a destination range.
Group** ConstructGroups(Group**,const Group*,const Group*,Group*,uint32_t);
// TS3W.exe 0x005752D0: Insert a group, preserving the original nonallocating overflow path.
void Insert(Groups*,Group*,const Group*);
// TS3W.exe 0x005753C0: Append positive-weight contributions and accumulate the total.
void Append(Groups*,const Group*);
}

#if defined(_M_IX86)
namespace ts3::anim::b19 {
inline uint8_t __fastcall HookConsumeDirty(DirtyState* s, void*) { return ConsumeDirty(s); }
inline Coherent* __fastcall HookInitCoherent(Coherent* s, void*) { return InitCoherent(s); }
inline Coherent* __fastcall HookGetCoherent(CoherentOwner* s, void*, bool b) { return GetCoherent(s,b); }
inline Ref* __fastcall HookFind(Cache* s, void*, uint32_t n) { return Find(s,n); }
inline void __fastcall HookPut(Cache* s, void*, Ref* p, uint32_t n) { return Put(s,p,n); }
inline void __fastcall HookClear(Cache* s, void*) { return Clear(s); }
inline void __fastcall HookAssignRange(Range* s, void*, const Record* f, const Record* l, uint32_t a) { return AssignRange(s,f,l,a); }
inline Group* __fastcall HookAssignGroup(Group* s, void*, const Group* p) { return AssignGroup(s,p); }
inline Group* __fastcall HookConstructGroup(Group* s, void*, const Group* p) { return ConstructGroup(s,p); }
inline void __fastcall HookInsert(Groups* s, void*, Group* p, const Group* q) { return Insert(s,p,q); }
inline void __fastcall HookAppend(Groups* s, void*, const Group* p) { return Append(s,p); }
}
#endif
