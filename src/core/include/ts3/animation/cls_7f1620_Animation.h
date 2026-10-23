#pragma once

#include <cstdint>
#include "ts3/layout.h"

namespace ts3::animation {

// Class with vtable 0x0104415C: a clip binding (matrix + target), created by
// ClipBindingOwner::AddBinding and destroyed through its scalar deleting destructor.
struct ClipBinding {
    uint32_t vtableAddress;  // 0x00
    void* target;            // 0x04
    uint32_t flags;          // 0x08 (bit 1 set on creation)
    uint32_t unk0c;          // 0x0C
    void* target2;           // 0x10
    uint32_t unk14;          // 0x14
    uint32_t unk18;          // 0x18 (1 on creation)
    uint32_t unk1c;          // 0x1C
    alignas(16) float matrix[16];  // 0x20..0x5F, four rows

    // Original 0x007F1620 (vtable slot 6): pointer to the matrix, this + 0x20.
    float* GetMatrix();
    // Original 0x00951480 (vtable slot 0, `ret 4`): scalar deleting destructor.
    ClipBinding* Destroy(uint32_t deleteFlags);
};
TS3_X86_OFFSET(ClipBinding, matrix, 0x20);
TS3_X86_SIZE(ClipBinding, 0x60);

// Class with vtable 0x0101EFE0, initialized by 0x007FE1F0.
struct ClipPatternSet {
    uint32_t vtableAddress;  // 0x00
    uint8_t base[0x4C];      // 0x04..0x4F, set by the base initializer 0x007FE180
    int32_t unk50;
    int32_t unk54;
    int32_t unk58;
    int32_t unk5c;
    int32_t unk60;
    float unk64, unk68, unk6c, unk70;
    float unk74;
    uint32_t unk78;
    uint32_t unk7c;          // not touched by the initializer
    uint32_t zero80[8];      // 0x80..0x9F
    struct Pattern {
        uint32_t a, b, c, d;  // string addresses; d = "SACS/ClipPattern"
    } patterns[2];            // 0xA0..0xBF
    uint32_t zeroC0[6];       // 0xC0..0xD7

    // Original 0x007FE1F0.
    ClipPatternSet* Initialize();
};
TS3_X86_OFFSET(ClipPatternSet, unk50, 0x50);
TS3_X86_OFFSET(ClipPatternSet, zero80, 0x80);
TS3_X86_OFFSET(ClipPatternSet, patterns, 0xA0);
TS3_X86_OFFSET(ClipPatternSet, zeroC0, 0xC0);
TS3_X86_SIZE(ClipPatternSet, 0xD8);

// 24-byte element of ClipList; owner is at +0x14.
struct ClipEntry {
    uint8_t unk[0x14];
    void* owner;  // 0x14
};
TS3_X86_SIZE(ClipEntry, 0x18);

// Vector-like list of ClipEntry (stride 24) at this + 4.
struct ClipList {
    uint32_t unk0;
    uint8_t* begin;     // 0x04
    uint8_t* end;       // 0x08
    uint8_t* capacity;  // 0x0C

    // Original 0x00806240 (`ret 8`): returns whatever the helper that stored the
    // entry returned (0 if the end slot address is null).
    uint32_t PushBack(const ClipEntry* entry, void* arg);
};
TS3_X86_OFFSET(ClipList, end, 8);

// Parameters read by CreateController (offsets in the original structure).
struct ClipControllerParams {
    uint8_t unk[0x1B0];
    void* resource;        // 0x1B0
    uint32_t unk1b4;
    uint32_t extra;        // 0x1B8
    uint32_t mode;         // 0x1BC
    uint8_t unk1c0[0x18];
    uint8_t byte1d8;       // 0x1D8
    uint8_t byte1d9;       // 0x1D9
    uint8_t byte1da;       // 0x1DA
};
TS3_X86_OFFSET(ClipControllerParams, resource, 0x1B0);
TS3_X86_OFFSET(ClipControllerParams, byte1d8, 0x1D8);

// Object that owns a clip controller slot at this + 8 and a source at this + 0x24.
struct ClipControllerHost {
    uint32_t vtableAddress;   // 0x00
    uint8_t unk04[4];
    uint8_t controllerSlot[0x1C];  // 0x08, passed to the controller constructor
    struct Source {
        uint8_t unk[0xB0];
        uint32_t clips;  // 0xB0
    }* source;                // 0x24

    // Original 0x0093EBE0 (`ret 4`).
    void* CreateController(const ClipControllerParams* params);
};
TS3_X86_OFFSET(ClipControllerHost, controllerSlot, 8);
TS3_X86_OFFSET(ClipControllerHost, source, 0x24);

// Source of binding transforms: lookup object at +0x28, 48-byte transforms at *(+0x9C).
struct BindingSource {
    uint8_t unk[0x28];
    uint8_t lookup[0x74];       // 0x28
    const uint8_t* transforms;  // 0x9C
};
TS3_X86_OFFSET(BindingSource, transforms, 0x9C);

// Owner of a list of ClipBinding pointers (vector at this + 0x54).
struct ClipBindingOwner {
    uint8_t unk00[0x40];
    BindingSource* source;   // 0x40
    uint8_t unk44[0x10];
    ClipBinding** unk54;     // 0x54
    ClipBinding** end;       // 0x58
    ClipBinding** capacity;  // 0x5C

    // Original 0x00953170 (`ret 4`): create a binding for `target`, append it, return it.
    ClipBinding* AddBinding(void* target);
};
TS3_X86_OFFSET(ClipBindingOwner, source, 0x40);
TS3_X86_OFFSET(ClipBindingOwner, end, 0x58);

}  // namespace ts3::animation
