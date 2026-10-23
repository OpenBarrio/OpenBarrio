#pragma once
#include "ts3/layout.h"

namespace ts3::misc {
struct GuidWords { uint32_t words[4]; };
// TS3W.exe 0x004A3890: compare all four GUID words, in EA's order.
int EqualGuid(const void* a, const void* b);
// TS3W.exe 0x004A3CF0: unsigned lexicographic comparison (word 0, 1, 3, 2).
bool __stdcall LessGuid(const void* a, const void* b);
struct NameNode { const char16_t* name; NameNode* next; };
// TS3W.exe 0x004A11E0: find an exact UTF-16 name in a singly-linked chain; arg 3 is ignored.
NameNode* __stdcall FindName(NameNode* head, const char16_t* const* name, int unused);
struct ValueOwner {
    uint8_t unknown[840]; int32_t value;
    // TS3W.exe 0x004A69F0: read the integer at +0x348.
    int32_t GetValue() const;
};
TS3_X86_OFFSET(ValueOwner, value, 840);
struct ResourceKey {
    uint16_t kind; uint8_t ignored[6]; uint32_t words[8]; GuidWords guid;
    // TS3W.exe 0x00C4B800: compare semantic key fields, ignoring bytes 2..7.
    int Equals(const void* other) const;
};
TS3_X86_SIZE(ResourceKey, 56);
struct ResourceNode { ResourceKey key; uint32_t value; uint32_t unknown; ResourceNode* next; };
TS3_X86_OFFSET(ResourceNode, next, 64);
struct ResourceIterator { ResourceNode* node; ResourceNode** bucket; };
struct ResourceMap {
    void* unknown; ResourceNode** buckets; uint32_t bucketCount; uint8_t tail[52];
    // TS3W.exe 0x00C4F670: find a key in its hash bucket, or return the sentinel iterator.
    ResourceIterator* Find(ResourceIterator* out, const ResourceKey* key);
    // TS3W.exe 0x00C50E80: get or insert a zero-valued key, returning the value address.
    uint32_t* GetOrInsert(const ResourceKey* key);
};
TS3_X86_SIZE(ResourceMap, 64);
struct ResourceIds {
    uint8_t unknown[340]; ResourceMap map; uint32_t nextId;
    // TS3W.exe 0x00C517D0: intern a resource key, allocating the next ID only on a miss.
    uint32_t Intern(const ResourceKey* key);
};
TS3_X86_OFFSET(ResourceIds, map, 340);
TS3_X86_OFFSET(ResourceIds, nextId, 404);
}

namespace ts3::misc {
struct ThreadState {
    void* vtable; uint8_t initialized; uint8_t unknown[539];
    void* event; void* thread; void* extraEvent; uint8_t tail[20]; uint8_t running;
    // TS3W.exe 0x004DAA70: signal and join an existing worker, closing its handle.
    bool Stop();
    // TS3W.exe 0x004DAB70: stop and close the initialized worker's events.
    bool Close();
};
TS3_X86_OFFSET(ThreadState, event, 544);
TS3_X86_OFFSET(ThreadState, thread, 548);
TS3_X86_OFFSET(ThreadState, running, 576);
struct OwnedList {
    void* unknown; void* next; void* prev; uint32_t count; uint8_t flag; uint8_t padding[3]; uint32_t extra;
};
struct WorldWorker {
    uint16_t active; uint8_t unknown[514]; ThreadState worker; OwnedList list;
    // TS3W.exe 0x004A4460: shut down an active world worker and clear its list.
    bool Shutdown();
};
TS3_X86_OFFSET(WorldWorker, worker, 516);
TS3_X86_OFFSET(WorldWorker, list, 1096);
struct UiLink { UiLink* next; UiLink* prev; uint8_t unknown[28]; void* object; };
TS3_X86_OFFSET(UiLink, object, 36);
struct UiCleanup {
    void* vtable; uint8_t unknown[560]; void* storage; uint8_t pad1[12]; void* inlineStorage;
    uint8_t pad2[144]; UiLink links;
    // TS3W.exe 0x004A1280: unlink UI owners, release their interfaces, and destroy the base.
    int Cleanup();
};
TS3_X86_OFFSET(UiCleanup, storage, 564);
TS3_X86_OFFSET(UiCleanup, inlineStorage, 580);
TS3_X86_OFFSET(UiCleanup, links, 728);
struct TreeItem { TreeItem* next; TreeItem* prev; void* object; };
struct TreeNode {
    TreeNode* left; TreeNode* right; uint8_t unknown[24]; void* data;
    void* end; uintptr_t capacity; uint8_t pad[12]; TreeItem items; uint32_t tail;
};
TS3_X86_OFFSET(TreeNode, data, 32);
TS3_X86_OFFSET(TreeNode, capacity, 40);
TS3_X86_OFFSET(TreeNode, items, 56);
TS3_X86_SIZE(TreeNode, 72);
struct TreeOwner {
    uint8_t unknown[24]; void* allocator;
    // TS3W.exe 0x004A4D40: recursively release the left subtree, list, buffer, and right chain.
    void DestroyTree(TreeNode* node);
};
TS3_X86_OFFSET(TreeOwner, allocator, 24);
}

namespace ts3::misc {
struct Trigger;
struct TriggerEvent {
    Trigger* owner; uintptr_t result; uintptr_t linked; uint32_t code;
    float x; float y; int8_t kind; int8_t device; uint16_t flags; uint16_t duplicate; uint16_t tail;
};
TS3_X86_SIZE(TriggerEvent, 32);
TS3_X86_OFFSET(TriggerEvent, kind, 24);
struct Trigger {
    Trigger* next; Trigger* prev; uint32_t unknown; void* resolver;
    void (__cdecl* callback)(uint32_t, TriggerEvent*, uintptr_t); uintptr_t context;
    uint8_t flags; int8_t device; uint16_t tail;
};
TS3_X86_OFFSET(Trigger, resolver, 12);
TS3_X86_OFFSET(Trigger, callback, 16);
TS3_X86_OFFSET(Trigger, flags, 24);
struct TriggerDispatcher {
    Trigger* next; Trigger* prev; TriggerEvent events[32]; uint32_t count; uint32_t mask;
    // TS3W.exe 0x0049D1D0: find an active event by code and signed kind/device.
    TriggerEvent* FindCode(uint32_t code, int kind, int device);
    // TS3W.exe 0x0049D220: find a matching result with at least one requested flag.
    TriggerEvent* FindResult(uintptr_t result, int kind, int device, uint32_t flags);
    // TS3W.exe 0x0049D280: swap-remove an event, notify, and remove its linked event.
    uintptr_t Remove(TriggerEvent* event, uint32_t message);
    // TS3W.exe 0x0049D330: mark duplicate bindings before sending a release notification.
    uintptr_t Release(TriggerEvent* event);
    // TS3W.exe 0x0049D390: remove events of a kind/device, with 7/-2 wildcards.
    uintptr_t Clear(int kind, int device);
    // TS3W.exe 0x0049D860: unlink a trigger and remove all its active events.
    uintptr_t Detach(Trigger* trigger);
    // TS3W.exe 0x0049DAF0: resolve an activation through the reverse trigger chain.
    Trigger* Resolve(TriggerEvent* event, uintptr_t* result, uint32_t* flags, uintptr_t* linked);
    // TS3W.exe 0x0049DB70: resolve a continuous event through the reverse trigger chain.
    Trigger* ResolveAxis(TriggerEvent* event, uintptr_t* result, uint32_t* flags);
    // TS3W.exe 0x0049DBE0: activate/deactivate an event, preserving swap-remove return behavior.
    uintptr_t Dispatch(TriggerEvent* event, int active);
    // TS3W.exe 0x0049DE20: dispatch a one-dimensional axis with hysteresis.
    uintptr_t Axis(uint32_t code, int device, uint32_t kind, float value, float threshold, float hysteresis, uint8_t* state);
    // TS3W.exe 0x0049E050: dispatch a radial-deadzone two-dimensional axis.
    uintptr_t Stick(uint32_t code, int device, uint32_t kind, float x, float y, float deadzone, uint8_t* state);
    // TS3W.exe 0x0049E230: dispatch a relative-motion event with the fixed code 1100.
    uintptr_t Motion(int device, uint32_t kind, uint32_t x, uint32_t y, uint32_t flags);
    // TS3W.exe 0x0049E2F0: resolve a text event using code 254, then notify with the actual code.
    uintptr_t Text(uint32_t code, int device, uint32_t kind, uint32_t flags);
    // TS3W.exe 0x0049E700: construct a digital event and dispatch its low-byte activation.
    uintptr_t Digital(uint32_t code, int device, uint32_t kind, int active, uint32_t flags);
};
TS3_X86_OFFSET(TriggerDispatcher, count, 1032);
TS3_X86_OFFSET(TriggerDispatcher, mask, 1036);
}

namespace ts3::misc {
struct TriggerWinProc {
    uint8_t unknown[16]; TriggerDispatcher* dispatcher; void* lock; uint8_t pad[48]; uint8_t wheelState; uint8_t captured;
    // TS3W.exe 0x007F0010: press a mouse binding and capture the window when accepted.
    bool MouseDown(void* window, int unused1, int unused2, uint32_t code, uint32_t flags);
    // TS3W.exe 0x007F00A0: cancel keyboard or mouse state on focus changes.
    bool Focus(void* window, int mode, int unused);
    // TS3W.exe 0x007F0100: activate a keyboard binding under the dispatcher lock.
    bool KeyDown(void* window, int unused, uint32_t code, uint32_t flags);
    // TS3W.exe 0x007F0150: release a keyboard binding under the dispatcher lock.
    bool KeyUp(void* window, int unused, uint32_t code, uint32_t flags);
    // TS3W.exe 0x007F01A0: dispatch the low UTF-16 word as a text event.
    bool Character(void* window, int unused, uint32_t code);
    // TS3W.exe 0x007F01F0: release mouse capture and the mouse binding.
    bool MouseUp(void* window, int unused1, int unused2, uint32_t code, uint32_t flags);
    // TS3W.exe 0x007F0260: forward relative mouse movement, always returning false.
    bool MouseMove(void* window, float x, float y, uint32_t flags);
    // TS3W.exe 0x007F02A0: scale wheel ticks and dispatch with configured hysteresis.
    bool Wheel(void* window, int unused1, int unused2, int unused3, int ticks);
    // TS3W.exe 0x007F0330: activate a gamepad digital binding.
    bool ButtonDown(void* window, int device, uint32_t code);
    // TS3W.exe 0x007F0380: release a gamepad digital binding.
    bool ButtonUp(void* window, int device, uint32_t code);
    // TS3W.exe 0x007F03D0: forward a gamepad stick with configured deadzone.
    bool PadStick(void* window, int device, uint32_t code, float x, float y);
    // TS3W.exe 0x007F0450: forward a gamepad axis with configured hysteresis.
    bool PadAxis(void* window, int device, uint32_t code, float value);
};
TS3_X86_OFFSET(TriggerWinProc, dispatcher, 16);
TS3_X86_OFFSET(TriggerWinProc, lock, 20);
TS3_X86_OFFSET(TriggerWinProc, captured, 73);
}
