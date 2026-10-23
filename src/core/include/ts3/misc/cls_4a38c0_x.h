// cls_4a38c0_x - portable rewrite of the TS3W.exe ResourceMan / DDFRecord / ResourceBase
// family spread over 0x004A38C0..0x004A662B plus 0x0072AE00 and 0x00739D90 (misc/, 26 entry
// points, 2085 bytes).  The ResourceMan part shares the verified layout of the services unit
// (`ts3/services/cls_4a4ba0_Services.h`); calls into other units go through the bridge in
// ts3/layout.h.
#pragma once

#include <cstdint>

#include "ts3/layout.h"
#include "ts3/services/cls_4a4ba0_Services.h"

namespace ts3 {
namespace misc {

// ---------------------------------------------------------------------------
// TS3W.exe 0x00FB84D0 / 0x00FB21D8: bare base object this unit's deleting destructor
// (0x004A3D30) resets. Only the two vtable words are ever touched.
// ---------------------------------------------------------------------------
struct ResourceBase {
    void* const* vtable;           // +0x00
    void* const* secondaryVtable;  // +0x04
};
TS3_X86_SIZE(ResourceBase, 8);

// ---------------------------------------------------------------------------
// TS3W.exe 0x004A42C0 allocates 584 bytes for this object. Primary vtable 0x00FB22C0,
// FileObject subobject at +0x08 with vtable 0x00FB2270.
// ---------------------------------------------------------------------------
struct DDFRecord {
    void* const* vtable;         // +0x000 (vt_DDFRecord)
    void* const* baseVtable;     // +0x004 (base interface)
    void* const* fileObject;     // +0x008 (vt_FileObject subobject)
    uint8_t pad0C[0x224];        // +0x00C
    uint32_t key[4];             // +0x230 (DDFRecord__SetKey)
    void* index;                 // +0x240
    uint8_t flag244;             // +0x244
    uint8_t pad245[3];           // +0x245
};
TS3_X86_SIZE(DDFRecord, 0x248);
TS3_X86_OFFSET(DDFRecord, key, 0x230);
TS3_X86_OFFSET(DDFRecord, index, 0x240);
TS3_X86_OFFSET(DDFRecord, flag244, 0x244);

// ---------------------------------------------------------------------------
// Root of the DDFRecord-key tree. `payload` is ServicePayload in the shared layout; the list
// of record objects hangs off payload.links (node+0x38).
// ---------------------------------------------------------------------------
struct RecordListLink {
    RecordListLink* next;      // +0
    RecordListLink* previous;  // +4
    DDFRecord* value;          // +8
};
TS3_X86_SIZE(RecordListLink, 12);

// ---------------------------------------------------------------------------
// ResourceMan entry points. `this` conventions come from the vtables:
// 0x00FB2410 (secondary, +0x04) and 0x00FB2420 (primary).
// ---------------------------------------------------------------------------

// TS3W.exe 0x004A38C0: returns `this` for interface id 0xEE3F516E (null when the adjusted
// pointer is 4) and null otherwise; `this` is the base subobject at object+4. EA quirk: the
// check is `this - 4 == 0`, the `and` then keeps the whole original pointer.
void* ResourceBaseQueryInterface(ResourceBase* self, uint32_t id);

// TS3W.exe 0x004A38E0: DDFRecord accessor, returns the address of the +0x230 key block.
void* DDFRecordKeyAddress(DDFRecord* self);

// TS3W.exe 0x004A38F0: copies four key dwords from `key` and returns the last one copied.
int32_t DDFRecordSetKey(DDFRecord* self, const uint32_t* key);

// TS3W.exe 0x004A3920: returns the +0x240 object pointer.
void* DDFRecordIndex(DDFRecord* self);

// TS3W.exe 0x004A3930: when the +0x244 flag is set, tail-calls the index object's vtable
// slot 15 with `this` and returns its full EAX; otherwise returns AL=1 (the upper bits of
// EAX are the caller's on that path).
uint32_t DDFRecordReset(DDFRecord* self);

// TS3W.exe 0x004A3950: clears `active` and calls the primary vtable slot 7 when it was set;
// the virtual call's result is discarded and AL=1 is always returned.
uint8_t ResourceManDeactivate(services::ResourceMan* self);

// TS3W.exe 0x004A3970: base queryInterface on the secondary subobject (object+4); id
// 0x0226A1DE returns object, 0xEE3F516E returns the secondary pointer (null when it is 4).
void* ResourceManQueryInterface(services::ResourceMan* self, uint32_t id);

// TS3W.exe 0x004A39A0: returns the constant class id 0x34728492.
uint32_t ResourceManClassId(services::ResourceMan* self);

// TS3W.exe 0x004A39B0: locks (arg != 0, timeout kTimeoutNone) or unlocks the mutex at +0x80
// and returns the mutex call's value.
int32_t ResourceManLockMutex(services::ResourceMan* self, uint32_t lock);

// TS3W.exe 0x004A3A90: deleting-destructor thunk for the secondary subobject (object+4);
// adjusts to the object and forwards to ResourceBaseDestroy.
ResourceBase* ResourceBaseDestroySecondary(ResourceBase* self, uint32_t flags);

// TS3W.exe 0x004A3AA0: interlocked release on the secondary subobject (object+4): the
// reference count is at object+8; at zero it pins the count to 1 and calls the primary
// vtable slot 0 with flag 1 (only when the object pointer is non-null).
int32_t ResourceManRelease(services::ResourceMan* self);

// TS3W.exe 0x004A3AD0: file open/mode machine. Mode 6 is remapped from the flag's low bits;
// modes 1/2/3/4/5 run the Win32 file helpers on the path; only the flag's low byte is tested.
uint8_t ResourceManOpen(services::ResourceMan* self, uint32_t flags, int32_t mode, uint32_t unused);

// TS3W.exe 0x004A3D30: deleting destructor of the base object: resets the two vtable words
// (the +4 store happens twice, the first one is dead) and frees the object when bit 0 of
// `flags` is set. Returns the object pointer.
ResourceBase* ResourceBaseDestroy(ResourceBase* self, uint32_t flags);

// TS3W.exe 0x004A3D60: deleting-destructor thunk for the FileObject subobject (object+8).
DDFRecord* DDFRecordDestroySecondary(DDFRecord* self, uint32_t flags);

// TS3W.exe 0x004A3D70: thunk for the FileObject subobject (object+8) forwarding to
// DDFRecordReset on the object.
uint32_t DDFRecordResetSecondary(DDFRecord* self);

// TS3W.exe 0x004A4330: deleting destructor of a DDFRecord: sets both vtables, destroys the
// FileObject subobject through 0x004DB9D0, installs the base vtable and frees when bit 0 of
// `flags` is set. Returns the object pointer.
DDFRecord* DDFRecordDestroy(DDFRecord* self, uint32_t flags);

// TS3W.exe 0x004A4370: assigns the UTF-16 path when no state is installed and appends a
// directory separator; returns whether the state is still empty.
uint8_t ResourceManSetPath(services::ResourceMan* self, const wchar_t* path);

// TS3W.exe 0x004A44C0: under the mutex walks the record tree and releases every record whose
// link list holds exactly one entry and whose flags (slot 4) report bit 1.
uint8_t ResourceManPurgeRecords(services::ResourceMan* self);

// TS3W.exe 0x004A4540: number of linked records for `key` (0 when the key is not found).
int32_t ResourceManGetRefCount(services::ResourceMan* self, const services::ServiceKey* key);

// TS3W.exe 0x004A4C10: under the mutex appends the 16-byte value of every record accepted by
// `filter` (its vtable slot 1) to the vector (end +4, capacity +8) and returns the accept
// count, regardless of whether the vector is null.
int32_t ResourceManListEntries(services::ResourceMan* self, void* vector, void* filter);

// TS3W.exe 0x004A5240: clears the record and entry trees, resets both sentinels and the
// state, and returns AL=1.
uint8_t ResourceManClear(services::ResourceMan* self);

// TS3W.exe 0x004A5610: deleting-destructor thunk for the secondary subobject (object+4).
services::ResourceMan* ResourceManDestroySecondary(services::ResourceMan* self, uint32_t flags);

// TS3W.exe 0x004A6030: deleting destructor: runs ResourceMan::Destroy (0x004A5620), then
// frees the object when bit 0 of `flags` is set. Returns the object pointer.
services::ResourceMan* ResourceManDestroy(services::ResourceMan* self, uint32_t flags);

// TS3W.exe 0x004A62B0: record open/register machine. `key` selects the tree entry, `out`
// receives the created DDFRecord (null disables creation), `flags` are the record flags,
// `mode` 6 is remapped like 0x004A3AD0, arg5 is unused, `times` optionally receives the file
// stamp (0 at +0, stamp at +4 and +8). Returns whether the entry was found or created.
uint8_t ResourceManOpenRecord(services::ResourceMan* self, const services::ServiceKey* key,
                              DDFRecord** out, uint32_t flags, int32_t mode, uint32_t unused,
                              int32_t* times);

// TS3W.exe 0x0072AE00: sets the +0x0C active byte when unset and returns AL=1.
uint8_t ResourceManActivate(services::ResourceMan* self);

// TS3W.exe 0x00739D90: returns the FileObject subobject (object+8) or null for a null object.
void* DDFRecordFileObject(DDFRecord* self);

// Raw void* adapters for the in-game harness (src/harness/harness.cpp), mirroring the
// HookIsDirectory/HookFindSubstring convention of other free-function units.
inline void* HookResourceBaseQueryInterface(void* self, uint32_t id)
{
    return ResourceBaseQueryInterface(static_cast<ResourceBase*>(self), id);
}

inline void* HookDDFRecordKeyAddress(void* self)
{
    return DDFRecordKeyAddress(static_cast<DDFRecord*>(self));
}

inline uint32_t HookDDFRecordSetKey(void* self, const void* key)
{
    return static_cast<uint32_t>(DDFRecordSetKey(static_cast<DDFRecord*>(self),
                                                  static_cast<const uint32_t*>(key)));
}

inline uint32_t HookDDFRecordIndex(void* self)
{
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(DDFRecordIndex(static_cast<DDFRecord*>(self))));
}

inline void* HookResourceManQueryInterface(void* self, uint32_t id)
{
    return ResourceManQueryInterface(static_cast<services::ResourceMan*>(self), id);
}

inline uint32_t HookResourceManClassId(void* self)
{
    return ResourceManClassId(static_cast<services::ResourceMan*>(self));
}

inline uint8_t HookResourceManActivate(void* self)
{
    return ResourceManActivate(static_cast<services::ResourceMan*>(self));
}

inline void* HookDDFRecordFileObject(void* self)
{
    return DDFRecordFileObject(static_cast<DDFRecord*>(self));
}

}  // namespace misc
}  // namespace ts3