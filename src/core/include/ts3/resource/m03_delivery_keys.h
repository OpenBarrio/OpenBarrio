#pragma once
#include "ts3/layout.h"
#include "ts3/resource/compressed_index.h"
#include "ts3/scratch/clu_408e20_Scratch.h"
#include "ts3/thread/mutex.h"

namespace ts3::resource {

// Preconditions: [first,last) is one contiguous ResourceKey array, sorted by unsigned
// instanceLow, instanceHigh, group, type (offsets 0,4,C,8). Equal keys are allowed.
// The empty range may be null. No range/key/owner validation is performed by EA.
// TS3W.exe 0x00736870: lower-bound membership, returning full EAX 0 or 1.
uint32_t DeliveryKeysContains(const ResourceKey* first, const ResourceKey* last,
                              const ResourceKey* key);

// Borrowed USED prefix only, not an allocation-size claim for the original class.
struct DeliveryKeySpan {
    const void* identity; // original vtable/opaque word, never read here
    const ResourceKey* first;
    const ResourceKey* last;

    // TS3W.exe 0x007368D0: ECX owner, one stack key argument (ret 4), full EAX membership.
    uint32_t Contains(const ResourceKey* key) const;
};
TS3_X86_OFFSET(DeliveryKeySpan, identity, 0);
TS3_X86_OFFSET(DeliveryKeySpan, first, 4);
TS3_X86_OFFSET(DeliveryKeySpan, last, 8);

// Actual producer 7368F0 constructs these five words (not just the borrowed span).
// This is a bounded DatabaseKeyFilter storage representation, not certification of
// a complete C++ class hierarchy or of unknown tables adjacent to FB2CBC.
struct DeliveryKeyFilter {
    DeliveryKeySpan span;
    const ResourceKey* capacity;
    const char* allocationName;
};
TS3_X86_OFFSET(DeliveryKeyFilter, capacity, 12);
TS3_X86_OFFSET(DeliveryKeyFilter, allocationName, 16);
TS3_X86_SIZE(DeliveryKeyFilter, 20);

// FB2CBC has one proven callable slot, 9D6C70 (base scalar deletion).
// The fastcall view supplies dummy EDX for the actual x86 thiscall table slot.
struct DeliveryKeyBaseTable {
    DeliveryKeySpan* (__fastcall* scalarDelete)(DeliveryKeySpan*, void*, uint32_t);
};
const DeliveryKeyBaseTable* DeliveryKeyBaseIdentity();
// TS3W.exe 0x00736840: free buffer, reset base identity, optionally free self; return receiver address even after release (flags bit zero only).
DeliveryKeySpan* DeliveryKeyScalarDelete(DeliveryKeySpan* self, uint32_t flags);
static_assert(offsetof(ResourceKey, instanceLow) == 0);
static_assert(offsetof(ResourceKey, instanceHigh) == 4);
static_assert(offsetof(ResourceKey, type) == 8);
static_assert(offsetof(ResourceKey, group) == 12);

// Complete array element, not a borrowed prefix of the owning vector/class.
struct DeliveryStringRecord {
    scratch::WideString text;
    uint32_t value;
};
TS3_X86_OFFSET(DeliveryStringRecord, text, 0);
TS3_X86_OFFSET(DeliveryStringRecord, value, 0x10);
TS3_X86_SIZE(DeliveryStringRecord, 20);

// Valid constructed strings and a backward-copy-compatible destination are required.
// Assignment uses the canonical string implementation (including its external growth boundary).
// TS3W.exe 0x00735AF0: cdecl reverse string/value assignment, returns destination begin; allocator/name word +C is untouched.
DeliveryStringRecord* DeliveryRecordsCopyBackward(const DeliveryStringRecord* first,
    const DeliveryStringRecord* last, DeliveryStringRecord* outEnd);

// Optional external heap binding. Only byte allocation/free cross this boundary;
// record traversal, strings, lifetime ordering and publication remain in the core.
// Null binding uses the canonical NamedAllocator (EA new[] / delete[] on x86).
struct DeliveryStorage {
    void* (*allocate)(size_t bytes, const char* name, unsigned flags,
                      unsigned extra, const char* file, unsigned line);
    void (*release)(void* block);
};
DeliveryStorage SetDeliveryStorage(DeliveryStorage storage);

// Borrowed USED vector fields, not a complete owner allocation-size claim.
struct DeliveryRecordVector {
    DeliveryStringRecord* begin;
    DeliveryStringRecord* end;
    DeliveryStringRecord* capacity;
    const char* allocationName;
    // Spare insertion requires begin <= position < end; callers append directly
    // when spare storage exists. Full insertion permits [begin,end], including empty.
    // TS3W.exe 0x00737800: ECX vector, position/source stack pointers, ret8; returns inserted value when spare, new capacity count when full.
    uint32_t Insert(DeliveryStringRecord* position, const DeliveryStringRecord* source);
};
TS3_X86_OFFSET(DeliveryRecordVector, begin, 0);
TS3_X86_OFFSET(DeliveryRecordVector, end, 4);
TS3_X86_OFFSET(DeliveryRecordVector, capacity, 8);
TS3_X86_OFFSET(DeliveryRecordVector, allocationName, 12);

} // namespace ts3::resource

namespace ts3::misc { struct ResourceKeyVector; }
namespace ts3::resource {
// Opaque borrowed resource-service identities. Neither adapter reads a receiver
// field: these are NOT DeliveryKeySpan, nor claims about full class allocation.
struct DeliveryResourceService;
struct DeliveryKeyLookup;
// Only external operations are bindable, never the adapters or key constructor.
// Null entries use canonical bridges on x86; native requires actual host leaves.
struct DeliveryAdapterLeaves {
    DeliveryKeyLookup* (*manager)() = nullptr; // 4AFD20
    uint32_t (*hasKey)(DeliveryKeyLookup*, const ResourceKey*) = nullptr; // slot17
    uint32_t (*request)(DeliveryResourceService*, const ResourceKey*, void**,
        uint32_t, void*, void*, const ResourceKey*) = nullptr; // 4B0810
    uint32_t (*operation)(DeliveryResourceService*, misc::ResourceKeyVector*,
        const void*, uint32_t) = nullptr; // 4B1AE0
    uint32_t (*cleanup)(misc::ResourceKeyVector*) = nullptr; // 4AFCD0
};
DeliveryAdapterLeaves SetDeliveryAdapterLeaves(DeliveryAdapterLeaves leaves);
// TS3W.exe 0x00736610: ECX service, six stack args/ret24; construct a local 16-byte key and forward five remaining args; preserve full EAX.
uint32_t DeliveryRequest(DeliveryResourceService*, const ResourceKey*, void**,
    uint32_t, void*, void*, const ResourceKey*);
// TS3W.exe 0x00736660: ECX service, three stack args/ret12; nonzero full result and nonnull vector invoke sort/unique; return cleanup EAX when called.
uint32_t DeliveryOperate(DeliveryResourceService*, misc::ResourceKeyVector*,
    const void*, uint32_t);
} // namespace ts3::resource

namespace ts3::resource {
// Borrowed USED hash-storage prefix, not DeliveryKeySpan and not an original
// allocation-size claim. The bucket pointer widens on native hosts.
struct DeliveryCleanupStorage {
    void* identity;
    void* buckets;
    uint32_t bucketCount;
    uint32_t size;
};
TS3_X86_OFFSET(DeliveryCleanupStorage, identity, 0);
TS3_X86_OFFSET(DeliveryCleanupStorage, buckets, 4);
TS3_X86_OFFSET(DeliveryCleanupStorage, bucketCount, 8);
TS3_X86_OFFSET(DeliveryCleanupStorage, size, 12);
// Only the external node-destruction operation is bindable. Null uses the
// original bridge on x86; native hosts must supply this unrewritten dependency.
using DeliveryDestroyNodes = void (*)(DeliveryCleanupStorage*, void*, uint32_t);
DeliveryDestroyNodes SetDeliveryDestroyNodes(DeliveryDestroyNodes binding);
// TS3W.exe 0x00735BC0: ECX receiver/plain ret; destroy nodes, zero size, free reloaded buckets if reloaded unsigned count >1; leave pointer/count intact.
// Destructor-style void API: EAX is incidental helper/free residue, not a self
// identity or a defined scalar result (some original callers forward residue).
void DeliveryCleanup(DeliveryCleanupStorage* self);
} // namespace ts3::resource

namespace ts3::resource {
// Borrowed USED view of the DatabaseShadowWatchersSignaled primary subobject (the
// ResourceSystem's embedded _tail at +0x18; TS3W.exe primary table 0x00FFE2F0, 28
// slots). Only the cells these two bound methods actually touch are declared: the
// primary vtable word at +0 and the +0x754 anchor pointer. This is NOT a complete
// allocation-size claim. The anchor cell is a pointer (its own callers dereference
// +0/+4/+8 and pass it as a receiver to 0x7356D0), never a uint32 token.
struct DeliveryShadowOwner {
    void* primaryVtable; // +0 EA primary vtable word; native code never dereferences it
    unsigned char unusedPrefix[0x754 - sizeof(void*)]; // untouched bytes up to the anchor
    void* anchor; // +0x754 pointer-sized linked-list anchor cell
};
TS3_X86_OFFSET(DeliveryShadowOwner, primaryVtable, 0);
TS3_X86_OFFSET(DeliveryShadowOwner, anchor, 0x754);

// Actual external slot-2 implementation boundary only (for this receiver the real
// callee is 734B50: int __thiscall (this, const wchar_t* source, int zero), ret 8).
// The 733E10 wrapper itself must never be bound here. Null uses the real x86 vtable
// dispatch; a native host must supply this typed callable.
using DeliveryShadowSlot2 = uint32_t (*)(DeliveryShadowOwner*, const void*, uint32_t);
DeliveryShadowSlot2 SetDeliveryShadowSlot2(DeliveryShadowSlot2 leaf);
// TS3W.exe 0x00733E10: ECX owner, one pointer stack argument, ret4. Loads [[owner]+8]
// (slot 2) and calls it with the unchanged receiver, the original argument and a
// literal zero; returns the callee's full EAX. No booleanization, discarded result,
// swapped arguments or guessed width.
uint32_t DeliveryShadowDispatch(DeliveryShadowOwner* owner, const void* source);
// TS3W.exe 0x00733E30: ECX owner, no stack arguments, plain ret. Returns the +0x754
// cell unchanged; EA adds no null/zero/range check and no masking.
void* DeliveryShadowAnchor(const DeliveryShadowOwner* owner);
} // namespace ts3::resource

namespace ts3::resource {
// Complete element of the +0x724 path-record vector; TS3W.exe tags this vector's
// element type as "ResourceSystem/DDFPathInfo". The path is a complete constructed
// WideString; the +0x10 cell is the record's associated identity. The original body
// loads it with a 32-bit the original instruction sequence; TS3W.exe has no other reader of this
// vector's cell, and the same cell holds an object pointer in the sibling
// DatabaseShadowWatchersSignaled instantiation (0x00738670 stores a
// DirectoryDefinition* there). It is therefore an object identity, not a
// body-width integer: native hosts widen it to a full pointer instead of
// truncating, and the array stride follows this real record layout.
struct DeliveryPathRecord {
    scratch::WideString text; // +0x00 complete constructed path string
    void* payload;            // +0x10 associated identity (4-byte EA cell)
};
TS3_X86_OFFSET(DeliveryPathRecord, text, 0);
TS3_X86_OFFSET(DeliveryPathRecord, payload, 0x10);
TS3_X86_SIZE(DeliveryPathRecord, 20);

// Borrowed USED prefix only: the vector cells these bodies read on the same
// embedded receiver as DeliveryShadowOwner (the +0x754 anchor lies behind them).
// Not a complete allocation-size claim for the original class. 0x00737BA0's append
// reads the insertion position from +0x728, compares it UNSIGNED against +0x72C and
// (when full) hands the whole vector at +0x724 to 0x00737800, whose growth path reads
// the vector word +0xC as the allocation tag -> the pointer-sized +0x730 cell.
struct DeliveryPathOwner {
    unsigned char unusedPrefix[0x724];
    DeliveryPathRecord* begin;    // +0x724
    DeliveryPathRecord* end;      // +0x728
    DeliveryPathRecord* capacity; // +0x72C (unsigned full test, never a count)
    const char* allocationName;   // +0x730 tag read by 0x00737800's growth path
};
TS3_X86_OFFSET(DeliveryPathOwner, begin, 0x724);
TS3_X86_OFFSET(DeliveryPathOwner, end, 0x728);
TS3_X86_OFFSET(DeliveryPathOwner, capacity, 0x72C);
TS3_X86_OFFSET(DeliveryPathOwner, allocationName, 0x730);

// Genuine external helper boundary: 0x004ED480, whose canonical production form is
// ts3::resourcesystem::HasPathPrefix(prefix, path, mode). The original subject
// always passes mode 5 and tests only AL of the returned word. A native host may
// install this typed binding because the canonical helper's mode-5 FoldByte path is
// not native-executable (its game folding table is NotRewritten). No traversal or
// match policy crosses this boundary.
using DeliveryPathPrefix = uint32_t (*)(const char16_t* prefix, const char16_t* path, int mode);
DeliveryPathPrefix SetDeliveryPathPrefix(DeliveryPathPrefix binding);
// TS3W.exe 0x00734160: ECX owner, one path argument, ret4. Snapshots begin/end
// BEFORE any external call; an empty snapshot returns full EAX 0 without reading the
// path. Per record calls 004ED480(record text, original argument, 5) (caller pops
// 12), tests AL ONLY, and on success returns the record's +0x10 cell reloaded AFTER
// the helper. Otherwise advances one 20-byte record and compares against the
// SNAPSHOTTED end, returning 0 on exhaustion. No sorting, longest-prefix search,
// skipped record, reloaded end, extra field write or null repair.
void* DeliveryPathFirstValue(DeliveryPathOwner* owner, const char16_t* path);

// Typed external seams for the unrewritten string helpers 0x00737BA0 crosses. The
// defaults below are the genuine host implementations (real image code on x86,
// native-executable equivalents of the proved 0040B700/00408E20/00408EB0 contracts
// on x64); they are never an image bridge, a no-op or a NotRewritten fallback. A
// host may bind its own implementation through SetDeliveryPathAppendLeaves; the
// subject's own sequencing, lifetimes, end publication, reload and saved index stay
// in the core and no whole-append provider crosses this boundary.
struct DeliveryPathAppendLeaves {
    // 0x0040B700 contract: assign the NUL-terminated source into the 16-byte string,
    // scanning for the terminator and growing through 0x00408830 (exactly
    // 2*(length+1) bytes with the destination's existing +0xC tag, sentinel when
    // length is zero, terminator written at the source length).
    void (*constructText)(scratch::WideString* out, const char16_t* source) = nullptr;
    // 0x00408E20 contract: assign [first,last) into the string. The length compare is
    // UNSIGNED (an inverted range takes the growth path); a longer source grows
    // through 0x00408B90 with the destination's own +0xC tag (minimum eight
    // characters), a shorter one memmoves and moves the terminator, shrinking `end`.
    void (*assignRange)(scratch::WideString* out, const char16_t* first,
                        const char16_t* last) = nullptr;
    // 0x00408EB0 contract: copy-construct the record's string only (exactly
    // 2*(length+1) bytes from the source +0xC tag, sentinel for an empty source). The
    // +0x10 identity cell is NOT part of this contract; the subject writes it after
    // the call.
    void (*constructRecordText)(DeliveryPathRecord* out,
                                const DeliveryPathRecord* source) = nullptr;
};
DeliveryPathAppendLeaves DefaultDeliveryPathAppendLeaves();
DeliveryPathAppendLeaves SetDeliveryPathAppendLeaves(DeliveryPathAppendLeaves leaves);

// TS3W.exe 0x00737BA0: ECX owner, one NUL-terminated path argument, ret 4.
// Constructs a temporary string from the argument (40B700), assigns its range into a
// local ResourceSystem/PathDdfInfo record initialized with the empty sentinel
// (begin=end=011C7E54, cap=011C7E56, tag 00FFE0F8, payload 0) through 408E20, frees
// the temporary before appending, then appends the local record at the owner's +0x724
// vector end: when the UNSIGNED comparison position < capacity holds it publishes
// end=position+20 BEFORE constructing, skips construction for a null position, and
// copies the local +0x10 cell into the record +0x10 AFTER the constructor; otherwise
// it calls the real 0x00737800 on the vector. After either branch it RELOADS the
// current begin/end, computes the signed byte extent divided by the native record
// stride and subtracts one, saves that index, frees the local record's buffer under
// the same masked-extent/non-null test and returns the SAVED index (never the release
// residue). Local temporaries are ordinary owned strings here; no lock, retain, path
// normalization or deduplication is added.
uint32_t DeliveryPathAppendRecord(DeliveryPathOwner* owner, const char16_t* path);
} // namespace ts3::resource

namespace ts3::resource {
// Complete element of the +0x2A8 hash-candidate array: the record's full 32-bit
// path hash and the associated object identity. TS3W.exe stores both cells as
// dwords and compares the hash with a 32-bit `cmp`; on native hosts the object
// cell widens to a full pointer, so the stride is the host layout, never a
// hard-coded 8 (that would walk every other element once pointers widen).
struct DeliveryHashRecord {
    uint32_t hash;  // +0x00 full 32-bit FNV-1a hash of the normalized path
    void* object;   // +0x04 associated object identity (4-byte EA cell)
};
TS3_X86_OFFSET(DeliveryHashRecord, hash, 0);
TS3_X86_OFFSET(DeliveryHashRecord, object, 4);
TS3_X86_SIZE(DeliveryHashRecord, 8);

// Borrowed USED prefix of the DatabaseShadowWatchersSignaled receiver (IDA class
// DatabaseShadowWatchersSignaled_3, size 2060, primary table 0x00FFE2F0). Only the
// cells this body actually touches are declared: the recursive mutex at +0x50
// (inside IDA's unnamed +0x04..+0x1D8 block) and the +0x2A8/+0x2AC begin/end of the
// {hash, object} candidate array (IDA f_2A8/f_2AC). Every other byte is padding
// here. This is NOT a complete allocation-size claim and NOT DeliveryPathOwner
// (whose own +0x724/+0x728 cells lie far behind this prefix). Both real callers
// (vtable slot 2 0x00734B50 and 0x00737950) pass their own receiver in ECX.
struct DeliveryHashOwner {
    unsigned char unusedPrefix[0x50];
    thread::Mutex mutex; // +0x50 recursive mutex, locked for the whole scan
    unsigned char unusedMiddle[0x2A8 - 0x50 - sizeof(thread::Mutex)];
    DeliveryHashRecord* begin; // +0x2A8 candidate array begin
    DeliveryHashRecord* end;   // +0x2AC candidate array end (snapshotted)
};
TS3_X86_OFFSET(DeliveryHashOwner, mutex, 0x50);
TS3_X86_OFFSET(DeliveryHashOwner, begin, 0x2A8);
TS3_X86_OFFSET(DeliveryHashOwner, end, 0x2AC);
TS3_X86_SIZE(DeliveryHashOwner, 0x2B0);

// Only the genuine external operations this body crosses are bindable; the
// normalization choice, hash sequencing, the lock-bound snapshot, the full-hash
// filter, the first-match loop, the post-compare reload and the pre-unlock saved
// result stay in the core. Null entries use the canonical/real x86 path (or throw
// on native when no native-executable implementation exists).
struct DeliveryHashLeaves {
    // 0x004ED2A0 ResolvePath(out, path, base, kind): resolves into a 260-unit
    // buffer. The subject discards the returned pointer and hashes the buffer as
    // the helper left it (a failed resolve is NOT repaired).
    char16_t* (*resolvePath)(char16_t* out, const char16_t* path, const char16_t* base,
                             int32_t kind) = nullptr;
    // 0x004EC3C0 literal mode check; only AL of the returned word is observed.
    uint32_t (*isTextMode)(int32_t mode) = nullptr;
    // 0x004FA640 hash(text, seed, mode); the subject always passes seed
    // 0x811C9DC5
    uint32_t (*hashText)(const char16_t* text, uint32_t seed, int32_t mode) = nullptr;
    // 0x004E16F0 int __thiscall Mutex::Lock(mutex, &timeout) ret 4; the timeout
    // word the subject passes is -1 (kTimeoutNone).
    int32_t (*lockMutex)(thread::Mutex* mutex, const uint32_t* timeout) = nullptr;
    // 0x004E17B0 int __thiscall Mutex::Unlock(mutex). Its result is discarded.
    int32_t (*unlockMutex)(thread::Mutex* mutex) = nullptr;
    // 0x004ED300 CompareResourcePaths(first, second, mode): first is the ORIGINAL
    // argument, second the record's slot-10 path, mode the literal 5.
    int32_t (*comparePaths)(const char16_t* first, const char16_t* second, int32_t mode) = nullptr;
    // Virtual slot 10 (vtable +0x28) of the record's object, returning its path.
    const char16_t* (*recordPath)(void* object) = nullptr;
};
DeliveryHashLeaves SetDeliveryHashLeaves(DeliveryHashLeaves leaves);
// TS3W.exe 0x00734220: ECX receiver, one path argument, ret 4. Resolves the
// argument into a 260-unit local buffer, lowercases it when the literal mode-5
// check yields AL 0, computes the full 32-bit FNV-1a hash (seed 0x811C9DC5, mode
// 0), locks the +0x50 mutex, snapshots +0x2A8/+0x2AC AFTER the lock, scans the
// {hash, object} records, and for the first full-hash match whose slot-10 path
// compares equal (full EAX 0) returns the record's object reloaded AFTER the
// compare and saved BEFORE the unlock. Exhaustion unlocks and returns full 0 (the
// unlock result is never returned). Hash mismatches cross no virtual/compare call.
// The resolve result is discarded (EA hashes whatever the helper left, including a
// failed resolve), and a dirty upper EAX/AL never changes either test.
void* DeliveryHashPathLookup(DeliveryHashOwner* owner, const char16_t* path);
} // namespace ts3::resource
