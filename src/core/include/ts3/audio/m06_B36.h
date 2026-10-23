#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::audio::b36 {
struct Node { uint32_t key; void* value; Node* next; };
struct Map {
    void* allocator;
    Node** buckets;
    uint32_t bucketCount, count;
    float load, growth;
    uint32_t spare;
    void* freeList;
    void* poolOwner;
    void* poolBegin;
    void* poolEnd;
    uint32_t stride;
    void* special;
};
TS3_X86_OFFSET(Map, freeList, 0x1c);
TS3_X86_SIZE(Map, 0x34);
struct Emitter {
    void** primary;
    void** secondary;
    uint32_t refs, unknown, enabled;
    Map voices;
    uint8_t voiceStorage[0xff0-sizeof(uint32_t)*13];
    Map values;
    uint8_t valueStorage[0x438-sizeof(uint32_t)*13];
    Map states;
};
TS3_X86_OFFSET(Emitter, voices, 0x14);
TS3_X86_OFFSET(Emitter, values, 0x1004);
TS3_X86_OFFSET(Emitter, states, 0x143c);
// TS3W.exe 0x008A30C0: Read a stored parameter through its virtual getter, leaving output untouched on miss.
bool ReadValue(Emitter*, uint32_t key, float* out);
// TS3W.exe 0x008A3110: Test membership in the parameter map.
bool HasValue(Emitter*, uint32_t key);
// TS3W.exe 0x008A3150: Update a state interface and broadcast its removal when inactive.
void UpdateState(Emitter*, uint32_t key, uint32_t state);
// TS3W.exe 0x008A3200: Release bucket chains and return internal nodes to the pool, excluding the special node.
void ClearBuckets(Map*, Node** buckets, uint32_t count);
// TS3W.exe 0x008A3280: Retain and insert a voice, then replay active states and unresolved parameters.
bool AddVoice(Emitter*, void* voice, void* callerStackWord);
// TS3W.exe 0x008A3490: Erase a voice, replay state removals and reset the enabled parameter.
bool RemoveVoice(Emitter*, void* voice);
// TS3W.exe 0x008A34DE: Continue a successful removal using the parent frame's captured node and bucket.
bool RemoveVoiceFound(Emitter*, void* voice, Node* node, Node** bucket);
// TS3W.exe 0x008A3590: Replace a voice by insertion before erasing its old key; null is not accepted on a hit.
bool ReplaceVoice(Emitter*, uint32_t key, void* voice);
// TS3W.exe 0x008A3650: Reject nonfinite parameter values, then update or construct a retained parameter.
void SetValue(Emitter*, uint32_t key, float value, uint32_t transition);
// TS3W.exe 0x008A3850: Resolve, construct or update a retained state and broadcast when its state is exactly one.
bool SetState(Emitter*, uint32_t key, uint32_t state);
struct FixedMap { Map map; Node* inlineBuckets[52]; Node inlineNodes[50]; };
TS3_X86_OFFSET(FixedMap, inlineNodes, 0x104);
// TS3W.exe 0x008A3C80: Construct a fixed hash map with 51 buckets and fifty pooled retained nodes.
FixedMap* ConstructMap(FixedMap*, uint32_t policy, uint32_t option);
// TS3W.exe 0x008A3A00: Detach states and clear both state and parameter maps.
bool Reset(Emitter*);
// TS3W.exe 0x008A3B70: Destroy three retained maps in state/parameter/voice order.
void* Destroy(Emitter*);
// TS3W.exe 0x008A3C70: Adjust secondary this and forward the deleting destructor (verified near twin).
void* DestroySecondary(void*, uint32_t flags);

#if defined(_M_IX86)
bool __fastcall HookReadValue(Emitter* s, void*, uint32_t k, float* o);
bool __fastcall HookHasValue(Emitter* s, void*, uint32_t k);
void __fastcall HookUpdateState(Emitter* s, void*, uint32_t k, uint32_t a);
void __fastcall HookClearBuckets(Map* s, void*, Node** b, uint32_t n);
bool __fastcall HookAddVoice(Emitter* s, void*, void* v);
bool __fastcall HookRemoveVoice(Emitter* s, void*, void* v);
bool __fastcall HookReplaceVoice(Emitter* s, void*, uint32_t k, void* v);
void __fastcall HookSetValue(Emitter* s, void*, uint32_t k, float f, uint32_t t);
bool __fastcall HookSetState(Emitter* s, void*, uint32_t k, uint32_t a);
FixedMap* __fastcall HookConstructMap(FixedMap* s, void*, uint32_t a, uint32_t b);
bool __fastcall HookReset(Emitter* s, void*);
void* __fastcall HookDestroy(Emitter* s, void*);
void* __fastcall HookDestroySecondary(void* s, void*, uint32_t flags);
#endif
}
