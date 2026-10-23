#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::m31 {
// Opaque boundary payload: its string-bearing internals are handled by game bridges.
struct Entry { unsigned char opaque[0x110]; };
struct EntryRange { Entry* begin; Entry* end; Entry* capacity; };
struct Slot {
    uint32_t id;
    unsigned char opaque04[4];
    EntryRange entries;
    unsigned char opaque14[4];
    Entry storage[10];
};
struct SlotRange { Slot* begin; Slot* end; Slot* capacity; void* descriptor; };
TS3_X86_OFFSET(Slot,entries,8);
TS3_X86_OFFSET(Slot,storage,0x18);
TS3_X86_SIZE(Slot,0xab8);
// TS3W.exe 0x00569840: assign an entry range, retaining the null-base overflow quirk and incidental EAX.
uintptr_t AssignEntries(EntryRange*,Entry*,Entry*,uintptr_t);
// TS3W.exe 0x00569920: assign existing slots and return the destination end.
Slot* CopySlots(Slot*,Slot*,Slot*);
// TS3W.exe 0x00569990: construct slots through an output cursor, including null-destination advancement.
Slot** ConstructSlots(Slot**,Slot*,Slot*,Slot*,uintptr_t);
// TS3W.exe 0x00569A00: allocate and construct a slot range with the original allocator metadata.
Slot* AllocateSlots(SlotRange*,uint32_t,Slot*,Slot*);
// TS3W.exe 0x00569A60: assign a slot vector without destroying its removed suffix.
SlotRange* AssignSlots(SlotRange*,const SlotRange*);
// TS3W.exe 0x00569B90: assign IK fields in string, slot, namespace, hash, scalar order.
ClipIkInfo* AssignIk(ClipIkInfo*,const ClipIkInfo*);
#if defined(_M_IX86)
namespace hooks {
uintptr_t __fastcall AssignEntries(EntryRange*,void*,Entry*,Entry*,uintptr_t);
Slot* __fastcall AllocateSlots(SlotRange*,void*,uint32_t,Slot*,Slot*);
SlotRange* __fastcall AssignSlots(SlotRange*,void*,const SlotRange*);
ClipIkInfo* __fastcall AssignIk(ClipIkInfo*,void*,const ClipIkInfo*);
}
#endif
}
