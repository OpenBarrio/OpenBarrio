// CAS class unit cls_4a6d60_CAS (TS3W.exe 0x004A6D60 .. 0x0072C610). Four families share the
// unit because IDA groups them under one class:
//
//   * 0x004FA4C0  Crc32(data, length, crc, invert)             (cdecl, leaf)
//   * 0x004A6D60 / 0x004A6DC0  the resource::DatabasePackedFile user-version accessors
//     (rows 004a6d60/004a6dc0 of the packed-file pilot), plus the two vtable-only helpers
//     0x0072C580 (verify) and 0x0072C610 (refresh) that produce/consume their values
//   * 0x005BBB70..0x005BC4AF  the CAS/CompositedTextureMediator (72 bytes)
//   * 0x006CBF90..0x006CEC2B  the CAS/MaterialTextureCompositor (the object whose map is
//     "CAS/MaterialTextureCompositor/mCacheChecks" and whose part table sits at +0x2A0)
//
// The DatabasePackedFile layout itself is owned by the packed-file pilot
// (ts3/resource/packed_file.h); DbpfUserVersionView exists only so the two rewritten
// members keep the original __thiscall ABI without editing that header.
#pragma once

#include <cstdint>

#include "ts3/cas/clu_5a37e0_CAS.h"  // CacheMap (same batch), embedded at +0x08 below
#include "ts3/layout.h"
#include "ts3/resource/packed_file.h"

namespace ts3::cas {

struct DNameNode;
struct MaterialTextureCompositor;

// The game's shift-left CRC-32 (table at 0x0114D330, poly 0x04C11DB7):
//   crc = table[(uint8_t)crc_index ^ (crc >> 24)] ^ (crc << 8)
// `invert` complements the result. TS3W.exe 0x004FA4C0, __cdecl (the caller pops).
uint32_t Crc32(const uint8_t* data, int32_t length, uint32_t crc, uint8_t invert);

// The two user-version methods of resource::DatabasePackedFile. `this` is really a
// resource::DatabasePackedFile (size 0x398); see ts3/resource/packed_file.h for the fields.
struct DbpfUserVersionView {
    // TS3W.exe 0x004A6D60, thiscall ret 8. Locks the record mutex; when the file is open
    // (or EnsureOpen succeeds, itself rewritten) copies header 0x0C/0x10 into *major/*minor
    // and returns 1, otherwise 0.
    uint8_t GetUserVersion(uint32_t* major, uint32_t* minor);
    // TS3W.exe 0x004A6DC0, thiscall ret 8. Under the record mutex: newUserVersionMinor,
    // newUserVersionMajor (in that order) and headerDirty = 1.
    void SetNewUserVersion(uint32_t major, uint32_t minor);
};

// TS3W.exe 0x0072C580, __stdcall ret 0xC: the first stack argument is the packed file, the
// next two are the data and its length (the original also reads the file into ecx). Returns
// 0 when GetUserVersion fails, or when the file's userVersionMajor is not the CRC of itself
// seeded with the data's CRC; reports a non-zero remainder mod 7 through 0x00C0FD60 and
// returns 1. No caller exists in the image: the function is only in a vtable.
uint8_t __stdcall VerifyUserVersionCrc(DbpfUserVersionView* file, const uint8_t* data, uint32_t length);

// TS3W.exe 0x0072C600, thiscall ret 0 (DNameNode, 0x0C bytes): vtable 0x00FE2594, +0x04 = 0.
// Returns `this`.
struct DNameNode {
    void* vtable;      // +0x00
    uint32_t field04;  // +0x04, the `% 7` value 0x0072C610 wants
    uintptr_t field08; // +0x08, untouched by 0x0072C600 (the mediator's +0x10 list anchor)
    DNameNode* Construct();
};
TS3_X86_OFFSET(DNameNode, field04, 4);
TS3_X86_OFFSET(DNameNode, field08, 8);
TS3_X86_SIZE(DNameNode, 0x0C);

// TS3W.exe 0x0072C610, thiscall ret 0xC (the node travels in ecx, the rest on the stack).
// Picks a performance-counter-derived value whose CRC (seeded with the data's CRC,
// inverted) leaves `node->field04` as its remainder mod 7, stores it with SetNewUserVersion
// and returns 1. `node` is the ecx argument; there are no callers in the image.
uint8_t RefreshUserVersionFromNode(DNameNode* node, DbpfUserVersionView* file, const uint8_t* data, uint32_t length);

// TS3W.exe 0x005BBB70, thiscall ret 0xC: CAS/CompositedTextureMediator (72 = 0x48 bytes).
// `memory` is the caller's 72-byte block; the constructor also initializes its embedded
// DNameNode at +0x08 (whose +0x08 is this object's +0x10 list anchor) and asks the
// "UnboundedTCC" service (0x0058B100) whether +0x38 is -1.
struct CompositedTextureMediator {
    void* vtable;           // +0x00 = 0x00FE2580
    int32_t field04;        // +0x04
    DNameNode node;         // +0x08 .. +0x14
    uintptr_t list14;       // +0x14 the +0x10 list anchor's back link
    const char* name;       // +0x18 = 0x00FE2540 "CAS/CompositorCache/MRUList"
    uint32_t field1C;       // +0x1C
    int64_t key;            // +0x20 sign-extended first argument
    uint32_t magic28;       // +0x28 = 0x04D82D90
    uint32_t field2C;       // +0x2C = 0
    void* field30;          // +0x30 released through its own vtable slot 1 by Destroy
    uint32_t field34;       // +0x34 = 0
    int32_t field38;        // +0x38 third argument, or -1 with UnboundedTCC
    int32_t field3C;        // +0x3C second argument
    int32_t field40;        // +0x40 first argument
    uint32_t field44;       // +0x44 not touched by the constructor (the block is 0x48)
    // TS3W.exe 0x005BBB70. `memory` must be 0x48 bytes.
    static CompositedTextureMediator* Construct(void* memory, int32_t arg1, int32_t arg2, int32_t arg3);
    // TS3W.exe 0x005BB960, thiscall ret 4 (tail call into 0x005BB770 with `field30`).
    uint32_t ReleaseKey();
    // TS3W.exe 0x005BBF60, thiscall ret 0. Resets the two vtables, drains the list anchored
    // at node.field08 through 0x004E3F70 and ends with the base vtable 0x00FA1B78.
    uint32_t Destroy();
    // TS3W.exe 0x005BC490, thiscall ret 4. Destroy, then frees `this` when flags bit 0 is
    // set; returns `this`.
    CompositedTextureMediator* DeleteAndDestroy(int flags);
};
TS3_X86_OFFSET(CompositedTextureMediator, node, 0x08);
TS3_X86_OFFSET(CompositedTextureMediator, list14, 0x14);
TS3_X86_OFFSET(CompositedTextureMediator, name, 0x18);
TS3_X86_OFFSET(CompositedTextureMediator, key, 0x20);
TS3_X86_OFFSET(CompositedTextureMediator, field30, 0x30);
TS3_X86_OFFSET(CompositedTextureMediator, field38, 0x38);
TS3_X86_SIZE(CompositedTextureMediator, 0x48);

// The +4 base sub-object of MaterialTextureCompositor (0x006CE900 lives in its vtable).
struct MaterialTextureCompositorBase4 {
    // TS3W.exe 0x006CE900, thiscall ret 4: an adjustor thunk, the incoming ecx is the +4
    // base pointer. Runs 0x006CEC10 on the enclosing object (ecx - 4).
    MaterialTextureCompositor* Delete(int flags);
};

// TS3W.exe 0x006CEB20 .. 0x006CEC2B: CAS/MaterialTextureCompositor. The object is at least
// 0xAD0 bytes: a CacheMap at +0x08 (whose +0x20/+0x24 links are the +0x28 list), the
// mediator and resource at +0x34/+0x38, lists at +0x3C/+0x44/+0x6C, the part table at
// +0x2A0 (522 slots, slot 521 at +0xAC4 is the sentinel node) and two dwords at +0xAC8.
struct MaterialTextureCompositor {
    void* vtable;                  // +0x000 = 0x00FF5E50
    void* vtable2;                 // +0x004 = 0x00FF5E48
    CacheMap map;                  // +0x008
    uint32_t field30;              // +0x030 "CAS/MaterialTextureCompositor/RequestsCompletedNotifiers"
    void* mediator;                // +0x034 the CompositedTextureMediator
    void* resource;                // +0x038 released through (value + 4)'s vtable slot 1
    uintptr_t list3C[2];           // +0x03C intrusive list head
    uintptr_t list44[2];           // +0x044 intrusive list head
    uint8_t unknown4C[0x20];       // +0x04C zeroed by the constructor
    uintptr_t list6C[2];           // +0x06C intrusive list head
    uint8_t unknown74[0x224];      // +0x074
    int32_t field298;              // +0x298 = -1
    uint8_t unknown29C[4];         // +0x29C
    uintptr_t partTable[522];      // +0x2A0 .. +0xAC7; [521] = the sentinel node pointer
    int32_t fieldAC8;              // +0xAC8 = -1
    int32_t fieldACC;              // +0xACC = 0

    // TS3W.exe 0x006CEB20, thiscall ret 0. Constructor; ends with Rebuild(1, 0, 0).
    MaterialTextureCompositor* Construct();
    // TS3W.exe 0x006CE7A0, thiscall ret 0. Destructor core. Returns its last eax.
    uint32_t DestroyCore();
    // TS3W.exe 0x006CEC10, thiscall ret 4 (0x006CE900 is its +4 adjustor thunk). DestroyCore,
    // then frees `this` when flags bit 0 is set; returns `this`.
    MaterialTextureCompositor* DeleteAndDestroy(int flags);
    // TS3W.exe 0x006CBDB0, thiscall ret 4. The deleting destructor of the +4 base: sets
    // +0x04 to the base vtable 0x00FF5B44 and frees `this` when flags bit 0 is set.
    MaterialTextureCompositor* DeleteBase(int flags);
    // TS3W.exe 0x006CBF90, thiscall ret 0xC. Rebuilds the mediator: releases the old one,
    // builds the wide-string source (`create` picks "Co_0", else "Wo"), allocates the
    // 0x48-byte mediator from 0x004E3F90 under "CAS/CompositedTextureMediator", installs it
    // at +0x34, stores `this + 4` into its +0x34 and releases the temporary string.
    void Rebuild(int8_t create, void* arg2, void* resource);
    // TS3W.exe 0x006CC0A0, thiscall ret 0. When +0x38 holds a resource, rebuilds the
    // mediator, unregisters the resource from the 0x004AFD20 service and releases it.
    void ReleaseResource();
    // TS3W.exe 0x006CC570, thiscall ret 4. Builds a temporary cache object for `resource`
    // (skipped when the "IgnoreWorldCache" service is on) and rebuilds the mediator with it.
    void RefreshWithResource(void* resource);
    // TS3W.exe 0x006CCB40, thiscall ret 8. Handles the two CAS messages that rebuild the
    // mediator (0x038E3C0E with a resource, 0x038E3C6C without); other messages return 0.
    uint8_t HandleMessage(int32_t message, void* resource);
    // TS3W.exe 0x006CCA00, thiscall ret 4. Clears the four dwords at node[8] + 0x48 for
    // every part-table node whose key[0]/key[1] equal key[0]/key[1]; always returns 1.
    uint8_t ClearMatchingPartTypes(const uint32_t* key);
};
TS3_X86_OFFSET(MaterialTextureCompositor, map, 0x008);
TS3_X86_OFFSET(MaterialTextureCompositor, field30, 0x030);
TS3_X86_OFFSET(MaterialTextureCompositor, mediator, 0x034);
TS3_X86_OFFSET(MaterialTextureCompositor, resource, 0x038);
TS3_X86_OFFSET(MaterialTextureCompositor, field298, 0x298);
TS3_X86_OFFSET(MaterialTextureCompositor, partTable, 0x2A0);
TS3_X86_OFFSET(MaterialTextureCompositor, fieldAC8, 0xAC8);
TS3_X86_OFFSET(MaterialTextureCompositor, fieldACC, 0xACC);
TS3_X86_SIZE(MaterialTextureCompositor, 0xAD0);

// TS3W.exe 0x00722E80, __cdecl ret 0: releases the global request list's payload through
// 0x006A0850, then resets the list head at 0x0115B874 and its three fields. Returns 0.
int InitRequestList();

}  // namespace ts3::cas