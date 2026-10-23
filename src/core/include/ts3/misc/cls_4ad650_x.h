// cls_4ad650_x - portable rewrite of the TS3W.exe I/O record read/write family
// (misc/, 25 entry points at 0x004AD650..0x00F20360, 2118 bytes).
//
// The family is the reader side of a packed-file record: one object implements the write
// interface (primary vtable 0x00FB2AD4 at +0x00) and embeds the read interface (vtable
// 0x00FB2A98 at +0x28).  The constructor 0x004AE2E0 lays the object out and gives the
// read subobject its vtable, so every entry point below takes one of the two views as
// `this`.  The manager entries (vtable 0x00FB2DA0) belong to the same subsystem.
//
// Calls into units that are not rewritten go through the bridge in ts3/layout.h; the
// memory-stream helper 0x004DEC90 is verified and called directly.
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3 {
namespace misc {

// ---------------------------------------------------------------------------
// TS3W.exe primary record object: vtable 0x00FB2AD4 at +0x00 and the read subobject at
// +0x28 (vtable 0x00FB2A98).  sizeof = 648 in the game (0x288).
// ---------------------------------------------------------------------------
struct PackedFileRecordRead;

struct PackedFileRecordWrite {
    void* const* vtable;   // +0x000 (0x00FB2AD4 after the constructor)
    uint32_t field04;      // +0x004
    uint32_t classId;      // +0x008 (0x12E4A892 after the constructor)
    uint32_t field0C;      // +0x00C
    uint32_t field10;      // +0x010
    uint32_t field14;      // +0x014
    uint32_t field18;      // +0x018
    uint32_t field1C;      // +0x01C
    void* resource;        // +0x020 released by PackedFileRecordReleaseResource
    uint32_t resourceState;// +0x024 (3 after the constructor)
    uint8_t tail028[0x288 - 0x028];

    // Member-form adapters for the in-game compare hooks (hookgen emits the member call
    // for thiscall rows; the bodies live below, after the free-function declarations).
    int32_t ClearResource();
    void* QueryInterfaceById(uint32_t id);
    PackedFileRecordRead* ReadView();
    void* Resource();
};
TS3_X86_OFFSET(PackedFileRecordWrite, resource, 0x20);
TS3_X86_OFFSET(PackedFileRecordWrite, resourceState, 0x24);

// ---------------------------------------------------------------------------
// The read subobject, overlaid at PackedFileRecordWrite + 0x28.
// ---------------------------------------------------------------------------
struct PackedFileRecordRead {
    void* const* vtable;   // +0x000 (0x00FB2A98)
    void* field04;         // +0x004 embedded object at primary + 0x2C
    uint8_t pad008[0x28];  // +0x008
    void* field30;         // +0x030 embedded object at primary + 0x58
    uint8_t pad034[0x258 - 0x034];
    uint8_t field258;      // +0x258 byte at primary + 0x280
    uint8_t tail259[7];    // +0x259

    // Member-form adapter for the in-game compare hook.
    uint32_t ReadClassId();
};
TS3_X86_OFFSET(PackedFileRecordRead, field04, 0x04);
TS3_X86_OFFSET(PackedFileRecordRead, field30, 0x30);
TS3_X86_OFFSET(PackedFileRecordRead, field258, 0x258);

// ---------------------------------------------------------------------------
// TS3W.exe 0x004B35A0: the manager object behind vtable 0x00FB2DA0.  The game constructor
// is outside this unit; only the fields this unit touches are named, the rest is opaque.
// ---------------------------------------------------------------------------
struct DpfManager {
    void* const* vtable;   // +0x000
    uint8_t flag04;        // +0x004
    uint8_t opaque005[0x200 - 5];
};
TS3_X86_OFFSET(DpfManager, flag04, 0x04);

// ---------------------------------------------------------------------------
// TS3W.exe 0x00924180: reference-counted object whose count is at +0x04.
// ---------------------------------------------------------------------------
struct RecordRuns {
    void* const* vtable;   // +0x000
    int32_t referenceCount;// +0x004
    uint8_t tail008[64 - 8];
};
TS3_X86_OFFSET(RecordRuns, referenceCount, 0x04);

// ---------------------------------------------------------------------------
// Primary record entries.  `this` is the primary view (or the object's vptr slot).
// ---------------------------------------------------------------------------

// TS3W.exe 0x004AD650: clears the +0x24 state before the +0x20 resource pointer and
// returns 0 (the store order is the original's).
int32_t PackedFileRecordClearResource(PackedFileRecordWrite* self);

// TS3W.exe 0x004AD660: the read subobject (object + 0x28), or null for a null object.
PackedFileRecordRead* PackedFileRecordReadView(PackedFileRecordWrite* self);

// TS3W.exe 0x004AD970: clears the resource fields and the +0x280 byte, then, when the
// +0x58 object's vtable slot 4 reports a readable state, takes its file name (slot 17,
// 260 wide chars) and calls the game function 0x004DBBD0 with it.  Returns that call's
// value, or 0 when the state is unreadable.
int32_t PackedFileRecordResetAndDiscard(PackedFileRecordWrite* self);

// TS3W.exe 0x004AD9D0: object + 0x58 when its vtable slot 4 reports active, else object
// + 0x2C.
void* PackedFileRecordActiveWriter(PackedFileRecordWrite* self);

// TS3W.exe 0x004AD9F0: when the +0x24 state is set, calls the +0x20 resource's vtable
// slot 15 with `self`; otherwise returns AL=1 (the original sets only AL).
uint8_t PackedFileRecordReleaseResource(PackedFileRecordWrite* self);

// TS3W.exe 0x00F20360: the +0x20 resource pointer.
void* PackedFileRecordResource(const PackedFileRecordWrite* self);

// TS3W.exe 0x004AF100: returns `this` for the interface ids 0x02269ED1 and 0xEE3F516E,
// null otherwise (ret 4).
void* PackedFileRecordQueryInterfaceById(void* self, uint32_t id);

// ---------------------------------------------------------------------------
// Read-subobject entries.  `this` is PackedFileRecordRead (primary + 0x28).  Each of the
// dispatch wrappers probes the embedded object at +0x30 (vtable slot 4) and forwards to a
// slot of it, or of the embedded object at +0x04 when the probe reports inactive.
// ---------------------------------------------------------------------------

// TS3W.exe 0x004ADA10: the interface id 0x12E4A892.
uint32_t PackedFileRecordReadClassId(PackedFileRecordRead* self);

// TS3W.exe 0x004ADA20: slot 7 of the active embedded object.
void* PackedFileRecordReadDispatch1C(PackedFileRecordRead* self);
// TS3W.exe 0x004ADA50: slot 8 (ret 4).
uint32_t PackedFileRecordReadDispatch20(PackedFileRecordRead* self, uint32_t a0);
// TS3W.exe 0x004ADA90: slot 9 (ret 4).
uint32_t PackedFileRecordReadDispatch24(PackedFileRecordRead* self, uint32_t a0);
// TS3W.exe 0x004ADAD0: slot 10 (ret 8).
uint32_t PackedFileRecordReadDispatch28(PackedFileRecordRead* self, uint32_t a0, uint32_t a1);
// TS3W.exe 0x004ADB10: slot 11.
uint32_t PackedFileRecordReadDispatch2C(PackedFileRecordRead* self);
// TS3W.exe 0x004ADB40: slot 12 (ret 8).
uint32_t PackedFileRecordReadDispatch30(PackedFileRecordRead* self, uint32_t a0, uint32_t a1);
// TS3W.exe 0x004ADB80: slot 13.
uint32_t PackedFileRecordReadDispatch34(PackedFileRecordRead* self);

// TS3W.exe 0x004ADBC0: the write/stream machine.  Fast path: when the +0x24 state is set
// and +0x30 is active, slot 14 of +0x30 receives (a0, a1).  Slow path: when the +0x04
// stream's slot 7 plus a1 stays under 2,000,000 and the +0x258 byte is clear and
// 0x004AD810 accepts the primary object, the stream is reset (0x004DEC90) with the
// +0x20 resource's slot 19 result and 0x004AD920 may run.  Returns the machine's byte.
uint8_t PackedFileRecordReadDispatch38(PackedFileRecordRead* self, uint32_t a0, uint32_t a1);

// ---------------------------------------------------------------------------
// Destructor thunks.
// ---------------------------------------------------------------------------

// TS3W.exe 0x004AE440: primary deleting destructor: runs the game destructor sequence
// 0x004AE3C0
// tested).  Returns the object pointer (ret 4).
void* PackedFileRecordDelete(PackedFileRecordWrite* self, uint32_t flags);

// TS3W.exe 0x004AE2D0: read-subobject deleting-destructor thunk: adjusts to the primary
// object and forwards to PackedFileRecordDelete.
void* PackedFileRecordReadDelete(PackedFileRecordRead* self, uint32_t flags);

// TS3W.exe 0x004B35A0: manager deleting destructor: runs the game destructor 0x004B30B0
// and frees the object when bit 0 of `flags` is set (only the low byte is tested).
// Returns the object pointer (ret 4).
void* DpfManagerDelete(DpfManager* self, uint32_t flags);

// ---------------------------------------------------------------------------
// Manager entries.
// ---------------------------------------------------------------------------

// TS3W.exe 0x004B05F0: mutex-protected load dispatch (6 arguments, ret 0x18).  Tries the
// vtable slot 23 factory first; otherwise resolves the a3/a4 objects through slots 17/11
// and runs the slot 21 prepare (a0, &out, 1, 3, 1, 0) plus the slot 7 commit
// (out, a1, a2, *(a0+8)).  Returns the AL byte of the machine.
uint8_t DpfManagerLoad(DpfManager* self, uint32_t a0, uint32_t a1, uint32_t a2,
                                     uint32_t a3, uint32_t a4, uint32_t a5);

// TS3W.exe 0x004B0750: same machine without the factory/mutex (5 arguments, ret 0x14).
uint8_t DpfManagerAdd(DpfManager* self, uint32_t a0, uint32_t a1, uint32_t a2,
                                    uint32_t a3, uint32_t a4);

// TS3W.exe 0x004B2770: clears the +0x04 flag, releases the +0xA0 vector entries, rebuilds
// the factory set from the +0x7C hash table, walks it, releases the +0x30 vector under the
// +0x48 mutex and returns AL=1.  When the +0x04 flag is clear it returns AL=1 immediately.
uint8_t DpfManagerShutdown(DpfManager* self);

// TS3W.exe 0x004AFD90: unused slot (10 arguments, ret 0x28); returns AL=0.
uint8_t DpfManagerUnused0(DpfManager* self, uint32_t a0, uint32_t a1, uint32_t a2,
                                        uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6,
                                        uint32_t a7, uint32_t a8, uint32_t a9);

// ---------------------------------------------------------------------------
// Miscellaneous.
// ---------------------------------------------------------------------------

// TS3W.exe 0x00924180: interlocked release on +0x04; when the count reaches zero it pins
// it to 1 and calls the object's vtable slot 2 with flag 1.  Returns the previous count.
int32_t RecordRunsRelease(RecordRuns* self);

// TS3W.exe 0x0053B170: installs the interface vtable 0x00FA1B78 and frees the object
// through 0x004E3F70 when bit 0 of `flags` is set (ret 4).  Returns the object pointer.
void* InterfaceFa1b78Destroy(void* self, uint32_t flags);

// ---------------------------------------------------------------------------
// Member-form adapters: the same bodies as the free functions, spelled the way
// hookgen emits a thiscall compare hook (cpp=ts3::misc::<Class>::<Method>).
// ---------------------------------------------------------------------------
inline int32_t PackedFileRecordWrite::ClearResource()
{
    return PackedFileRecordClearResource(this);
}
inline void* PackedFileRecordWrite::QueryInterfaceById(uint32_t id)
{
    return PackedFileRecordQueryInterfaceById(this, id);
}
inline uint32_t PackedFileRecordRead::ReadClassId()
{
    return PackedFileRecordReadClassId(this);
}
inline PackedFileRecordRead* PackedFileRecordWrite::ReadView()
{
    return PackedFileRecordReadView(this);
}
inline void* PackedFileRecordWrite::Resource()
{
    return PackedFileRecordResource(this);
}

// ---------------------------------------------------------------------------
// Raw void* adapters for the in-game harness (src/harness/harness.cpp).
// ---------------------------------------------------------------------------
inline int32_t HookPackedFileRecordClearResource(void* self)
{
    return PackedFileRecordClearResource(static_cast<PackedFileRecordWrite*>(self));
}
inline void* HookPackedFileRecordReadView(void* self)
{
    return PackedFileRecordReadView(static_cast<PackedFileRecordWrite*>(self));
}
inline uint8_t HookPackedFileRecordReleaseResource(void* self)
{
    return PackedFileRecordReleaseResource(static_cast<PackedFileRecordWrite*>(self));
}
inline void* HookPackedFileRecordResource(void* self)
{
    return PackedFileRecordResource(static_cast<const PackedFileRecordWrite*>(self));
}
inline uint32_t HookPackedFileRecordReadClassId(void* self)
{
    return PackedFileRecordReadClassId(static_cast<PackedFileRecordRead*>(self));
}
inline int32_t HookRecordRunsRelease(void* self)
{
    return RecordRunsRelease(static_cast<RecordRuns*>(self));
}
inline void* HookInterfaceFa1b78Destroy(void* self, uint32_t flags)
{
    return InterfaceFa1b78Destroy(self, flags);
}

}  // namespace misc
}  // namespace ts3