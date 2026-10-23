#pragma once
// SpeedTree / Swarm effects — class unit at 0x0092B510 (10 functions).
// Rewritten from the disassembly of TS3W.exe (Steam 1.67.2, image at 0x400000).
// Partial layouts: only the fields the rewritten functions touch are named.
#include <cstdint>
#include "ts3/layout.h"

namespace ts3::speedtree {

// ---------------------------------------------------------------------------
// Parameter block the collide walkers pass around (the list element's +4).
// 0x0092B680 / 0x0092B9C0 read the flags and the near/far effect vectors.
// ---------------------------------------------------------------------------
struct EffectParams {
    uint32_t flags;          // +0x00 bit0 collide, bit1 reflect, bit2 transform,
                             //       bit4 range-gated reflect, bit5 range-gated
                             //       (without bit4) reflect
    uint8_t pad04[0x0C];     // +0x04..+0x0F
    float stiffness;         // +0x10 near bend: +1.0 inside the refraction mix
    float spread;            // +0x14 near spread: 1.0 - this inside the mix
    float weight;            // +0x18 collision weight (scaled by the walker's scale arg)
    float fade;              // +0x1C == 0.0f forces the near vector; 1.0f forces
                             //       the far one; otherwise far iff fade >= random
                             //       (ordered; near otherwise incl. NaN, consuming
                             //       the RNG)
    uint8_t pad20[0x18];     // +0x20..+0x37
    void* nearBegin;         // +0x38 EffectInstance* array, spawned on a near hit
    void* nearEnd;           // +0x3C
    uint8_t pad40[8];        // +0x40..+0x47
    void* farBegin;          // +0x48 EffectInstance* array, spawned on a far hit
    void* farEnd;            // +0x4C
};
TS3_X86_SIZE(EffectParams, 0x50);

// One 0x18-byte entry of a collide list (walker iteration step).
struct EffectInstance {
    void* object;            // +0x00 collider; vtable slots 2/3/4/5 per the flags
    const EffectParams* params; // +0x04
    int32_t baseOn1;         // +0x08 nonzero: use offset a4 + baseOff1
    int32_t baseOff1;        // +0x0C
    int32_t baseOn2;         // +0x10 nonzero (two-base walker): use a5 + baseOff2
    int32_t baseOff2;        // +0x14
};
TS3_X86_SIZE(EffectInstance, 0x18);

// The walker's list argument (+0 begin, +4 end; step 0x18).
struct EffectList {
    EffectInstance* begin;
    EffectInstance* end;
};
TS3_X86_SIZE(EffectList, 8);

// Emit-target pointer vector of 0x0092B510's a5 (EASTL-like begin/end/cap).
struct EffectPtrVector {
    const EffectInstance** begin;
    const EffectInstance** end;
    const EffectInstance** capacity;
};
TS3_X86_SIZE(EffectPtrVector, 12);

// The 56-byte transform record: 0x0092B510 builds one on the stack and hands it
// to vtable slot +0x18; the ctor's 8 instance records are the same shape.
struct TransformRecord {
    uint16_t type;           // +0x00 (SpawnFamilyEffects writes 4)
    uint16_t flags;          // +0x02 (SpawnFamilyEffects writes 1)
    float position[3];       // +0x04
    float scale;             // +0x10
    float matrix[9];         // +0x14 row-major 3x3
};
TS3_X86_SIZE(TransformRecord, 56);

// One 0x58-byte entry of the instance vector at VisualEffect +0x220.
struct InstanceRecord {
    uint8_t pad00[0x38];     // +0x00..+0x37
    void* object;            // +0x38 vtable slot+0x10 alive check, +0x14 emit
    uint8_t pad3C[4];        // +0x3C..+0x3F skipped by 0x0092D230's stride walk
    float weight;            // +0x40 emission weight per step (scaled by step)
    uint32_t flags;          // +0x44 bit 0x40 fixed density, bit 0x80 → 0.0 else 1.0
    uint8_t pad48[0x10];     // +0x48..+0x57 record stride is 0x58
};
TS3_X86_SIZE(InstanceRecord, 0x58);

// Counters 0x0092D230 bumps (its a3).
struct EmitCounters {
    uint8_t pad00[8];        // +0x00..+0x07
    uint32_t processed;      // +0x08 instances examined
    uint32_t emitted;        // +0x0C alive instances emitted
};
TS3_X86_SIZE(EmitCounters, 16);

// The animation clock object hanging off VisualEffect +0x40 (total at +0x30).
struct TimeSource {
    uint8_t pad00[0x30];     // +0x00..+0x2F
    float total;             // +0x30 remaining seconds (read by 0x0092D230)
};
TS3_X86_SIZE(TimeSource, 0x34);

// Geometry the plane-culling test queries (VisualEffect +0x40's target).
struct EffectGeometry {
    uint32_t flags;          // +0x00 bit 0x40000 side-gated, bit 0x80000 inverting
    uint8_t pad04[0x10];     // +0x04..+0x13
    float radius;            // +0x14 compared against the plane distance
};
TS3_X86_SIZE(EffectGeometry, 0x18);

// Side object 0x0092DE40 reads through VisualEffect +0x274.
struct CullSide {
    uint8_t pad00[0xB4];     // +0x00..+0xB3
    uint32_t maskA;          // +0xB4 AND-ed with +0xB8, bit0 gates the radius test
    uint32_t maskB;          // +0xB8
};
TS3_X86_SIZE(CullSide, 0xBC);

// ---------------------------------------------------------------------------
// VisualEffect — 0x27D bytes (sizeof rounds to 0x280 with alignment).
// Constructed by 0x0092FA00, torn down by 0x009302F0.
// ---------------------------------------------------------------------------
struct VisualEffect {
    void* const* vtable0;    // +0x00 ctor writes 0x01042570
    void* const* vtable1;    // +0x04 ctor writes 0x01042528
    void* const* vtable2;    // +0x08 ctor writes 0x01042520
    uint32_t zero0C;         // +0x0C
    uint32_t zero10;         // +0x10
    uint32_t zero14;         // +0x14
    const char* nameBegin;   // +0x18 empty-string trio -> .data 0x011C7E54
    const char* nameEnd;     // +0x1C .. 0x011C7E54
    const char* nameCap;     // +0x20 .. 0x011C7E55
    const char* nameTag;     // +0x24 "Swarm/cVisualEffect/mName" @ 0x01042504
    float zero28;            // +0x28 0.0f
    uint32_t hole2C;         // +0x2C..+0x2F never written by the ctor (kept as an
                             //       explicit field: without it every later offset
                             //       would shift down by 4)
    int32_t minus1_30;       // +0x30 -1
    int32_t minus1_34;       // +0x34 -1
    int32_t minus1_38;       // +0x38 -1
    uint32_t zero3C;         // +0x3C
    TimeSource* time;        // +0x40 (0x0092D230 reads +0x30.total;
                             //       0x0092DE40 reads flags/radius through it)
    uint32_t cullFlags;      // +0x44 bit 0x10 visible; bits 0x200/0x400 gate the
                             //       on/off callbacks of 0x0092DE40
    float param48;           // +0x48 runtime copy of .data 0x011DC0F0
    float param4C;           // +0x4C runtime copy of .data 0x011DC0F4
    uint32_t zero50;         // +0x50
    float density;           // +0x54 default emission density (0x0092D230)
    uint32_t zero58;         // +0x58
    uint32_t zero5C;         // +0x5C
    TransformRecord records[8]; // +0x60..+0x21F stride 0x38
    // instance vector at +0x220: pointers to 0x58-byte InstanceRecords
    InstanceRecord* instBegin;  // +0x220
    InstanceRecord* instEnd;    // +0x224
    InstanceRecord* instCap;    // +0x228
    const char* componentTag;   // +0x22C "Swarm/VisualEffect/ComponentRecs" @ 0x010424E0
    void* obj230;            // +0x230 released by 0x009302F0 (vtable slot +2)
    uint32_t zero234;        // +0x234
    void* obj238;            // +0x238 released by 0x009302F0 (vtable slot +2)
    uint32_t zero23C;        // +0x23C
    float negOne240[3];      // +0x240 -1.0f x3
    float cornerA[3];        // +0x24C .data F8,FC,100
    float cornerB[3];        // +0x258 .data F8,FC,100
    float cornerC[3];        // +0x264 .data 104,108,10C
    uint32_t zero270;        // +0x270
    CullSide* side;          // +0x274 read by 0x0092DE40 (+0xB4/+0xB8)
    uint32_t zero278;        // +0x278
    uint8_t flag27C;         // +0x27C

    // 0x0092FA00 — initialise a fresh VisualEffect (returns this; size 0x27D).
    VisualEffect* Construct();

    // 0x009302F0 — release the owned objects, clear the instance vector and the
    // name string, reset the vtables; returns this + 4 (null for a null this).
    uint8_t* Destruct();

    // 0x0092D230 — advance the effect clock by 1/15 s steps and emit through
    // every alive instance (thiscall, ret 8; a2 unused, a3 = counters).
    // Early-out returns the time pointer; the loop path's eax is junk.
    float* StepInstanceEffects(int32_t unusedA2, EmitCounters* counters);

    // 0x0092DE40 — update the visible flag from the plane distance vs the
    // geometry radius (thiscall, ret 4); fires the on/off callbacks. The
    // returned eax varies per path (see the .cpp); replicated as int32_t.
    int32_t UpdatePlaneCulling(const float* plane /*pt[3] then n[3]*/);
};
TS3_X86_OFFSET(VisualEffect, time, 0x40);
TS3_X86_OFFSET(VisualEffect, cullFlags, 0x44);
TS3_X86_OFFSET(VisualEffect, records, 0x60);
TS3_X86_OFFSET(VisualEffect, instBegin, 0x220);
TS3_X86_OFFSET(VisualEffect, side, 0x274);
TS3_X86_OFFSET(VisualEffect, flag27C, 0x27C);

// ---------------------------------------------------------------------------
// Free functions (cdecl entries).
// ---------------------------------------------------------------------------

// 0x0092B510 — spawn one effect instance from the family list (a2) held by the
// manager (a1) and configure it from the wind vector; optionally appends it to
// the target pointer vector (grow helper 0x008B70E0 when full) and tail-calls
// its Release. Returns 0 when the factory yields nothing.
int32_t SpawnFamilyEffects(void* manager, const EffectInstance** family, const float* wind,
                           int32_t baseOffset, EffectPtrVector* addTo, void* attachTo,
                           const char* name);

// 0x0092B680 — walk the collide list; reflect/transform the position in place
// and spawn the near/far instance families on a hit. Returns 1 when the list is
// exhausted without a far hit, 0 after a far spawn. Both spawns use a1 as the
// factory manager (a2 is never read); a9/a10/a11 feed the spawn (set-base arg /
// attachTo / name); a6 is the collide/transform attach arg, a7 the wind.
char CollideEffects(void* manager, void* farManager, const EffectList* list,
                    int32_t baseOffset, float scale, void* attachTo, const float* wind,
                    float position[3], int32_t spawnBaseOffset, void* spawnAttachTo,
                    const char* name);

// 0x0092B9C0 — same for two bases (per-element base1/base2 offsets). a7/a8 are
// the wind/vector pair the slots take, a9 the position the walker rewrites,
// a10 the extra slot-4/5 argument, a11..a14 feed the spawn (set-base arg /
// addTo / attachTo / name); both spawns use a1 as the factory manager; a2 is
// never read.
char CollideEffectsTwoBases(void* manager, void* farManager, const EffectList* list,
                            int32_t base1, int32_t base2, float scale,
                            const float* wind, const float* vec2, float position[3],
                            int32_t slotArg, int32_t spawnBaseOffset,
                            EffectPtrVector* spawnAddTo, void* spawnAttachTo,
                            const char* name);

// 0x0092C000 — rejection-sample a point in the box {min[0..2], max[0..2]} via the
// game RNG at .data 0x011DC060 (slot 0x004FA870, 3 draws per try). Returns out.
float* RandomPointInBox(float* out, const float* box);

// 0x0092C220 — mid-position wind matrix: copies wind to out[3..5], then writes
// the basis rows at out[0..2] = (y, -x, 0)/len and out[6..8] = cross products;
// a degenerate (x,y) length writes (0,1,0)/(1,0,0) instead.
void MakeWindMatrixMidPos(const float* wind, void* out36);

// 0x0092C320 — lead-position wind matrix: copies wind to out[0..2], then writes
// the basis rows at out[3..5] = (-y, x, -0)/len and out[6..8] = cross products;
// a degenerate length writes (1,0,0)/(0,1,0) instead.
void MakeWindMatrixLeadPos(const float* wind, void* out36);

} // namespace ts3::speedtree
