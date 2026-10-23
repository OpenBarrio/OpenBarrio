// Rewritten from TS3W.exe unit cls_683120_Lot (7 functions):
//   0x00683120  LotFloorCollisionShape::Construct      (thiscall, ret 0)    vtable 0x00FEFD10
//   0x00683130  LotFloorCollisionShape::ShapeCode      (thiscall, ret 0)    vtable slot 1
//   0x00A8CA70  LotFloorCollisionShape::ConstructFrom  (thiscall, ret 4)
//   0x00C29B80  HeightFieldShape::Construct            (thiscall, ret 0x10) vtable 0x0107690C
//   0x00C29C60  HeightFieldShape::Sample               (thiscall, ret 0xC)
//   0x00C2A240  HeightFieldShape::ConstructFrom        (thiscall, ret 4)
//   0x00C2AAF0  HeightFieldShape::Clone                (thiscall, ret 4)
//
// The base ctor 0x00682890 lives in clu_682890_Lot, the base copy ctor 0x00682950 and
// the refcount bump 0x00682750 in clu_67f4a0_Physics, and the allocator 0x004E3FB0 in
// clu_4e3fb0_Renderer - none rewritten yet, all called through the game bridge.
//
// Layout follows TS3W.exe objects on x86 (see layout.h); x64 builds let the pointer
// fields grow - the asserts only apply where a 32-bit game exists to match.
#pragma once

#include <cstdint>

#include <ts3/layout.h>

namespace ts3::lot {

// 0x00FEFCB0 - root of the collision-shape family; its constructor lives in
// clu_682890_Lot (0x00682890) and its copy ctor in clu_67f4a0_Physics (0x00682950),
// both reached through the bridge from this unit's functions.
class LotCollisionShape {
public:
    void* vtable;                       // +0x00
    // +0x04..+0x0F: no function of this unit ever reads or writes these bytes (the EA
    // constructors leave whatever was in the allocation).
    uint8_t data04[0x58];               // +0x04..+0x5B: base payload owned by the
                                        // other units (fields at +0x38/+0x3C = -1,
                                        // +0x4C refcount, +0x48..+0x58 zeroed by the
                                        // EA copy ctor instead of copied)
};
TS3_X86_SIZE(LotCollisionShape, 0x5C);

// 0x00FEFD10
class LotFloorCollisionShape : public LotCollisionShape {
public:
    uint8_t pad5c[4];                   // +0x5C..+0x5F: untouched by this unit
    uint8_t data60[0x30];               // +0x60..+0x8F: owned by this class; copied
                                        // verbatim by 0x00A8CA70 (2x movaps + 4 dwords)

    // 0x00683120: runs the base ctor (0x00682890 through the bridge) and installs
    // vtable 0x00FEFD10; writes +0x00..+0x5B only (+0x04..+0x0F keep the old bytes);
    // returns this.
    LotFloorCollisionShape* Construct();
    // 0x00683130: vtable slot 1 of both LotFloorCollisionShape and
    // HeightFieldShape - returns the fixed shape code 0x065BAD16 and touches nothing.
    uint32_t ShapeCode();
    // 0x00A8CA70: base copy ctor 0x00682950 through the bridge, then vtable
    // 0x00FEFD10
    // reads/writes +0x60..+0x8F with movaps); returns this.
    LotFloorCollisionShape* ConstructFrom(const LotFloorCollisionShape* src);
};
TS3_X86_SIZE(LotFloorCollisionShape, 0x90);

// 0x0107690C - a floor shape carrying a uint16 height grid.
class HeightFieldShape : public LotFloorCollisionShape {
public:
    void* samples;                      // +0x90: base of the uint16 grid (a pointer;
                                        // the ctor receives it as a raw stack dword)
    int32_t stride;                     // +0x94: row stride in bytes
    int32_t width;                      // +0x98: x bound (sample x clamps against it)
    int32_t height;                     // +0x9C: y bound

    // 0x00C29B80: calls 0x00683120, stores the four dwords at +0x90..+0x9C, installs
    // vtable 0x0107690C; returns this.
    HeightFieldShape* Construct(uint32_t samplesAddr, int32_t stride, int32_t width,
                                int32_t height);
    // 0x00C29C60: clamps x against width and y against height (unsigned compare: an
    // out-of-range non-negative index pulls to the bound, a negative one to 0), reads
    // *(uint16*)(samples + y*stride + 2*x), converts to float and broadcasts it to
    // all four lanes of *(__m128*)out (16 bytes; out must be 16-aligned); returns out.
    void* Sample(void* out, int32_t x, int32_t y);
    // 0x00C2A240: 0x00A8CA70 then vtable 0x0107690C and the four fields at
    // +0x90..+0x9C from src; returns this.
    HeightFieldShape* ConstructFrom(const HeightFieldShape* src);
    // 0x00C2AAF0: allocates 0xA0 bytes aligned 0x10 through 0x004E3FB0 (tag
    // "Terrain/ClonedHFShape"), copy-constructs from this, writes the new pointer to
    // *out (always, even when the allocation failed), bumps the refcount through
    // 0x00682750
    // EA quirk: the success path only sets AL after the refcount call, so the full
    // return keeps the refcount's high bytes; the copy ctor zeroes +0x4C first, so
    // the count is 1 and the result is 1.
    uint32_t Clone(void** out);
};

TS3_X86_SIZE(HeightFieldShape, 0xA0);
TS3_X86_OFFSET(HeightFieldShape, samples, 0x90);
TS3_X86_OFFSET(HeightFieldShape, stride, 0x94);
TS3_X86_OFFSET(HeightFieldShape, width, 0x98);
TS3_X86_OFFSET(HeightFieldShape, height, 0x9C);

}  // namespace ts3::lot
