#pragma once
#include "ts3/anim/m05_layouts.h"
#include "ts3/swarm/clu_405a50_Swarm.h"
namespace ts3::anim::m82 {
struct InlineString { char* begin; char* end; char* capacity; void* descriptor; char storage[96]; };
struct Channel {
    InlineString name, bone;
    unsigned char opaqueE0[0x20];
    uint32_t namespaceIndex, nameHash, boneHash, trailing;
};
struct ChannelVector { Channel* begin; Channel* end; Channel* capacity; void* descriptor; Channel storage[10]; };
struct Slot { uint32_t index, opaque04; ChannelVector channels; };
struct SlotVector { Slot* begin; Slot* end; Slot* capacity; void* descriptor; };
struct SourceChannel { uint32_t value; char name[512]; char bone[512]; };
struct SourceSlot { uint32_t opaque; uint32_t count; SourceChannel* channels[1]; };
struct SourceSlots { uint32_t count; SourceSlot* slots[1]; };
struct Source { unsigned char opaque00[16]; SourceSlots* slots; const char* name; unsigned char opaque18[12]; const void* rotation; };
struct Reader { void* vtable; uint32_t opaque04; void* stream; uint32_t opaque0c; uint32_t endian; uint8_t good; };
struct String { char* begin; char* end; char* capacity; const char* descriptor; };
TS3_X86_SIZE(InlineString,112);
TS3_X86_SIZE(Channel,272);
TS3_X86_SIZE(Slot,2744);
TS3_X86_OFFSET(Source,rotation,0x24);
TS3_X86_OFFSET(Reader,good,0x14);
// TS3W.exe 0x00578DE0: Insert copies into the slot vector; preserves capacity-pointer and helper return values.
uintptr_t Insert(SlotVector*,Slot*,uint32_t,const Slot*);
// TS3W.exe 0x00579000: Resize the slot vector, returning the signed byte delta on shrink.
uintptr_t Resize(SlotVector*,uint32_t);
// TS3W.exe 0x005790A0: Populate IK slots, namespaces, name hash and optional rotation from a clip view.
uint8_t Populate(ClipIkInfo*,const Source*,uint32_t);
// TS3W.exe 0x00579490: Deserialize IK metadata and channels, clearing flag bits 2 and 3 even after failed reads.
uint8_t Read(ClipIkInfo*,Reader*,void*);
// TS3W.exe 0x00579F50: Resolve a clip ID from a string range, optionally returning its unqualified hash.
uint64_t ClipId(const String*,uint64_t*);
// TS3W.exe 0x0057A400: Resolve a clip ID from a C string, returning zero for null.
uint64_t ClipIdCString(const char*,uint64_t*);
inline uintptr_t __fastcall HookInsert(SlotVector* s, void*, Slot* p, uint32_t n, const Slot* v) { return Insert(s,p,n,v); }
inline uintptr_t __fastcall HookResize(SlotVector* s, void*, uint32_t n) { return Resize(s,n); }
inline uint8_t __fastcall HookPopulate(ClipIkInfo* s, void*, const Source* v, uint32_t n) { return Populate(s,v,n); }
inline uint8_t __fastcall HookRead(ClipIkInfo* s, void*, Reader* r, void* p) { return Read(s,r,p); }
}
