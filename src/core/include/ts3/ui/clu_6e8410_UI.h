#pragma once
#include "ts3/layout.h"
namespace ts3::ui {
struct UiLink { UiLink* next; UiLink* prev; };
struct UiOwner { unsigned char pad[28]; void* value; };
TS3_X86_OFFSET(UiOwner, value, 28);
TS3_X86_SIZE(UiLink, 8);
struct UiEntry {
    void** vtable; unsigned char pad04[36];
    UiLink link; UiOwner* owner; void* arg52; UiEntry* arg56;
    unsigned char pad60[20]; uint32_t active;
    unsigned char pad84[316]; UiEntry* context; UiLink children;
    unsigned char pad412[109]; uint8_t flags521; unsigned char pad522[6];
    unsigned char binding[140]; uint8_t flags668; unsigned char pad669[23];
    uintptr_t (*callback)(UiEntry*); unsigned char pad696[8]; uint32_t first, second;
    // TS3W.exe 0x006E8410: copy two scalar fields, returning the second.
    uint32_t GetPair(void* a, void* b);
    // TS3W.exe 0x006F4910: update binding flags and notify the owner.
    uint8_t SetBinding(uint32_t a, uint32_t b, void* callbackArg);
    // TS3W.exe 0x006F4ED0: invoke the optional cdecl callback.
    uintptr_t Invoke();
    // TS3W.exe 0x006F4F70: short-circuit two low-byte binding predicates.
    int HasBinding();
    // TS3W.exe 0x006FA870: test the scalar at +0x50.
    int IsActive();
    // TS3W.exe 0x006FAC50: return the pointer at +0x190 (IDA's scheduler name is spurious).
    UiEntry* GetContext();
    // TS3W.exe 0x006FAD00: get the owner's +0x1c pointer, or null.
    void* GetOwnerValue();
    // TS3W.exe 0x006FC9E0: notify each child; EAX is undefined for an empty list.
    uintptr_t NotifyChildren();
};
TS3_X86_OFFSET(UiEntry, link, 40); TS3_X86_OFFSET(UiEntry, owner, 48);
TS3_X86_OFFSET(UiEntry, active, 80); TS3_X86_OFFSET(UiEntry, context, 400);
TS3_X86_OFFSET(UiEntry, binding, 528); TS3_X86_OFFSET(UiEntry, callback, 692);
TS3_X86_OFFSET(UiEntry, first, 704); TS3_X86_SIZE(UiEntry, 712);
struct alignas(16) UiIdentity {
    uint32_t tag; unsigned char untouched[12]; float matrix[16];
    // TS3W.exe 0x006FAE00: write tag and identity matrix, leaving bytes 4..15 untouched.
    UiIdentity* Init();
};
TS3_X86_SIZE(UiIdentity, 80);
struct UiManager {
    unsigned char pad[0x1028]; void* sentinel;
    unsigned char pad102c[8]; unsigned char map[4]; void** buckets; uint32_t bucketIndex;
    // TS3W.exe 0x00700150: look up the entry and update its binding or release the argument.
    uint8_t Bind(uint32_t lo, uint32_t hi, uint32_t a, uint32_t b, void* arg);
    // TS3W.exe 0x00701FF0: update entry identity and map membership, then resolve its context.
    bool Update(UiEntry* entry, uint64_t key, int32_t kind, int32_t mode, uint8_t flag);
};
TS3_X86_OFFSET(UiManager, map, 0x1034); TS3_X86_OFFSET(UiManager, buckets, 0x1038);
struct UiCleanup {
    unsigned char pad[612]; UiEntry* primary; UiEntry* secondary; UiEntry* tertiary;
    unsigned char pad624[8]; UiEntry** begin; void* unused636; UiEntry** blockEnd; UiEntry*** block;
    UiEntry** end; unsigned char pad652[20]; void* resource; UiEntry* entry;
    // TS3W.exe 0x00AE3FD0: detach entries, optionally release resource and primary.
    void Clear(uint8_t release, uint8_t notify);
};
TS3_X86_OFFSET(UiCleanup, begin, 632); TS3_X86_OFFSET(UiCleanup, resource, 672);
struct UiAnimation {
    UiEntry base; unsigned char pad712[2188-712]; int32_t index, selection; float last;
    unsigned char pad2200[200]; int32_t counts[25]; float threshold;
    // TS3W.exe 0x00C117F0: advance the selected animation when crossing a time threshold.
    void Tick();
};
// +592/+596/+597 are within the base's opaque binding storage.
TS3_X86_OFFSET(UiAnimation, index, 2188); TS3_X86_OFFSET(UiAnimation, threshold, 2500);
}
