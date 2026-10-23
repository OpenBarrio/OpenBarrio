#pragma once
#include "ts3/render/m07_abi.h"
#include <cstdint>
namespace ts3::render::m07::b33 {
// Sparse native views. Unknown words/padding are deliberately not initialized.
struct Hash {
    void* opaque;
    void** buckets;
    uint32_t bucketCount, size;
    float maxLoad, growth;
    void* pool;
    const char* name;
};
struct Group { uint8_t kind; uint8_t padding[3]; Hash collections, dirty; };
struct alignas(16) WorldTree {
    Hash objects;
    void* owner;
    uint8_t gap[12];
    alignas(16) float bounds[8];
    uint32_t enabled;
    Hash lots;
    void* active;
};
struct Level {
    void* first; void* second;
    Hash dirtyRooms, pending;
    uint32_t unknown48;
    Group groups[5];
    uint32_t state;
};
TS3_X86_SIZE(Hash,0x20);
TS3_X86_SIZE(Group,0x44);
TS3_X86_OFFSET(WorldTree,owner,0x20);
TS3_X86_OFFSET(WorldTree,bounds,0x30);
TS3_X86_OFFSET(WorldTree,lots,0x54);
TS3_X86_OFFSET(WorldTree,active,0x74);
TS3_X86_OFFSET(Level,dirtyRooms,8);
TS3_X86_OFFSET(Level,groups,0x4c);
TS3_X86_OFFSET(Level,state,0x1a0);
struct Constants { void** emptyBuckets; const char* objects; const char* lots; const char* collections; const char* dirty; const char* dirtyRooms; const char* pending; };
extern Constants constants;
struct Key { uint32_t lo,hi; };
struct Entry { Key key; void* value; uint32_t hash; Entry* next; };
struct Iterator { Entry* node; void* bucket; };
struct InsertResult { Iterator iterator; uint8_t inserted; };
struct Node { void** vtable; uint32_t refs; Key key; void* p10; void* p14; void* p18; void* value; };
struct SceneObject { void* prefix[4]; Key key; };
struct Collection { Node** begin; Node** end; Node** capacity; const char* name; Node** local; Node* storage[4]; };
struct Rank { void* room; float weight; };
struct RankArray { Rank* begin; Rank* end; Rank* capacity; const char* name; Rank* local; };
struct RoomLevel { void* room; uint8_t opaque[416]; };
struct LotLevels { uint8_t opaque[16]; RoomLevel levels[12]; };
struct RigSource { uint8_t opaque[548]; uint8_t flags; };
struct EmitterSource { void** vtable; uint8_t opaque[252]; uint8_t flags; };
struct LightingData { uint8_t opaque[16]; EmitterSource** begin; EmitterSource** end; uint8_t gap[40]; void** occluders; void** occludersEnd; };
struct Room { uint8_t opaque[12]; void* id; uint8_t gap[176]; uint32_t width,height; uint8_t tail[48]; float (*transform)[4]; };
struct WorldSettings { uint8_t opaque[260]; void* lights; };
struct OwnerData { uint8_t opaque[32]; WorldSettings* settings; };
struct Owner { OwnerData* data; uint8_t opaque[1692]; Level level; };
struct PointerArray { void** begin; void** end; void** capacity; const char* name; };
struct Globals { PointerArray* worldLights; uint32_t* worldInit; PointerArray* roomOccluders; uint32_t* roomInit; };
extern Globals globals;
struct Dependencies {
    Thiscall<Iterator*,Hash,Iterator*,const Key*> find=nullptr;
    Thiscall<void**,Hash,const Key*> index=nullptr;
    Thiscall<InsertResult*,Hash,InsertResult*,const Key*,uint32_t> insert=nullptr;
    void* (__cdecl* allocate)(uint32_t,const char*,uint32_t,uint32_t,uint32_t,uint32_t)=nullptr;
    Thiscall<void*,void,WorldTree*,uint32_t,uint32_t> lotCtor=nullptr;
    Thiscall<void*,SceneObject> leafData=nullptr;
    Thiscall<Node*,void,SceneObject*,void*,Level*,Group*> leafCtor=nullptr;
    Thiscall<void,Collection,Node**,Node**> grow=nullptr;
    Thiscall<uintptr_t,Group,uint32_t,uint32_t,Node*> append=nullptr;
    Thiscall<Node*,void,uint32_t,uint32_t,void*,void*,Level*,Group*> emitterCtor=nullptr;
    Thiscall<Node*,void,uint32_t,uint32_t,void*,Level*,void*,uint8_t,Group*> rigCtor=nullptr;
    Thiscall<uintptr_t,Level,Node*> populate=nullptr;
    Thiscall<void,void,RankArray*> rank=nullptr;
    void (__cdecl* sort)(Rank*,Rank*)=nullptr;
    Thiscall<void,void> retainRoom=nullptr;
    void (__cdecl* free)(void*)=nullptr;
    Thiscall<RigSource*,Node,uint32_t> getRig=nullptr;
    void* (__cdecl* manager)()=nullptr;
    Thiscall<LightingData*,void,uint32_t,uint32_t> lighting=nullptr;
    Thiscall<void,Node,uint32_t> enable=nullptr;
    Thiscall<uint32_t,Node> grouped=nullptr;
    Thiscall<Node*,Level,void*,void*,uint32_t,uint32_t> emitter=nullptr;
    Thiscall<void*,Level,void*,void*,uint32_t,uint32_t,uint8_t> rig=nullptr;
    Thiscall<void,Node,Node*> link=nullptr;
    Thiscall<Node*,void,uint32_t,uint32_t,LightingData*,Node*,Level*,Group*> occluderCtor=nullptr;
    int (__cdecl* atExit)(void (__cdecl*)())=nullptr;
    Thiscall<void,Room> clearLights=nullptr;
    Thiscall<uintptr_t,Level,Room*,uint32_t> collect=nullptr;
    Thiscall<uintptr_t,void,const float*,PointerArray*> query=nullptr;
    Thiscall<uintptr_t,Room,void*> addLight=nullptr;
    Thiscall<void,Room> clearOccluders=nullptr;
    Thiscall<void,Level,void*,PointerArray*> collectOccluders=nullptr;
    Thiscall<void,Room,void*> addOccluder=nullptr;
    Thiscall<uintptr_t,Level,Room*> updateWorld=nullptr;
    Thiscall<void,Level,Group*,void*,void*> visit=nullptr;
    Thiscall<uintptr_t,Room> finish=nullptr;
};
extern Dependencies dependencies;
// TS3W.exe 0x006C52A0: look up a two-word lighting key and return null for the sentinel.
void* __fastcall Lookup(Group*,void*,uint32_t lo,uint32_t hi);
// TS3W.exe 0x006C5400: lazily allocate a lot tracker, retaining a null insertion on allocation failure.
void* __fastcall GetLot(WorldTree*,void*,uint32_t lo,uint32_t hi);
// TS3W.exe 0x006C59E0: allocate and register a world leaf under its object's two-word key.
Node* __fastcall WorldLeaf(Hash*,void*,SceneObject*);
// TS3W.exe 0x006C5D50: append a node to a lazily created inline collection and return its virtual retain result.
uintptr_t __fastcall Append(Group*,void*,uint32_t lo,uint32_t hi,Node*);
// TS3W.exe 0x006C5E70: create and insert an emitter in both emitter and all-node collections.
Node* __fastcall Emitter(Level*,void*,void* emitter,void* data,uint32_t lo,uint32_t hi);
// TS3W.exe 0x006C5EE0: insert a rig twice, invoke slot 2, then return its value field (no null guard).
void* __fastcall Rig(Level*,void*,void* rig,void* owner,uint32_t lo,uint32_t hi,uint8_t paired);
// TS3W.exe 0x006C6550: create a scene leaf, insert it twice and populate lighting dependencies.
uintptr_t __fastcall SceneLeaf(Level*,void*,void* data,SceneObject*);
TS3_X86_OFFSET(Node,value,0x1c);
TS3_X86_OFFSET(SceneObject,key,0x10);
TS3_X86_SIZE(Collection,0x24);
TS3_X86_OFFSET(LightingData,occluders,0x40);
TS3_X86_OFFSET(Room,width,0xc0);
TS3_X86_OFFSET(Room,transform,0xf8);
TS3_X86_OFFSET(Owner,level,0x6a0);
// TS3W.exe 0x006C5C20: select and retain the highest-ranked active room; skip an already selected room.
void __fastcall SelectRoom(WorldTree*,void*);
// TS3W.exe 0x006C6090: populate rigs, grouped emitters and occluders, returning the original raw pointer result.
uintptr_t __fastcall Populate(Level*,void*,Node*);
// TS3W.exe 0x006C6AB0: refresh room lights and transform the world-light query center with SSE ordering.
uintptr_t __fastcall UpdateWorld(Level*,void*,Room*);
// TS3W.exe 0x006C7010: refresh room occluders, world lights, and node callbacks in original order.
uintptr_t __fastcall UpdateRoom(Level*,void*,Room*);
// TS3W.exe 0x006C52E0: initialize world-object and lot maps, preserving opaque fields and holes.
WorldTree* __fastcall InitWorld(WorldTree*,void*,void* owner);
// TS3W.exe 0x006C5680: initialize collection maps, writing only the low kind byte.
Group* __fastcall InitGroup(Group*,void*,uint8_t kind);
// TS3W.exe 0x006C6F60: initialize five lighting levels and dirty-room maps without clearing padding.
Level* __fastcall InitLevel(Level*,void*);
}
