#pragma once
#include "ts3/layout.h"
#include "ts3/misc/clu_4a6c30_x.h"
namespace ts3::audio {
struct B09Record { uint32_t key, reserved, value0, value1; };
struct B09Node { B09Record value; B09Node* next; uint32_t unused; };
struct B09Map {
    void* opaque;
    B09Node** buckets;
    uint32_t bucketCount, count;
    unsigned char policy[12];
    const char* label;
};
struct B09InsertResult { B09Node* node; B09Node** bucket; bool inserted; };
struct B09ModifierNode { uint32_t key; void* modifier; B09ModifierNode* next; };
struct B09ModifierMap {
    void* opaque; B09ModifierNode** buckets; uint32_t bucketCount, count;
    unsigned char policy[12]; const char* label;
};
struct B09SoundLink { B09SoundLink* first; B09SoundLink* next; };
struct B09SoundHead { void** vtable; unsigned char prefix[16-sizeof(void*)]; B09SoundLink link; };
struct B09Candidate { B09SoundHead head; unsigned char gap[0xadc-24]; uint32_t flags; };
struct B09Query {
    unsigned char prefix[16]; float priority; uint32_t reserved; uint64_t key;
    unsigned char gap[24]; uint32_t type; void* keyBegin; void* keyEnd;
    uint32_t gap44[3]; uint32_t id; void* idBegin; void* idEnd;
};
struct B09Manager {
    unsigned char prefix[0x23ecc]; B09SoundHead soundList;
    unsigned char beforeRecords[0x27bd0-0x23ee4]; B09Map records;
    void* stream; uint32_t streamState; B09ModifierMap modifiers;
    unsigned char gap1a[0x40]; uint32_t pendingState;
    unsigned char gap1b[0x27d04 - 0x27c5c]; void* provider; uint32_t providerState;
    B09ModifierMap callbacks;
    uint32_t soundComparator; misc::ResourceTreeNode soundAnchor; uint32_t soundCount;
    const char* soundLabel; float position[3];
};
TS3_X86_OFFSET(B09Manager,modifiers,0x27bf8);
TS3_X86_OFFSET(B09Manager,callbacks,0x27d0c);
TS3_X86_OFFSET(B09Map,buckets,4);
TS3_X86_OFFSET(B09Map,label,28);
TS3_X86_OFFSET(B09Node,next,16);
// TS3W.exe 0x00866190: Erase the first contiguous run of matching hash records.
uint32_t B09Erase(B09Map* map, const uint32_t* key);
// TS3W.exe 0x00866200: Insert a unique record, preserving the allocator and rehash policy.
B09InsertResult* B09Insert(B09Map* map, B09InsertResult* result, const B09Record* value, uint32_t hint);
// TS3W.exe 0x00866B80: Erase a manager record by key.
uint32_t B09ManagerErase(B09Manager* manager, uint32_t key);
// TS3W.exe 0x00866C60: Insert a manager record; the reserved payload word is unspecified in EA.
void B09ManagerInsert(B09Manager* manager, uint32_t key, uint32_t value0, uint32_t value1);
// TS3W.exe 0x00866CA0: Insert a key/callback pair through the shared unique-map helper.
void B09RegisterCallback(B09Manager* manager, uint32_t key, void* callback);
// TS3W.exe 0x00866F80: Register a nonnull modifier and retain it only on successful insertion; AL is the return contract.
uint8_t B09RegisterModifier(B09Manager* manager, uint32_t key, void* modifier);
struct B09TreeValue { uint32_t key; void* object; };
struct B09TreeNode { misc::ResourceTreeNode links; B09TreeValue value; };
struct B09Tree {
    uint32_t comparator; misc::ResourceTreeNode anchor; uint32_t count; const char* label;
};
// TS3W.exe 0x00866EE0: Allocate a tree node, retain its value, then link and publish it.
B09TreeNode** B09TreeInsert(B09Tree* tree, B09TreeNode** output, B09TreeNode* position, const B09TreeValue* value, uint32_t forceLeft);
// TS3W.exe 0x00866FE0: Unlink a tree node and release its value, resetting the last reference before deletion.
B09TreeNode** B09TreeErase(B09Tree* tree, B09TreeNode** output, B09TreeNode* node);
// TS3W.exe 0x00866AC0: Notify retained sounds before forwarding an eligible resource event to the base system.
void B09NotifySounds(B09Manager* manager, uint32_t kind, uintptr_t payload, uintptr_t arg2, uintptr_t arg3, void* event);
// TS3W.exe 0x00866BA0: Update routing position, base system, then each retained sound in tree order.
void B09UpdateSounds(B09Manager* manager, float first, float second);
// TS3W.exe 0x00866320: Clear provider and modifier ownership before shutting down the base system; return its AL.
uint8_t B09Shutdown(B09Manager* manager);
// TS3W.exe 0x00866000: Select and retain an eligible sound, preserving EA's ordered comparisons and tie replacement.
uint8_t B09SelectSound(B09Manager* manager, const B09Query* query, B09Candidate** output);
// TS3W.exe 0x008663D0: Remove a one-shot script callback before invoking it with three boxed arguments.
void B09InvokeCallback(B09Manager* manager, uint32_t kind, uint32_t key, void* middle, uint32_t last);
// TS3W.exe 0x00866CE0: Resolve an event's modifier IDs with underscore-prefix fallback and publish the resulting key.
void B09DispatchEvent(B09Manager* manager, void* event);
}

namespace ts3::audio {
// TS3W.exe 0x00866000: ABI adapter for B09SelectSound.
uint8_t __fastcall HookB09SelectSound(B09Manager* manager, void*, const B09Query* query, B09Candidate** output);
// TS3W.exe 0x00866190: ABI adapter for B09Erase.
uint32_t __fastcall HookB09Erase(B09Map* map, void*, const uint32_t* key);
// TS3W.exe 0x00866200: ABI adapter for B09Insert.
B09InsertResult* __fastcall HookB09Insert(B09Map* map, void*, B09InsertResult* result, const B09Record* value, uint32_t hint);
// TS3W.exe 0x00866320: ABI adapter for B09Shutdown.
uint8_t __fastcall HookB09Shutdown(B09Manager* manager, void*);
// TS3W.exe 0x008663D0: ABI adapter for B09InvokeCallback.
void __fastcall HookB09InvokeCallback(B09Manager* manager, void*, uint32_t kind, uint32_t key, void* middle, uint32_t last);
// TS3W.exe 0x00866AC0: ABI adapter for B09NotifySounds.
void __fastcall HookB09NotifySounds(B09Manager* manager, void*, uint32_t kind, uintptr_t payload, uintptr_t arg2, uintptr_t arg3, void* event);
// TS3W.exe 0x00866B80: ABI adapter for B09ManagerErase.
uint32_t __fastcall HookB09ManagerErase(B09Manager* manager, void*, uint32_t key);
// TS3W.exe 0x00866BA0: ABI adapter for B09UpdateSounds.
void __fastcall HookB09UpdateSounds(B09Manager* manager, void*, float first, float second);
// TS3W.exe 0x00866C60: ABI adapter for B09ManagerInsert.
void __fastcall HookB09ManagerInsert(B09Manager* manager, void*, uint32_t key, uint32_t value0, uint32_t value1);
// TS3W.exe 0x00866CA0: ABI adapter for B09RegisterCallback.
void __fastcall HookB09RegisterCallback(B09Manager* manager, void*, uint32_t key, void* callback);
// TS3W.exe 0x00866CE0: ABI adapter for B09DispatchEvent.
void __fastcall HookB09DispatchEvent(B09Manager* manager, void*, void* event);
// TS3W.exe 0x00866EE0: ABI adapter for B09TreeInsert.
B09TreeNode** __fastcall HookB09TreeInsert(B09Tree* tree, void*, B09TreeNode** output, B09TreeNode* position, const B09TreeValue* value, uint32_t forceLeft);
// TS3W.exe 0x00866F80: ABI adapter for B09RegisterModifier.
uint8_t __fastcall HookB09RegisterModifier(B09Manager* manager, void*, uint32_t key, void* modifier);
// TS3W.exe 0x00866FE0: ABI adapter for B09TreeErase.
B09TreeNode** __fastcall HookB09TreeErase(B09Tree* tree, void*, B09TreeNode** output, B09TreeNode* node);
}
