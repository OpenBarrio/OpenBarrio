#pragma once
#include "ts3/layout.h"
#include "ts3/io/file_stream.h"
#include "ts3/io/memory_stream.h"

namespace ts3::misc {
struct WorkString {
    char16_t* begin;
    char16_t* end;
    char16_t* capacity;
    const char* name;
    uint32_t flags;
};
struct WorkKey { char16_t* begin; char16_t* end; char16_t* capacity; const char* name; };
struct WorkNode { WorkKey key; void* value; WorkNode* next; };
// Scheduler's secondary interface begins at +4; flags are relative to the primary base.
struct WorkQueue {
    void** vtable;
    void** secondary;
    uint32_t references;
    uint8_t active, pad0D[3];
    void* target;
    WorkString* begin;
    WorkString* end;
    WorkString* capacity;
    const char* allocatorName;
    uint8_t pending, notify, pad26[2];
    uint32_t hashPolicy;
    WorkNode** buckets;
    uint32_t bucketCount, nodeCount;
    float limit, interval;
    uint32_t timerFlags;
    const char* timerName;
    uint32_t timer[5];
    // TS3W.exe 0x00408760: decrement the wrapping refcount and delete at zero.
    uint32_t Release();
    // TS3W.exe 0x004087B0: mark the queue active; only AL is defined.
    uint8_t Activate();
    // TS3W.exe 0x004087C0: mark work pending; EAX is unspecified.
    void MarkPending();
    // TS3W.exe 0x004087D0: return the pending byte, not a normalized bool.
    uint8_t IsPending() const;
    // TS3W.exe 0x004087E0: store and return the low argument byte.
    uint8_t SetNotify(uint32_t value);
    // TS3W.exe 0x004088C0: detach the active target and call cleanup slot 7.
    uint8_t Deactivate();
    // TS3W.exe 0x00408910: flush, replace the target with retain/release, then resume.
    uintptr_t SetTarget(void* replacement);
    // TS3W.exe 0x004089B0: process queued scripts, catching EA::ArgScript::cError by reference.
    uint32_t Process();
    // TS3W.exe 0x00408A8E: continuation of Process, stop/reset timing and accumulate time.
    void FinishItem(float& elapsed);
    // TS3W.exe 0x00409420: flush bucket values then release buckets and queued strings.
    uintptr_t Clear();
    // TS3W.exe 0x004096B0: restore class vtables and destroy all owned storage.
    void Destruct();
    // TS3W.exe 0x00409800: destruct and conditionally delete, returning the original this.
    WorkQueue* DeletingDestruct(uint32_t flags);
    // TS3W.exe 0x00504DD0: increment the wrapping refcount.
    uint32_t AddRef();
};
TS3_X86_OFFSET(WorkQueue, references, 8);
TS3_X86_OFFSET(WorkQueue, active, 0xC);
TS3_X86_OFFSET(WorkQueue, target, 0x10);
TS3_X86_OFFSET(WorkQueue, pending, 0x24);
TS3_X86_OFFSET(WorkQueue, buckets, 0x2C);
TS3_X86_OFFSET(WorkQueue, timer, 0x48);
TS3_X86_SIZE(WorkQueue, 0x5C);
TS3_X86_SIZE(WorkString, 20);
TS3_X86_OFFSET(WorkNode, value, 0x10);
TS3_X86_OFFSET(WorkNode, next, 0x14);
struct QueueNotification {
    void** vtable;
    uint32_t references;
    uint8_t active, pad09[3];
    void* target;
    WorkString* begin;
    WorkString* end;
    WorkString* capacity;
    const char* allocatorName;
    uint8_t pending, notify;
    // TS3W.exe 0x00408780: notification on the secondary interface (+4).
    uint8_t Notify(uint32_t message, uintptr_t ignored);
    // TS3W.exe 0x00409770: secondary-interface adjusting deleting destructor.
    WorkQueue* DeletingDestruct(uint32_t flags);
};
TS3_X86_OFFSET(QueueNotification, pending, 0x20);
struct PixelDecoder {
    uint32_t unused[2];
    uint32_t format;
    // TS3W.exe 0x00444180: expand packed RGB, leaving outputs alone for unknown formats.
    uint8_t DecodeRGB(uint32_t pixel, uint8_t* red, uint8_t* green, uint8_t* blue) const;
    // TS3W.exe 0x004442A0: expand RGBA, preserving EA's nonstandard 1555/4444 alpha.
    uint8_t DecodeRGBA(uint32_t pixel, uint8_t* red, uint8_t* green, uint8_t* blue, uint8_t* alpha) const;
    uint8_t DecodeRGBBytes(uint32_t p, void* r, void* g, void* b) const
    { return DecodeRGB(p, static_cast<uint8_t*>(r), static_cast<uint8_t*>(g), static_cast<uint8_t*>(b)); }
    uint8_t DecodeRGBABytes(uint32_t p, void* r, void* g, void* b, void* a) const
    { return DecodeRGBA(p, static_cast<uint8_t*>(r), static_cast<uint8_t*>(g), static_cast<uint8_t*>(b), static_cast<uint8_t*>(a)); }
};
TS3_X86_SIZE(PixelDecoder, 12);
struct CircularValue {
    CircularValue* next;
    CircularValue* prev;
    uint32_t value[3];
    // TS3W.exe 0x00456F30: initialize a self-linked node and copy three dwords.
    CircularValue* Init(const uint32_t* source);
};
TS3_X86_SIZE(CircularValue, 20);
struct CursorPair {
    uint32_t unused;
    void* begin;
    void* current;
    uint8_t pad0C[0x10];
    void* secondBegin;
    void* secondCurrent;
    // TS3W.exe 0x0049DAB0: reset both cursors and return the first begin pointer.
    void* Reset();
    uintptr_t ResetAddress() { return reinterpret_cast<uintptr_t>(Reset()); }
};
TS3_X86_OFFSET(CursorPair, secondBegin, 0x1C);
TS3_X86_SIZE(CursorPair, 0x24);
struct BufferOwner {
    void** vtable;
    io::SharedBuffer* buffer;
    // TS3W.exe 0x004559D0: release the shared buffer and restore the base vtable.
    int32_t Destruct();
};
struct ObserverOwner {
    void* allocation;
    uint8_t pad04[0x14];
    uint8_t observer[0x24];
    void* target;
    uint8_t pad40[0x20];
    BufferOwner stream;
    // TS3W.exe 0x00471160: clear the observer twice around target/buffer cleanup.
    uintptr_t Destruct();
};
TS3_X86_OFFSET(ObserverOwner, observer, 0x18);
TS3_X86_OFFSET(ObserverOwner, target, 0x3C);
TS3_X86_OFFSET(ObserverOwner, stream, 0x60);
struct DdfRecord {
    void** vtable;
    int32_t references;
    io::FileStream file;
    uint32_t key[4];
    void* owner;
    uint8_t opened;
    // TS3W.exe 0x004A42C0: construct the DDF record and embedded closed file.
    DdfRecord* Construct(void* manager, const uint32_t* source);
};
TS3_X86_OFFSET(DdfRecord, file, 8);
TS3_X86_OFFSET(DdfRecord, key, 0x230);
TS3_X86_OFFSET(DdfRecord, owner, 0x240);
TS3_X86_OFFSET(DdfRecord, opened, 0x244);
struct DdfLink { DdfLink* next; DdfLink* prev; void* value; };
struct DdfList {
    WorkKey path;
    void* parameter;
    void* parameter2;
    DdfLink head;
    // TS3W.exe 0x004A4860: initialize a named record list; optionally assign its path.
    DdfList* Construct(const char16_t* source, void* p1, void* p2);
};
TS3_X86_OFFSET(DdfList, head, 0x18);
struct DdfTreeNode { uint8_t treeData[0x38]; DdfLink records; };
using DdfCallback = uintptr_t (*)(uintptr_t, uintptr_t, const char16_t*, uintptr_t);
struct DdfManager {
    uint8_t pad00[0x54];
    void* treeSentinel;
    DdfTreeNode* treeFirst;
    uint8_t pad5C[0x18];
    void* context;
    void* provider;
    DdfCallback callback;
    uint8_t mutex[0x18];
    // TS3W.exe 0x004A39E0: call the custom callback, or bridge to the default resolver.
    uintptr_t Resolve(uintptr_t a, uintptr_t b, const char16_t* path, uintptr_t mode);
    // TS3W.exe 0x004A3D80: OR the file flags for every record under the manager lock.
    uint32_t Flags(DdfList* list);
    // TS3W.exe 0x004A3DE0: resolve/copy a UTF-16 path and release its temporary storage.
    uintptr_t CopyPath(uintptr_t id, char16_t* out);
    // TS3W.exe 0x004A4610: close and release records in every tree node.
    uintptr_t Clear();
};
TS3_X86_OFFSET(DdfManager, treeFirst, 0x58);
TS3_X86_OFFSET(DdfManager, context, 0x74);
TS3_X86_OFFSET(DdfManager, callback, 0x7C);
TS3_X86_OFFSET(DdfManager, mutex, 0x80);
// TS3W.exe 0x004A4590: remove a matching record from the circular list (stdcall ret 8).
#if defined(_M_IX86)
uint8_t __stdcall RemoveDdfRecord(DdfList* list, DdfRecord* record);
#else
uint8_t RemoveDdfRecord(DdfList* list, DdfRecord* record);
#endif
// TS3W.exe 0x00409290: release every hash-chain value and owned key, then clear buckets.
#if defined(_M_IX86)
void __stdcall FreeWorkNodes(WorkNode** buckets, uint32_t count);
#else
void FreeWorkNodes(WorkNode** buckets, uint32_t count);
#endif
// TS3W.exe 0x00409C80: format into the game's 1 KiB buffer and append to the destination.
uintptr_t FormatWork(void* destination, const char* format, ...);
// TS3W.exe 0x0049EF80: deserialize both record tables, preserving read attempts after a failure.
int32_t ReadRecordTables(void* destination, void* stream, uint32_t mode);
} // namespace ts3::misc
