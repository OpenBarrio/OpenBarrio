#pragma once
#include "ts3/layout.h"
#include "ts3/resource/compressed_index.h"

namespace ts3::resource::m03::delivery {
struct ServiceInterfaceView;
#if defined(_M_IX86)
using ServiceMethod = uint32_t (__thiscall*)(ServiceInterfaceView*);
#else
using ServiceMethod = uint32_t (*)(ServiceInterfaceView*);
#endif
// Bounded borrowed vtable prefix, not a claim about the complete class.
struct ServiceVtableView {
    void* unused[20];
    ServiceMethod lock;
    ServiceMethod finalize;
};
struct ServiceInterfaceView { const ServiceVtableView* vtable; };
// Only the primary prefix and adjusted interface used by this algorithm.
// The opaque primary state is not interpreted; native alignment grows the prefix.
struct ServiceView {
    const void* primaryVtable;
    uint32_t opaqueState;
    ServiceInterfaceView adjusted;
};
TS3_X86_OFFSET(ServiceView, primaryVtable, 0);
TS3_X86_OFFSET(ServiceView, opaqueState, 4);
TS3_X86_OFFSET(ServiceView, adjusted, 8);
TS3_X86_OFFSET(ServiceInterfaceView, vtable, 0);
TS3_X86_OFFSET(ServiceVtableView, lock, 0x50);
TS3_X86_OFFSET(ServiceVtableView, finalize, 0x54);

// TS3W.exe 0x00734420: conditionally finalize the adjusted interface; test only AL, reload vtable, preserve upper EAX.
uint32_t ConditionalService(ServiceView* self);
}

namespace ts3::resource::m03::delivery {
// External 0x72CB90 contract: thiscall receiver, one pointer argument, ret 4,
// result in AL (upper EAX unspecified). Only this external base is injectable.
using BaseUpdate = uint32_t (*)(ServiceInterfaceView*, void*);
// TS3W.exe 0x00734670: call base update, conditionally finalize, restore saved AL.
uint32_t UpdateAndFinalize(ServiceInterfaceView* self, void* argument, BaseUpdate base);
// TS3W.exe 0x007346A0: gate on slot 20 AL, run the no-op false base, reload slot 21 and restore update AL.
uint32_t GatedUpdateAndFinalize(ServiceInterfaceView* self, void* argument);
}

namespace ts3::resource::m03::delivery {
// UTF-16 fixed-string begin/end pairs. Borrowed prefix only; neither this
// extent nor the opaque gaps describe an EA allocation size. Native alignment
// and pointer growth intentionally change offsets on x64.
struct PathEndpoints { const char16_t* begin; const char16_t* end; };
struct EndpointOwnerView {
    unsigned char opaquePrefix[0x2ec];
    PathEndpoints primaryPath;
    unsigned char opaqueBetween[0x508-0x2f4];
    PathEndpoints secondaryPath;
};
TS3_X86_OFFSET(EndpointOwnerView, primaryPath, 0x2ec);
TS3_X86_OFFSET(EndpointOwnerView, secondaryPath, 0x508);
TS3_X86_OFFSET(PathEndpoints, begin, 0);
TS3_X86_OFFSET(PathEndpoints, end, 4);
#if defined(_M_IX86)
static_assert(offsetof(EndpointOwnerView, primaryPath)+offsetof(PathEndpoints,end)==0x2f0);
static_assert(offsetof(EndpointOwnerView, secondaryPath)+offsetof(PathEndpoints,end)==0x50c);
#endif
// TS3W.exe 0x007341B0: return primary UTF-16 begin if endpoints differ, otherwise null.
const char16_t* SelectPrimaryPath(const EndpointOwnerView* self);
// TS3W.exe 0x007341D0: return secondary UTF-16 begin if endpoints differ, otherwise null.
const char16_t* SelectSecondaryPath(const EndpointOwnerView* self);
}

namespace ts3::resource::m03::delivery {
// Canonical borrowed prefix for these two methods only. The external base owns
// interpretation of the opaque bytes; this is not an EA allocation-size claim.
struct RetainedHandleView;
struct RetainedOwnerView;
#if defined(_M_IX86)
using RetainedMethod = uint32_t (__thiscall*)(RetainedOwnerView*);
#else
using RetainedMethod = uint32_t (*)(RetainedOwnerView*);
#endif
struct RetainedVtableView {
    void* unused[20];
    RetainedMethod lock;
    RetainedMethod finalize;
};
struct RetainedOwnerView {
    const RetainedVtableView* vtable;
    unsigned char opaquePrefix[0xb0-sizeof(void*)];
    RetainedHandleView* retained;
};
TS3_X86_OFFSET(RetainedOwnerView, retained, 0xb0);
TS3_X86_OFFSET(RetainedOwnerView, vtable, 0);
TS3_X86_OFFSET(RetainedVtableView, lock, 0x50);
TS3_X86_OFFSET(RetainedVtableView, finalize, 0x54);
// Genuine external boundaries: ECX receiver, ret4 for set; ret8 for
// handle/base dispatch. Pointer arguments are borrowed, not integer addresses.
struct DispatchExternals {
    uint32_t (*set)(RetainedOwnerView*, uint32_t);
    uint32_t (*handle)(RetainedHandleView*, void*, void*);
    uint32_t (*base)(RetainedOwnerView*, void*, void*);
};
// TS3W.exe 0x00734550: set active, reload handle or gate base dispatch, finalize, clear active; preserve full saved EAX.
uint32_t DispatchRetained(RetainedOwnerView* self, void* arg1, void* arg2,
                          const DispatchExternals& external);
// External boundaries: 72AE00 / 72C6A0 take this owner in ECX and no
// stack arguments; 72DB80 takes the handle in ECX and may destroy it (not
// the owner). Callbacks expose full EAX, including otherwise unspecified bits.
// Base callbacks may change retained; release may change it but cannot own self.
using OwnerExternal = uint32_t (*)(RetainedOwnerView*);
using ReleaseExternal = uint32_t (*)(RetainedHandleView*);
// TS3W.exe 0x00734510: normalize only external 72AE00's AL, preserving upper EAX.
uint32_t NormalizeStatus(RetainedOwnerView* self, OwnerExternal activate);
// TS3W.exe 0x00734520: call base, reload retained, release before clearing, preserve last external upper EAX.
uint32_t CleanupRetained(RetainedOwnerView* self, OwnerExternal base, ReleaseExternal release);
}

namespace ts3::resource {
struct ResourceKey;
struct MemoryDBRecordInfo;
}
namespace ts3::resource::m03::delivery {
// Real argument types of the six stack arguments, taken from the canonical record read
// path (MemoryDB::Read 0x0072BD10) and from the two captured callees: 0x0072DBD0 hashes
// the key ([0]^[0xC], so it is the 16-byte ResourceKey) and 0x0072D470 writes the four
// dwords of the info out buffer. Only borrowed pointers/words cross this boundary; the
// core dereferences none of them. The genuine 0x0072D470 is called directly, not through
// the owner table (only slots 20/21 are table calls).
struct RetainedSixExternals {
    uint32_t (*set)(RetainedOwnerView*, uint32_t);
    uint32_t (*handle)(RetainedHandleView*, const ResourceKey*);
    uint32_t (*read)(RetainedOwnerView*, const ResourceKey*, void**, uint32_t, uint32_t, uint8_t,
                     MemoryDBRecordInfo*);
};
// TS3W.exe 0x007345D0: six stack arguments, ret 0x18. With outRecord==null and info==null it
// raises the retained gate (0x0072C6C0 self,1), reloads self+B0 after that callback, probes
// a present holder through 0x0072DBD0(handle,key) and returns that AL over the upper EAX of
// the always-run clear (self,0); a null holder falls through to the ordinary path AFTER the
// clear. Ordinary path: current table slot20 (0x00734710); AL==0 returns the full slot20 EAX;
// otherwise the genuine 0x0072D470(self,key,outRecord,flags,mode,flagByte,info) AL is saved,
// table slot21 (0x007347C0) is reloaded after that call and invoked when the saved AL is zero
// or outRecord is null, and the result keeps the last external call's upper EAX.
uint32_t DispatchRetainedSix(RetainedOwnerView* self, const ResourceKey* key, void** outRecord, uint32_t flags,
                             uint32_t mode, uint8_t flagByte, MemoryDBRecordInfo* info,
                             const RetainedSixExternals& external);
}

namespace ts3::resource::m03::delivery {
// The retained DatabaseIndex created by 0x00734450 and released by the 0x00734520 family.
// It is the very opaque retained-handle type of the +0xB0/+0xB8 field, so production stores
// and reloads the pointer without a cast and dereferences no byte of it.
using DatabaseIndexHandle = RetainedHandleView;
// Borrowed package stream the index is loaded from (0x0072DC60's first argument). Only the
// identity crosses this boundary; the load callback owns interpretation.
struct DatabaseIndexStream;
// The original allocator 0x004E3F90 is the game's six-argument `Alloc(size_t size, const
// char* typeName, uint32_t flags, uint32_t reserved, const char* debugFile, uint32_t
// debugLine)` as its other call sites (e.g. 0x00734860) and ts3::NamedAllocator show.
// `requestedSize` is the original literal 0x24 and is reported only: it is not native
// allocation-size proof and never sizes a host allocation.
struct DatabaseIndexAllocationRequest {
    uint32_t requestedSize;
    const char* typeName;
    uint32_t flags;
    uint32_t reserved;
    const char* debugFile;
    uint32_t debugLine;
};
// Genuine external leaves captured, never rewritten: 0x72C6C0 mutex lock/unlock (thiscall
// ret 4, full EAX), the six-argument cdecl allocator, constructor 0x72DB40 (returns the
// constructed object, not necessarily the allocation), retain 0x72D890 and load 0x72DC60
// (thiscall ret 8; only AL is the subject's result).
struct DatabaseIndexExternals {
    uint32_t (*lock)(RetainedOwnerView*, uint32_t acquire);
    DatabaseIndexHandle* (*allocate)(const DatabaseIndexAllocationRequest& request);
    DatabaseIndexHandle* (*construct)(DatabaseIndexHandle* allocation);
    uint32_t (*retain)(DatabaseIndexHandle* handle);
    uint32_t (*load)(DatabaseIndexHandle* handle, DatabaseIndexStream* stream, uint32_t mode);
};
}
namespace ts3::resource::m03::delivery {
// The shadow primary object (TS3W.exe table 0x00FFE0D0) at the prefix 0x00734450 uses. The
// +8 database interface is the very object waves 4-6 call the retained owner, so its table
// pointer is 0x00FFE078 and its `retained` field at +0xB0 is this primary's +0xB8 index
// handle; its canonical class layout (DatabasePackedStream, mutex at +0x40) is what 0x72C6C0
// locks. Only used bytes are claimed: not an EA allocation-size or complete-class view.
struct DatabaseIndexOwnerView {
    const void* primaryVtable;      // 0x00 primary identity; this subject reads no slot of it
    uint32_t opaquePrimaryState;    // 0x04
    RetainedOwnerView database;     // 0x08 the +8 database interface (same object as +0xB0 host)
    unsigned char opaqueToGate[0xE0 - 0x08 - sizeof(RetainedOwnerView)];
    uint32_t gate;                  // 0xE0 nonzero suppresses creation exactly like a handle
    unsigned char opaqueToStamp[0xF8 - 0xE4];
    uint32_t stampLow;              // 0xF8 modification time low dword (0x7343C0 checks it)
    uint32_t stampHigh;             // 0xFC modification time high dword
    uint32_t fileSize;              // 0x100 file size low dword (0x7343C0 checks it)
};
TS3_X86_OFFSET(DatabaseIndexOwnerView, primaryVtable, 0x00);
TS3_X86_OFFSET(DatabaseIndexOwnerView, opaquePrimaryState, 0x04);
TS3_X86_OFFSET(DatabaseIndexOwnerView, database, 0x08);
TS3_X86_OFFSET(DatabaseIndexOwnerView, gate, 0xE0);
TS3_X86_OFFSET(DatabaseIndexOwnerView, stampLow, 0xF8);
TS3_X86_OFFSET(DatabaseIndexOwnerView, stampHigh, 0xFC);
TS3_X86_OFFSET(DatabaseIndexOwnerView, fileSize, 0x100);
// TS3W.exe 0x00734450 (shadow-primary table 0x00FFE0D0 slot 5, 186 bytes, ret 0x18): lock the
// +8 database interface with flag 1; when both the retained index handle and the gate are
// zero, allocate 0x24 bytes through the six-argument allocator (0x24, "ResourceSystem/
// DatabaseIndex", 0,0,null,0), construct (a null allocation bypasses the constructor), store
// the constructor's RETURN into the handle field before testing it, retain it, reload the
// handle and load through 0x72DC60(handle,a1,a2); on a nonzero result AL store a6/a5/a3 into
// +0xFC/+0xF8/+0x100 (a4 is never read). Every path runs exactly one unlock, then the result
// is the unlock's EAX with its low byte replaced by the saved AL: 1 skipped, 0 failed, or the
// load's raw AL. Upper EAX always comes from the final unlock, never from an earlier call.
// EA identity kept: the raw descriptor address 0x00FFD980 is original-only and is not passed
// by this core; native has no game allocator address.
uint32_t CreateRetainedDatabaseIndex(DatabaseIndexOwnerView* self, DatabaseIndexStream* stream, uint32_t mode,
                                     uint32_t size, uint32_t unused, uint32_t stampLow, uint32_t stampHigh,
                                     const DatabaseIndexExternals& external);
}

namespace ts3::resource::m03::delivery {
// The four-dword stack temporary 0x007351E0 builds for the 0x0072C930/0x0072DC00 services
// and frees through 0x004E3F70. x86 layout is exactly the original {begin, end, reserved,
// descriptor}: the first two are pointers consumed by 0x0072DC00's range walk, and the
// original-only descriptor literal is the same "ResourceSystem/DatabaseIndex" identity the
// allocator request names. Native keeps the typed fields; no raw image address is stored.
struct RetainedTemporary {
    void* begin;          // original dword 0; freed through 0x004E3F70 when nonnull
    void* end;            // original dword 1; range end read by 0x0072DC00
    uint32_t reserved;    // original dword 2; zero at this site
    const char* typeName; // original dword 3; semantic type identity, not a raw image address
};
TS3_X86_OFFSET(RetainedTemporary, begin, 0x0);
TS3_X86_OFFSET(RetainedTemporary, end, 0x4);
TS3_X86_OFFSET(RetainedTemporary, reserved, 0x8);
TS3_X86_OFFSET(RetainedTemporary, typeName, 0xC);
TS3_X86_SIZE(RetainedTemporary, 0x10);
// Genuine external leaves captured, never rewritten: 0x72C6C0 mutex (thiscall ret 4, full
// EAX), the six-argument cdecl allocator 0x4E3F90, constructor 0x72DB40 (returns the
// constructed object, which need not equal the allocation), retain 0x72D890 (thiscall
// refcount increment; this subject discards EAX), the interface service 0x72C930 (thiscall
// ret 8; result discarded), the handle service 0x72DC00 (thiscall ret 4; result discarded),
// the interior +0x1C wide filename read 0x72C920 (thiscall, no stack arguments), the cdecl
// stat time 0x4DBCF0 (mode 2 selects the 64-bit modification time, 0 on failure), the cdecl
// FindFirstFileW size 0x4DBC60 (size low dword, -1 on failure) and the cdecl 0x4E3F70 free
// of the temporary's first member. Only borrowed pointers/words cross this boundary; the
// core dereferences none of them.
struct RetainedAcquireExternals {
    uint32_t (*lock)(RetainedOwnerView*, uint32_t acquire);
    DatabaseIndexHandle* (*allocate)(const DatabaseIndexAllocationRequest& request);
    DatabaseIndexHandle* (*construct)(DatabaseIndexHandle* allocation);
    uint32_t (*retain)(DatabaseIndexHandle* handle);
    uint32_t (*prepare)(RetainedOwnerView* self, RetainedTemporary* temporary, uint32_t mode);
    void (*loadTemporary)(DatabaseIndexHandle* handle, RetainedTemporary* temporary);
    const wchar_t* (*acquireSource)(RetainedOwnerView* self);
    uint64_t (*convertTimestamp)(const wchar_t* fileName, uint32_t mode);
    uint32_t (*convertSize)(const wchar_t* fileName);
    void (*releaseTemporary)(void* first);
};
// TS3W.exe 0x007351E0 (shadow-primary table 0x00FFE0D0 slot 7, 317 bytes, ret 0xC): lock the
// +8 database interface with 1; when its +0xB0 retained handle is null, call current table
// slot20 and, only on a nonzero AL, reload the table and call slot21; reload the retained
// handle after those callbacks. If nonnull, take it as the local result, retain it and write
// the current self+0xF8/+0xFC into the third output pair and self+0x100 plus 0 into the
// second. Otherwise, when self+0xE0 is nonzero, allocate 0x24 bytes through the six-argument
// allocator (0x24, "ResourceSystem/DatabaseIndex", 0,0,null,0); a null allocation skips the
// constructor, which itself may return null or a distinct object; only a nonnull constructor
// return becomes the local result and is retained, then the local four-dword temporary
// {0,0,0,name} is published through 0x72C930, loaded through 0x72DC00, the interior +0x1C
// wide filename is converted to the 64-bit stamp written low/high into the third pair and to
// the size written into the second pair with a zero high dword, and the temporary's first
// member is freed when nonnull. This temporary path never stores into self+0xB8. Every normal
// path runs exactly one final unlock, then stores the saved local handle into the first
// output and returns the canonical full EAX 0/1 from that handle, not the unlock's EAX.
// Metadata is never written on null-return or no-gate paths; each output region is written as
// two dwords in the original order, and a2/a3 carry size and timestamp respectively. The raw
// 0x00FFD980 descriptor identity is original-only and is not passed by this core.
uint32_t AcquireRetainedOutputs(DatabaseIndexOwnerView* self, DatabaseIndexHandle** outHandle, uint32_t* outSize,
                                uint32_t* outStamp, const RetainedAcquireExternals& external);
}

namespace ts3::resource::m03::delivery {
// The same live shadow primary object (TS3W.exe table 0x00FFE0D0) as DatabaseIndexOwnerView, but
// this method only reads these bytes, so the view claims only them: the borrowed +0xC0 critical-
// section storage is never interpreted (the enter/leave seams own the lock), +0xE0 is the
// refresh gate, +0xE8/+0xEC and +0xF0/+0xF4 are the cached 64-bit window and +0x104 the exact
// full-dword state. The x86 offsets are asserted; native alignment intentionally changes them
// and none of this is an EA allocation-size or complete-class claim.
struct DeadlineRefreshView {
    unsigned char opaqueToCriticalSection[0xC0];
    unsigned char criticalSection[0x18]; // x86 CRITICAL_SECTION storage, borrowed and opaque
    unsigned char opaqueToGate[0xE0 - 0xC0 - 0x18];
    uint32_t gate;          // 0xE0: exact full-dword 1 test before and after the enter
    unsigned char opaqueToBase[0xE8 - 0xE4];
    uint32_t baseLow;       // 0xE8
    uint32_t baseHigh;      // 0xEC
    uint32_t deadlineLow;   // 0xF0
    uint32_t deadlineHigh;  // 0xF4
    unsigned char opaqueToState[0x104 - 0xF8];
    uint32_t state;         // 0x104: exact full-dword 1 test
};
TS3_X86_OFFSET(DeadlineRefreshView, criticalSection, 0xC0);
TS3_X86_OFFSET(DeadlineRefreshView, gate, 0xE0);
TS3_X86_OFFSET(DeadlineRefreshView, baseLow, 0xE8);
TS3_X86_OFFSET(DeadlineRefreshView, baseHigh, 0xEC);
TS3_X86_OFFSET(DeadlineRefreshView, deadlineLow, 0xF0);
TS3_X86_OFFSET(DeadlineRefreshView, deadlineHigh, 0xF4);
TS3_X86_OFFSET(DeadlineRefreshView, state, 0x104);
// Genuine external leaves captured, never rewritten: the +0xC0 critical-section enter/leave pair
// (IAT 0xF9512C / 0xF95130) and QueryPerformanceCounter (IAT 0xF953B8) each take one borrowed
// pointer; 0x735320 is thiscall with no stack arguments and its EAX is discarded. The core
// dereferences none of them and never constructs a real lock. The 0xF953B8 adapter writes the
// 64-bit count through the pointer into the caller's owned local.
struct DeadlineRefreshExternals {
    void (*enterCriticalSection)(void* criticalSection);
    void (*leaveCriticalSection)(void* criticalSection);
    void (*queryPerformanceCounter)(uint64_t* count);
    uint32_t (*refresh)(DeadlineRefreshView* self);
};
// TS3W.exe 0x007355A0 (shadow-primary table 0x00FFE0D0 slot 6, 165 bytes, ret 4): a full-dword
// self+0x104 == 1 gate returns AL 0 with no calls; a full-dword self+0xE0 != 1 returns AL 1
// with no lock. Otherwise enter self+0xC0, reload self+0xE0 and, when it changed, leave and
// return AL 1 without refreshing. If it is still 1 and the LOW BYTE of the argument is
// nonzero, refresh directly. With argument byte 0, QueryPerformanceCounter fills an owned local,
// the current self+0xE8/+0xEC is subtracted with modulo-64 SUB/SBB and the elapsed high word is
// compared UNSIGNED to the current self+0xF4: greater refreshes, less leaves and returns AL 0;
// equal compares the low word unsigned to self+0xF0, where equality refreshes. Refresh calls
// 0x735320(self) and ignores its AL; every entered path runs exactly one leave. Only AL is
// contractual; the upper EAX bits are caller/external residue and are never normalized (the
// early state gate clears only AL and keeps the caller's upper EAX).
uint8_t RefreshOnForceOrDeadline(DeadlineRefreshView* self, uint32_t force,
                                 const DeadlineRefreshExternals& external);
}

namespace ts3::resource::m03::delivery {
// USED receiver prefix of 0x007354F0 only (corpus name DatabaseShadowWatchersSignaled_3,
// primary table 0x00FFE2F0 slot 12). The method reads only the embedded secondary base at
// +8: the constructor 0x00736DB0 installs table 0x00FFE250 (DatabaseShadowWatchersSignaled_2)
// into that word. Its byte offset 0x64 slot is the genuine collector 0x004B1300. Bytes the
// body does not read (the primary table, +4) stay uninterpreted: no complete-class or
// allocation-size claim.
struct SignalEmbeddedBase {
    const void* vtable; // +0 of the embedded base: table 0x00FFE250 (owned by the receiver)
};
struct SignalReceiverView {
    void* primaryVtable;         // +0: this method never reads the primary table
    void* unused04;              // +4: never read
    SignalEmbeddedBase embedded; // +8 on x86: first byte of the embedded Signaled_2 base
};
TS3_X86_OFFSET(SignalReceiverView, embedded, 8);
// The circular collector list head 0x007354F0 builds on its stack and hands to the
// embedded base's slot 25. Genuine 0x004B1300 appends 12-byte nodes {next, prev, observer}
// at the tail using collector->typeName as the allocator's type identity; the same
// three-field head is modelled by cls_4a3a40_x's NodeListHead. Node+8 is the observer.
struct WatcherNode {
    WatcherNode* next;  // +0
    WatcherNode* prev;  // +4
    void* observer;     // +8: notified through its table's byte offset 0x28 slot
};
struct WatcherCollector {
    void* next;           // +0: the sentinel itself when empty
    void* prev;           // +4
    const char* typeName; // +8: "ResourceSystem/FRCacheList" (original image address 0x00FFE1C0)
};
TS3_X86_SIZE(WatcherNode, 0xC);
TS3_X86_SIZE(WatcherCollector, 0xC);

struct KeySignalMessage;
// The identity word the original stores at message+0 is the cls_532c30_x key-record
// vtable 0x00FFE054, whose used slot +4 is 0x007341F0 KeyRecordEquals (rewritten by
// m03_delivery_accounting). Portable production stores this descriptor with the same used
// method; slot 0 there is the scalar deleting dtor, never invoked on a stack message.
#if defined(_M_IX86)
using KeySignalEquals = uint32_t (__fastcall*)(const KeySignalMessage*, void*, const ResourceKey*);
#else
using KeySignalEquals = uint32_t (*)(const KeySignalMessage*, const ResourceKey*);
#endif
struct KeySignalVtable {
    void* unused00;         // +0: EA's 0x009D6C70 scalar deleting dtor, unused here
    KeySignalEquals equals; // +4: the used method (KeyRecordEquals contract)
};
TS3_X86_OFFSET(KeySignalVtable, equals, 4);
// TS3W.exe 0x007354F0 store target; null identity object never dereferenced by this subject.
const KeySignalVtable* KeySignalIdentity();
// The local key signal copied to owned backing BEFORE the collection callback. Placement
// established by the accounting producer/consumer pair (identity +0, unwritten gap +4, the
// four ResourceKey words +8..+0x14) and independently produced by this body, which stores
// a0[0]/a0[4]/a0[8]/a0[0xC] into +0x14..+0x20 of its frame. +4 is never written by the
// original and never read by the used method, so production leaves it uninitialized too.
struct KeySignalMessage {
    const void* identity; // +0: 0x00FFE054 on the original, KeySignalIdentity() in production
    uint32_t unused04;    // +4: producer-unwritten gap
    ResourceKey key;      // +8..+0x14: instanceLow/instanceHigh/type/group
};
TS3_X86_OFFSET(KeySignalMessage, identity, 0);
TS3_X86_OFFSET(KeySignalMessage, unused04, 4);
TS3_X86_OFFSET(KeySignalMessage, key, 8);
TS3_X86_SIZE(KeySignalMessage, 0x18);
// Genuine external seams captured, never rewritten: 0x004B1300 is slot 25 (byte offset
// 0x64) of the embedded base table 0x00FFE250 (append nodes, full EAX discarded here); the
// per-node observer's slot 10 (byte offset 0x28) receives the owned message pointer and its
// result is discarded; 0x004E3F70 is the cdecl node release. Only borrowed pointers cross
// this boundary; the core dereferences none of the callbacks' storage beyond the shared
// WatcherNode/WatcherCollector links it built itself.
struct KeySignalExternals {
    void (*collect)(SignalEmbeddedBase* embedded, WatcherCollector* collector);
    void (*notify)(void* observer, KeySignalMessage* message);
    void (*release)(void* node);
};
// TS3W.exe 0x007354F0 (primary table 0x00FFE2F0 slot 12, 161 bytes, ret 4): copy all four
// key words to owned stack backing BEFORE any external call; build the local sentinel
// collector {"ResourceSystem/FRCacheList"} and the local message {key-record identity,
// gap, copied key}; call the embedded +8 base's collection slot with the collector; notify
// every node's +8 observer with the SAME owned message, reloading each node's next link
// AFTER that notification; then free every node starting from a freshly reloaded sentinel
// next link, reading next BEFORE each free. An empty list skips both loops. The original's
// EAX is incidental residue (the local sentinel address when nothing was freed, otherwise
// the last free's result); this entry point is void like the other destructor-style methods
// and the caller never consumes it.
void SignalKeyWatchers(SignalReceiverView* self, const ResourceKey* key,
                       const KeySignalExternals& external);
}
