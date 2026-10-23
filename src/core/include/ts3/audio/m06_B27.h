#pragma once
#include "ts3/layout.h"
namespace ts3::audio::m06_B27 {
struct Link { Link* left; Link* right; Link* parent; uint32_t color; };
struct SoundEntry { Link link; uint32_t key; void* sound; };
struct Tree { void* allocator; Link sentinel; uint32_t size; SoundEntry* free; alignas(void*) uint8_t pool[144]; };
struct Pair { uint32_t key[2]; Tree tree; };
struct Node { Pair value; uint32_t cached; Node* next; uint32_t unused; };
struct Hash { uint32_t unknown; Node** buckets; uint32_t count,size; uint32_t policy[3]; const char* label; };
struct Iterator { Node* node; Node** bucket; };
struct InsertResult { Node* node; Node** bucket; uint8_t inserted; };
struct Data { void** table; volatile long refs; uint32_t count; void* bytes; uint32_t size; };
struct QueueLink { QueueLink* next; QueueLink* prev; };
struct QueueNode { QueueLink link; Data* data; };
struct SoundArray {
    void* table; uint32_t unknown; Hash hash; QueueLink queue; const char* label; uint32_t reserved;
    alignas(void*) uint8_t mutex[sizeof(void*)==4?24:40]; uint32_t reserved2[2];
    void* mode; void* modeEnd; void* modeCap; uint32_t reserved3; uint32_t active; uint32_t reserved4[2]; float update;
};
struct Deque { void** map; uint32_t capacity; void* current; void* first; void* last; void** begin;
    void* currentEnd; void* firstEnd; void* lastEnd; void** end; const char* label; };
TS3_X86_SIZE(Tree,0xAC); TS3_X86_SIZE(Pair,0xB4); TS3_X86_SIZE(Node,0xC0);
TS3_X86_OFFSET(Node,next,0xB8); TS3_X86_OFFSET(SoundArray,queue,0x28);
TS3_X86_OFFSET(SoundArray,mutex,0x38); TS3_X86_OFFSET(SoundArray,active,0x68);
TS3_X86_OFFSET(SoundArray,update,0x74); TS3_X86_OFFSET(Deque,end,0x24);
// TS3W.exe 0x00871D60: Construct a key/tree pair and consume the by-value source tree.
Pair* MakePair(Pair* out,uint32_t low,uint32_t high,Tree source);
// TS3W.exe 0x00871DA0: Copy a key/tree pair through the typed tree-copy boundary.
Pair* CopyPair(Pair* out,const Pair* source);
// TS3W.exe 0x00871DD0: Allocate and copy a hash node; keep null-allocation dereference.
Node* AllocateNode(Hash* self,const Pair* source);
// TS3W.exe 0x00871E20: Stop active sounds, clear the hash and queued data under a recursive lock.
void Clear(SoundArray* self);
// TS3W.exe 0x00871F30: Remove a handle from every active sound tree and recycle empty hash nodes.
void Remove(SoundArray* self,uint32_t handle);
// TS3W.exe 0x00872050: Stop active sounds without resetting the active flag or clearing queued data.
void Stop(SoundArray* self);
// TS3W.exe 0x00872140: Find or insert a key pair, consulting the original prime rehash policy.
InsertResult* Insert(Hash* self,InsertResult* out,const Pair* value,uint32_t ignored);
// TS3W.exe 0x00872220: Destroy a sound array; retain the final cleanup EAX result.
uintptr_t Destroy(SoundArray* self);
// TS3W.exe 0x00872290: Return the insertion output pointer (verified InsertAdapter twin).
InsertResult* Find(Hash* self,InsertResult* out,const Pair* value);
// TS3W.exe 0x00872C60: Process the front retained packet and reset its last reference before deletion.
void Process(SoundArray* self);
// TS3W.exe 0x00872D00: Retain and enqueue packet data, process immediately and return AL=1.
uint8_t Enqueue(SoundArray* self,uint32_t count,void* bytes,uint32_t size);
// TS3W.exe 0x00872E00: Poll nonempty packet queues under the recursive mutex; ignore both arguments.
void Poll(SoundArray* self,uint32_t ignored1,uint32_t ignored2);
// TS3W.exe 0x00872E50: Relocate or grow a deque pointer map; return the first block's end.
void* Relocate(Deque* self,uint64_t request);
struct Api {
    Pair* CopyPair(void* p) { return m06_B27::CopyPair(reinterpret_cast<Pair*>(this),static_cast<Pair*>(p)); }
    Node* AllocateNode(void* p) { return m06_B27::AllocateNode(reinterpret_cast<Hash*>(this),static_cast<Pair*>(p)); }
    void Clear() { m06_B27::Clear(reinterpret_cast<SoundArray*>(this)); }
    void Remove(uint32_t h) { m06_B27::Remove(reinterpret_cast<SoundArray*>(this),h); }
    void Stop() { m06_B27::Stop(reinterpret_cast<SoundArray*>(this)); }
    InsertResult* Insert(void* o,void* p,uint32_t b) { return m06_B27::Insert(reinterpret_cast<Hash*>(this),static_cast<InsertResult*>(o),static_cast<Pair*>(p),b); }
    uintptr_t Destroy() { return m06_B27::Destroy(reinterpret_cast<SoundArray*>(this)); }
    InsertResult* Find(void* o,void* p) { return m06_B27::Find(reinterpret_cast<Hash*>(this),static_cast<InsertResult*>(o),static_cast<Pair*>(p)); }
    void Process() { m06_B27::Process(reinterpret_cast<SoundArray*>(this)); }
    uint8_t Enqueue(uint32_t n,void* p,uint32_t z) { return m06_B27::Enqueue(reinterpret_cast<SoundArray*>(this),n,p,z); }
    void Poll(uint32_t a,uint32_t b) { m06_B27::Poll(reinterpret_cast<SoundArray*>(this),a,b); }
    void* Relocate(uint32_t low,uint32_t high) { return m06_B27::Relocate(reinterpret_cast<Deque*>(this),(uint64_t(high)<<32)|low); }
};
}
