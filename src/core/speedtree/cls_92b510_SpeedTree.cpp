// SpeedTree / Swarm effects — class unit at 0x0092B510 (10 functions).
// Rewritten from the disassembly of TS3W.exe (Steam 1.67.2, image base 0x400000).
// Original addresses are noted on every function. FP op order is transcribed
// literally from the x87/SSE sequences so bit patterns match the original.
#include <cmath>
#include <cstring>
#include "ts3/speedtree/cls_92b510_SpeedTree.h"

namespace ts3 {
// Absolute read of a .data/.rodata global. In exehost (no static ctors) these
// hold the file's bytes, identically for the original and the rewrite; in the
// live game they hold the initialised values.
template <class T>
inline T ImageWord(uintptr_t address)
{
    T value;
    std::memcpy(&value, reinterpret_cast<const void*>(address), sizeof value);
    return value;
}
} // namespace ts3

namespace ts3::speedtree {
namespace {

// FLD/FSTP quiets signaling NaNs, unlike an SSE bit-copy.
float X87Copy(float value)
{
#if defined(_M_IX86)
    float result;
    __asm { fld value }
    __asm { fstp result }
    return result;
#else
    uint32_t bits;
    std::memcpy(&bits, &value, 4);
    if ((bits & 0x7f800000u) == 0x7f800000u && (bits & 0x007fffffu))
        bits |= 0x00400000u;
    std::memcpy(&value, &bits, 4);
    return value;
#endif
}

// 0x004FA870 — thiscall RandomFloat(this=&g_rng @ .data 0x11DC060), returns a
// double in [0,1). Not rewritten here (belongs to clu_4fa870_SpeedTree).
inline double RandomFloat()
{
    return ts3::CallGameMethod<double, 0x004FA870>(
        reinterpret_cast<void*>(static_cast<uintptr_t>(0x011DC060)));
}

// 0x008B70E0 — __stdcall grow helper for an effect-pointer vector
// (stdcall, 2 stack args, ret 8): pushes the new slot address and the old end.
// Not in layout.h (which only bridges cdecl/thiscall), so it lives here.
template <class R, unsigned kAddress, class... Args>
R CallGameStdcall(Args... args)
{
#if defined(_M_IX86)
    using Fn = R(__stdcall*)(Args...);
    return reinterpret_cast<Fn>(static_cast<uintptr_t>(kAddress))(args...);
#else
    (void)sizeof...(args);
    ts3::AbortNotRewritten("a game __stdcall function");
    return R{};
#endif
}

// 0x0091AC60 — thiscall factory: (this=manager, familyList, name) -> instance*.
// Not rewritten (belongs to clu_409b50_Swarm).
inline void* SpawnInstance(void* manager, const EffectInstance** family, const char* name)
{
    return ts3::CallGameMethod<void*, 0x0091AC60>(manager, family, name);
}

// 0x0091A440 — thiscall vector teardown (walks 0x58-byte records).
// Not rewritten (belongs to clu_409b50_Swarm).
inline void DestroyInstanceVector(void* self, void* begin, void* end)
{
    ts3::CallGameMethod<void, 0x0091A440>(self, begin, end);
}

// 0x004E3F70 — cdecl free(). Not rewritten (clu_4e3f70_Scratch).
inline void GameFree(void* p)
{
    ts3::CallGameFunction<void, 0x004E3F70>(p);
}

// The 0.5 constant at 0x00F9A5AC used by the box midpoint map.
inline float Half() { return ts3::ImageWord<float>(0x00F9A5AC); }

// The 1.0 constant at 0x0107A538 (scale, length normaliser).
inline float One() { return ts3::ImageWord<float>(0x0107A538); }

// The -0.0 constant at 0x00F988F0 (the negate-by-subtract idiom).
inline float NegZero() { return ts3::ImageWord<float>(0x00F988F0); }

// The 1/15 clock step constant at 0x0103ED88.
inline float ClockStep() { return ts3::ImageWord<float>(0x0103ED88); }

} // namespace

// ---------------------------------------------------------------------------
// 0x0092C220 — MakeWindMatrixMidPos (cdecl, void, out is 9 floats)
//
// Copies wind to out[3..5] unconditionally, then builds two basis rows from
// the XY wind: out[0..2] = (y, -x, 0)/len and out[6..8] the cross products.
// A degenerate XY length (!(len > 0), incl. NaN) writes (0,1,0)/(1,0,0).
// ---------------------------------------------------------------------------
void MakeWindMatrixMidPos(const float* wind, void* out36)
{
    float* out = static_cast<float*>(out36);
    out[3] = X87Copy(wind[0]);
    out[4] = X87Copy(wind[1]);
    out[5] = X87Copy(wind[2]);
    const float x = wind[0];
    const float y = wind[1];

    // x87: y squared first, then x^2, sum, sqrt (extended), rounded to float.
    const float len = static_cast<float>(
        std::sqrt(static_cast<double>(y) * y + static_cast<double>(x) * x));
    if (!(len > 0.0f)) {
        out[0] = 0.0f;
        out[1] = One();
        out[2] = 0.0f;
        out[6] = One();
        out[7] = 0.0f;
        out[8] = 0.0f;
        return;
    }
    const float invLen = One() / len;
    out[0] = y * invLen;
    out[1] = invLen * (-x);
    out[2] = invLen * ts3::ImageWord<float>(0x00F98964); // mulss invLen, [0xF98964=+0.0]
    // Cross products against the wind (out[6..8] = z*out1 - y*out2, ...).
    const float crossZ = wind[2], crossY = wind[1], crossX = wind[0];
    out[6] = crossZ * out[1] - crossY * out[2];
    out[7] = crossX * out[2] - crossZ * out[0];
    out[8] = crossY * out[0] - crossX * out[1];
}

// ---------------------------------------------------------------------------
// 0x0092C320 — MakeWindMatrixLeadPos (cdecl, void, out is 9 floats)
//
// Copies wind to out[0..2], then builds out[3..5] = (-y, x, -0)/len and the
// cross products at out[6..8]. Degenerate length writes (1,0,0)/(0,1,0).
// ---------------------------------------------------------------------------
void MakeWindMatrixLeadPos(const float* wind, void* out36)
{
    float* out = static_cast<float*>(out36);
    const float x = wind[0];
    const float y = wind[1];
    const float z = wind[2];
    out[0] = X87Copy(x);
    out[1] = X87Copy(y);
    out[2] = X87Copy(z);

    const float c = NegZero();
    // The original derives the components by double subtraction through -0.0:
    // b = -0.0 - (-0.0 - x) (== x), cy = -0.0 - y (== -y). Transcribed as is.
    const float a = c - x;
    const float b = c - a;
    const float cy = c - y;
    const float len = static_cast<float>(
        std::sqrt(static_cast<double>(b) * b + static_cast<double>(cy) * cy));
    if (!(len > 0.0f)) {
        out[3] = One();
        out[4] = 0.0f;
        out[5] = 0.0f;
        out[6] = 0.0f;
        out[7] = One();
        out[8] = 0.0f;
        return;
    }
    const float invLen = One() / len;
    out[4] = b * invLen;
    out[5] = invLen * c;   // mulss invLen, -0.0f
    out[3] = cy * invLen;
    out[6] = y * out[5] - z * out[4];
    out[7] = z * out[3] - x * out[5];
    out[8] = x * out[4] - y * out[3];
}

// ---------------------------------------------------------------------------
// 0x0092C000 — RandomPointInBox (cdecl, returns out)
//
// Rejection-samples a point in the unit ball (3 RNG draws per try; the draws
// are mapped to [-1,1) in extended precision: (float)(2u - 1.0f)), then maps
// it into the box {min[0..2], max[0..2]} by ((hi-lo)*r + (lo+hi)) * 0.5.
// ---------------------------------------------------------------------------
float* RandomPointInBox(float* out, const float* box)
{
    const float one = One();
    const float half = Half();
    float r[3];
    for (;;) {
        for (int i = 0; i < 2; ++i) {
            const double u = RandomFloat();
            r[i] = static_cast<float>(u + u - static_cast<double>(one));
        }
        const double u = RandomFloat();
        double sum;
#if defined(_M_IX86)
        float third;
        const float first = r[0], second = r[1];
        // C040 is FST, not FSTP: retain the mapped third draw on x87
        // through the square and additions, rounding only the output copy.
        __asm { fld u }
        __asm { fadd st(0), st(0) }
        __asm { fsub one }
        __asm { fst third }
        __asm { fmul st(0), st(0) }
        __asm { fld second }
        __asm { fmul st(0), st(0) }
        __asm { faddp st(1), st(0) }
        __asm { fld first }
        __asm { fmul st(0), st(0) }
        __asm { faddp st(1), st(0) }
        // Compare before spilling the sum (rounding at the unit boundary
        // could otherwise turn a rejected point into an accepted one).
        unsigned char accepted;
        __asm { fld1 }
        __asm { fcomip st(0), st(1) }
        __asm { fstp st(0) }
        __asm { setae accepted }
        r[2] = third;
        sum = accepted ? 0.0 : 2.0;
#else
        const long double third = static_cast<long double>(u) * 2 - one;
        r[2] = static_cast<float>(third);
        sum = (third * third + static_cast<long double>(r[1]) * r[1]) +
              static_cast<long double>(r[0]) * r[0];
#endif
        if (sum <= 1.0) {
            break;
        }
    }
    for (int i = 0; i < 3; ++i) {
        const float lo = box[i];
        const float hi = box[i + 3];
        out[i] = ((hi - lo) * r[i] + (lo + hi)) * half;
    }
    return out;
}

// ---------------------------------------------------------------------------
// 0x0092B510 — SpawnFamilyEffects (cdecl, 7 args)
//
// Spawns one instance via the 0x0091AC60 factory (this=a1 manager, a2 family,
// a7 name), AddRefs it, hands it a 56-byte stack transform record (type 4,
// flags 1, position = wind[0..2], scale 1.0, matrix = the globals at
// 0x11DC020..0x11DC040), then drives vtable slots 6/5/16/2. Appends the
// instance to the emit vector a5 (stdcall grow helper 0x008B70E0 when full)
// and tail-calls Release (slot 1), returning its result. Factory null -> 0.
// ---------------------------------------------------------------------------
int32_t SpawnFamilyEffects(void* manager, const EffectInstance** family, const float* wind,
                           int32_t baseOffset, EffectPtrVector* addTo, void* attachTo,
                           const char* name)
{
    void* obj = SpawnInstance(manager, family, name);
    if (!obj) {
        return 0;
    }
    // vtable[0] AddRef.
    ts3::CallGameVirtual<void, 0>(obj);

    // The 56-byte transform record on the stack.
    TransformRecord rec{};
    rec.type = 4;
    rec.flags = 1;
    rec.position[0] = wind[0];
    rec.position[1] = wind[1];
    rec.position[2] = wind[2];
    rec.scale = One();
    for (int i = 0; i < 9; ++i) {
        rec.matrix[i] = ts3::ImageWord<float>(0x011DC020 + static_cast<uintptr_t>(i) * 4);
    }
    // vtable[6] set-record (slot +24).
    ts3::CallGameVirtual<void, 6>(obj, &rec);
    // vtable[5] attach (slot +20) — only when attachTo non-null.
    if (attachTo) {
        ts3::CallGameVirtual<void, 5>(obj, attachTo);
    }
    // vtable[16] set-base (slot +64).
    ts3::CallGameVirtual<void, 16>(obj, baseOffset);
    // vtable[2] activate (slot +8, one zero arg).
    ts3::CallGameVirtual<void, 2>(obj, 0);

    int32_t growResult = 0;
    if (addTo) {
        const EffectInstance** end = addTo->end;
        if (end >= addTo->capacity) {
            // __stdcall 0x008B70E0(end, &obj): grows the vector and appends
            // *(&obj); the original reloads obj from the same slot after it.
            void* objLocal = obj;
            growResult = CallGameStdcall<int32_t, 0x008B70E0>(end, &objLocal);
            obj = objLocal;
        } else {
            addTo->end = end + 1;
            if (end) {
                *end = reinterpret_cast<const EffectInstance*>(obj);
                // vtable[0] AddRef again (the vector's own reference).
                ts3::CallGameVirtual<void, 0>(obj);
            }
        }
    }
    if (!obj) {
        // Only reachable if the grow helper nulls obj; returns its eax then.
        return growResult;
    }
    // Tail-call vtable[1] Release (slot +4); its result is our return value.
    return ts3::CallGameVirtual<int32_t, 1>(obj);
}

// ---------------------------------------------------------------------------
// Shared walker body for 0x0092B680 / 0x0092B9C0.
//
// Walks the 0x18-byte collide list. Per element, params->flags dispatch:
//   bit2  -> reflect through slot 4 (fills the plane, bounces pos if t > 0)
//   bit4  -> transform pos through slot 5
//   bit0  -> collide through slot 2 (weight*scale); on a hit, fade picks the
//            near or far family; near also refraction-blends pos and continues
//   else  -> discarded reflect through slot 4
//            (B680: bit0x10 set, gated by slot 3's double vs the raw weight;
//             B9C0: bit0x10 clear && bit0x20 set, no gate)
// The two walkers' slot argument orders differ (per the call sites below): a2
// is never read by either; every spawn uses a1. B680 slots take (attachTo/a7,
// ..., 0, b1, 0) shapes, B9C0's are B680's shifted by one a-slot with the
// extra a10 in the zero positions: (a7/a8, ..., a10, b1, b2[, 0]). Near:
// spawn the near family and blend; continue. Far: spawn the far family and
// return 0. Exhausted: return 1.
// ---------------------------------------------------------------------------
namespace {

// B680 args: manager(a1), farManager(a2, unused), list(a3), baseOff(a4),
//            scale(a5), attachTo(a6), wind(a7), pos(a8), spawnBase(a9),
//            spawnAttach(a10), name(a11).
// B9C0 args: manager(a1), farManager(a2, unused), list(a3), base1(a4),
//            base2(a5), scale(a6), wind(a7), vec2(a8), pos(a9),
//            slotArg(a10), spawnBase(a11), spawnAddTo(a12), spawnAttach(a13),
//            name(a14). The two-base flavour has no attachTo/scale-bit slot:
//            its transform's first arg is a7, so the wrapper passes null as
//            argA6 (never read on that side).
template <bool kTwoBases>
char CollideEffectsImpl(void* manager, const EffectList* list,
                        int32_t base1Off, int32_t base2Off, float scale,
                        void* argA6, const float* argA7, const float* argA8,
                        float* pos, int32_t slotArg, int32_t spawnBase,
                        EffectPtrVector* spawnAddTo, void* spawnAttach,
                        const char* name)
{
    for (EffectInstance* e = list->begin; e != list->end;
         e = reinterpret_cast<EffectInstance*>(reinterpret_cast<char*>(e) + 0x18)) {
        const EffectParams* p = e->params;
        const uint32_t flags = p->flags;
        const int32_t b1 = e->baseOn1 ? base1Off + e->baseOff1 : 0;
        const int32_t b2 = kTwoBases ? (e->baseOn2 ? base2Off + e->baseOff2 : 0) : 0;
        void* obj = e->object;

        if ((flags & 2) != 0) {
            // Reflect: slot 4 writes the plane; push pos out when the plane
            // sits in front (t = -0.0 - dot > 0).
            float plane[3] = {0.0f, 0.0f, 0.0f};
            bool hit;
            if (kTwoBases) {
                hit = ts3::CallGameVirtual<bool, 4>(obj, argA8, argA8, plane, slotArg,
                                                    b1, b2, 0) != 0;
            } else {
                hit = ts3::CallGameVirtual<bool, 4>(obj, argA7, argA7, plane, 0,
                                                    b1, 0, 0) != 0;
            }
            if (hit) {
                const float dot = (pos[2] * plane[2] + pos[1] * plane[1]) + pos[0] * plane[0];
                const float t = NegZero() - dot;
                if (t > 0.0f) {
                    pos[0] += plane[0] * t;
                    pos[1] += plane[1] * t;
                    pos[2] += plane[2] * t;
                }
            }
            continue;
        }
        if ((flags & 4) != 0) {
            // Transform the position in place.
            if (kTwoBases) {
                ts3::CallGameVirtual<void, 5>(obj, argA7, argA8, pos, slotArg, b1, b2);
            } else {
                ts3::CallGameVirtual<void, 5>(obj, argA6, argA7, pos, 0, b1, 0);
            }
            continue;
        }
        if ((flags & 1) == 0) {
            // No collide bit: discarded reflect, optionally gated.
            if (kTwoBases) {
                if ((flags & 0x10) == 0 && (flags & 0x20) != 0) {
                    float discard[3] = {0.0f, 0.0f, 0.0f};
                    float plane[3] = {0.0f, 0.0f, 0.0f};
                    ts3::CallGameVirtual<void, 4>(obj, argA8, discard, plane, slotArg,
                                                  b1, b2, 0);
                }
            } else if ((flags & 0x10) != 0) {
                // Gate first: slot 3 returns a double; proceed only when the
                // UNSCALED weight exceeds it (fld weight; fcomip; jbe next).
                const double gate = ts3::CallGameVirtual<double, 3>(obj, argA7, 0, 0);
                p = e->params;
                if (!(static_cast<double>(p->weight) > gate)) {
                    continue;
                }
                float discard[3] = {0.0f, 0.0f, 0.0f};
                float plane[3] = {0.0f, 0.0f, 0.0f};
                ts3::CallGameVirtual<void, 4>(obj, argA7, discard, plane, 0, b1, 0, 0);
            }
            continue;
        }

        // Collide: slot 2, first stack arg = weight * scale.
        float plane[3] = {0.0f, 0.0f, 0.0f};
        const float weight = p->weight * scale;
        bool hit;
        if (kTwoBases) {
            hit = ts3::CallGameVirtual<bool, 2>(obj, weight, argA7, argA8, plane,
                                                b1, b2) != 0;
        } else {
            hit = ts3::CallGameVirtual<bool, 2>(obj, weight, argA6, argA7, plane,
                                                b1, 0) != 0;
        }
        if (!hit) {
            continue;
        }

        // Hit: fade picks near/far. fade == 0 -> near; fade == 1 -> far
        // (ordered equality via the lahf/test 0x44/jnp idiom); otherwise
        // fld fade; fcomip; jae far, i.e. far iff fade >= random (ordered),
        // near otherwise (incl. unordered). The RNG is consumed only here.
        p = e->params;
        const float fade = p->fade;
        bool nearHit;
        if (fade == 0.0f) {
            nearHit = true;
        } else if (fade == 1.0f) {
            nearHit = false;
        } else {
            const double rnd = RandomFloat();
            nearHit = !(static_cast<double>(fade) >= rnd);
        }
        p = e->params;
        void* familyBegin = nearHit ? p->nearBegin : p->farBegin;
        void* familyEnd = nearHit ? p->nearEnd : p->farEnd;
        if (familyBegin != familyEnd) {
            // Both walkers spawn through a1 (a2 is never read). B680's wind
            // arg is a7 and addTo null; B9C0's is a8 with addTo a12.
            SpawnFamilyEffects(manager, reinterpret_cast<const EffectInstance**>(familyBegin),
                               kTwoBases ? argA8 : argA7, spawnBase,
                               kTwoBases ? spawnAddTo : nullptr,
                               spawnAttach, name);
        }
        if (!nearHit) {
            return 0;
        }
        // Refraction blend: k1 = stiffness + 1.0f, k2 = 1.0f - spread.
        const float dot = (pos[2] * plane[2] + pos[1] * plane[1]) + pos[0] * plane[0];
        p = e->params;
        const float k1 = p->stiffness + 1.0f;
        const float k2 = 1.0f - p->spread;
        for (int i = 0; i < 3; ++i) {
            const float t = pos[i] - plane[i] * dot;
            pos[i] = pos[i] - (plane[i] * dot * k1 + t * k2);
        }
    }
    return 1;
}

} // namespace

// 0x0092B680 — CollideEffects (cdecl, 11 args).
char CollideEffects(void* manager, void* /*farManager a2 unused*/, const EffectList* list,
                    int32_t baseOffset, float scale, void* attachTo, const float* wind,
                    float position[3], int32_t spawnBaseOffset, void* spawnAttachTo,
                    const char* name)
{
    return CollideEffectsImpl<false>(manager, list, baseOffset, 0, scale,
                                     attachTo, wind, wind, position, 0,
                                     spawnBaseOffset, nullptr, spawnAttachTo, name);
}

// 0x0092B9C0 — CollideEffectsTwoBases (cdecl, 14 args). a2 is never read.
// a7/a8 feed the slots, a9 is the position the walker rewrites, a10 is the
// extra slot-4/5 argument, a11..a14 feed the spawn (set-base / addTo /
// attachTo / name).
char CollideEffectsTwoBases(void* manager, void* /*farManager a2 unused*/, const EffectList* list,
                            int32_t base1, int32_t base2, float scale,
                            const float* wind, const float* vec2, float position[3],
                            int32_t slotArg, int32_t spawnBaseOffset,
                            EffectPtrVector* spawnAddTo, void* spawnAttachTo,
                            const char* name)
{
    return CollideEffectsImpl<true>(manager, list, base1, base2, scale,
                                    nullptr, wind, vec2, position, slotArg,
                                    spawnBaseOffset, spawnAddTo, spawnAttachTo, name);
}

// ---------------------------------------------------------------------------
// 0x0092D230 — StepInstanceEffects (thiscall, ret 8; a2 unused, a3 = counters)
//
// Advances the effect clock in <= 1/15 s steps over the remaining time at
// (this+0x40)->+0x30. Each step walks the 0x58-stride instance vector at
// this+0x220/0x224, bumps counters, asks each object to be alive (slot 4) and
// emits through slot 5 (weight*step, density, counters). Density = this+0x54
// unless record flags 0x40 (then 1.0, or 0.0 with 0x80). Early-out returns the
// time pointer; the loop path's eax is junk (returns null here).
// ---------------------------------------------------------------------------
float* VisualEffect::StepInstanceEffects(int32_t /*unusedA2*/, EmitCounters* counters)
{
    VisualEffect* self = this;
    float remaining = self->time->total;
    if (!(remaining > 0.0f)) {
        return reinterpret_cast<float*>(self->time);
    }
    for (;;) {
        // comiss step, remaining; ja -> &remaining else &step: min(step, remaining)
        const float step = (ClockStep() > remaining) ? remaining : ClockStep();
        InstanceRecord* const stepEnd = self->instEnd;
        for (InstanceRecord* it = self->instBegin; it != stepEnd;
             it = reinterpret_cast<InstanceRecord*>(reinterpret_cast<char*>(it) + 0x58)) {
            ++counters->processed;
            void* obj = it->object;
            if (ts3::CallGameVirtual<bool, 4>(obj) != 0) {
                ++counters->emitted;
                const uint32_t f = it->flags;
                float density;
                if ((f & 0x40u) != 0) {
                    density = ((f & 0x80u) != 0) ? 0.0f : 1.0f;
                } else {
                    density = X87Copy(self->density);
                }
                ts3::CallGameVirtual<void, 5>(obj, it->weight * step, density, counters);
            }
        }
        remaining -= step;
        if (!(remaining > 0.0f)) {
            break;
        }
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// 0x0092DE40 — UpdatePlaneCulling (thiscall, ret 4, 1 stack arg)
//
// Distance test of the plane (pt[0..2], n[3..5]) against the geometry radius
// at (this+0x40)->+0x14, with the reference point at this+0x64..0x6C plus
// [0x70] * dir at +0x9C..0xA4 (rotated by the 3x3 at +0x74..0x94 when the
// byte at +0x60 has bit 1). The compared distance is the sqrt of the squared
// residual components, not the raw projection. Geometry flags 0x40000 /
// 0x80000 combine the cull-side mask at this+0x274 (+0xB4/+0xB8 bit 0) with
// the current visible flag (bit 0x10 of +0x44); transitions set/clear it and
// fire vtable slots 2/3 (arg 0) subject to the 0x200/0x400 gate. The returned
// eax differs per path and is replicated here.
// ---------------------------------------------------------------------------
int32_t VisualEffect::UpdatePlaneCulling(const float* plane)
{
    VisualEffect* self = this;
    const float* base = reinterpret_cast<const float*>(self);
    float d0 = base[0x9C / 4];
    float d1 = base[0xA0 / 4];
    float d2 = base[0xA4 / 4];
    if ((reinterpret_cast<const uint8_t*>(self)[0x60] & 2u) != 0) {
        // r_i = (m_i2*d2 + m_i1*d1) + m_i0*d0, operand order per the asm.
        const float r0 = (base[0x8C / 4] * d2 + base[0x80 / 4] * d1) + base[0x74 / 4] * d0;
        const float r1 = (base[0x90 / 4] * d2 + base[0x84 / 4] * d1) + base[0x78 / 4] * d0;
        const float r2 = (base[0x94 / 4] * d2 + base[0x88 / 4] * d1) + base[0x7C / 4] * d0;
        d0 = r0;
        d1 = r1;
        d2 = r2;
    }
    const float ref0 = base[0x64 / 4] + base[0x70 / 4] * d0;
    const float ref1 = base[0x68 / 4] + base[0x70 / 4] * d1;
    const float ref2 = base[0x6C / 4] + base[0x70 / 4] * d2;
    const float n0 = plane[3];
    const float n1 = plane[4];
    const float n2 = plane[5];
    const float t0 = plane[0] - ref0;
    const float t1 = plane[1] - ref1;
    const float t2 = plane[2] - ref2;
    const float num = (n2 * t2 + n1 * t1) + n0 * t0;
    const float den = (n2 * n2 + n1 * n1) + n0 * n0;
    const float distRaw = NegZero() - num / den;
    const float comp2 = n2 * distRaw + t2;
    const float comp1 = n1 * distRaw + t1;
    const float comp0 = n0 * distRaw + t0;
    const float dist = static_cast<float>(std::sqrt(
        (static_cast<double>(comp2) * comp2 + static_cast<double>(comp1) * comp1) +
        static_cast<double>(comp0) * comp0));

    const uint32_t flags = self->cullFlags;
    const uint32_t oldVis = (flags >> 4) & 1u;
    const void* geometry = self->time;
    const uint32_t gflags = *static_cast<const uint32_t*>(geometry);
    const float radius =
        *reinterpret_cast<const float*>(static_cast<const uint8_t*>(geometry) + 0x14);
    const bool radiusHit = radius > dist; // comiss radius, dist; strict greater

    uint32_t nextVis = oldVis;
    if ((gflags & 0x40000u) != 0) {
        const CullSide* side = self->side;
        const bool sideBit = ((side->maskB & side->maskA) & 1u) != 0;
        if (sideBit) {
            nextVis = radiusHit ? 1u : 0u;
        }
    } else if ((gflags & 0x80000u) != 0) {
        if (radiusHit) {
            const CullSide* side = self->side;
            const bool sideBit = ((side->maskB & side->maskA) & 1u) != 0;
            if (sideBit) {
                nextVis = oldVis ^ 1u;
            }
        }
    } else {
        nextVis = radiusHit ? 1u : 0u;
    }

    if (nextVis == oldVis) {
        // No transition: eax = (flags >> 4) with only AL masked to bit 0.
        const uint32_t shifted = flags >> 4;
        return static_cast<int32_t>((shifted & 0xFFFFFF00u) | (shifted & 1u));
    }
    if (nextVis != 0) {
        // Enabling: set bit 0x10, fire slot 2 unless bit9 && !bit10.
        self->cullFlags = flags | 0x10u;
        const uint32_t f = self->cullFlags;
        const bool blocked = ((f >> 9) & 1u) != 0 && ((f >> 10) & 1u) == 0;
        if (!blocked) {
            return ts3::CallGameVirtual<int32_t, 2>(self, 0);
        }
        return static_cast<int32_t>((flags | 0x10u) >> 10);
    }
    // Disabling: clear bit 0x10, fire slot 3 iff bit9 && !bit10. The two skip
    // returns differ: !bit9 keeps the raw updated flags, bit9&&bit10 shifts.
    self->cullFlags = flags & ~0x10u;
    const uint32_t f = self->cullFlags;
    const bool bit9 = ((f >> 9) & 1u) != 0;
    const bool bit10 = ((f >> 10) & 1u) != 0;
    if (bit9 && !bit10) {
        return ts3::CallGameVirtual<int32_t, 3>(self, 0);
    }
    if (bit9) {
        return static_cast<int32_t>((flags & ~0x10u) >> 10);
    }
    return static_cast<int32_t>(flags & ~0x10u);
}

// ---------------------------------------------------------------------------
// 0x0092FA00 — VisualEffect::Construct (thiscall ctor, eax = this, size 0x27D)
//
// Writes the final vtables, the empty-string name trio (begin==end at the
// static .data 0x11C7E54 so the dtor's free check skips), the -1 sentinels,
// the 8 transform records (stride 0x38) from the runtime globals, and the
// trailing corner points. Every record scale is 1.0f (the original loads
// 0x107A538 once into xmm0 and never reloads it for the 8 stores). +0x2C is
// never written (a hole the ctor leaves untouched).
// ---------------------------------------------------------------------------
VisualEffect* VisualEffect::Construct()
{
    VisualEffect* s = this;
    auto* bytes = reinterpret_cast<uint8_t*>(s);
    auto* words = reinterpret_cast<uint32_t*>(s);

    // Final vtables (the original writes intermediates first, then these).
    words[0x00 / 4] = 0x01042570u;
    words[0x04 / 4] = 0x01042528u;
    words[0x08 / 4] = 0x01042520u;
    words[0x0C / 4] = 0;
    words[0x10 / 4] = 0;
    words[0x14 / 4] = 0;

    // Name trio -> static empty string (begin==end, cap one past; free skipped).
    words[0x18 / 4] = 0x011C7E54u;
    words[0x1C / 4] = 0x011C7E54u;
    words[0x20 / 4] = 0x011C7E55u;
    words[0x24 / 4] = 0x01042504u; // "Swarm/cVisualEffect/mName" tag
    words[0x28 / 4] = 0;           // 0.0f
    // +0x2C..0x2F intentionally left untouched (padding hole).
    words[0x30 / 4] = 0xFFFFFFFFu;
    words[0x34 / 4] = 0xFFFFFFFFu;
    words[0x38 / 4] = 0xFFFFFFFFu;
    words[0x3C / 4] = 0;
    words[0x40 / 4] = 0;
    words[0x44 / 4] = 0;
    // Runtime globals for +0x48..0x4C (file zeros in exehost, filled live).
    words[0x48 / 4] = ts3::ImageWord<uint32_t>(0x011DC0F0);
    words[0x4C / 4] = ts3::ImageWord<uint32_t>(0x011DC0F4);
    words[0x50 / 4] = 0;
    words[0x54 / 4] = 0; // 0.0f density
    words[0x58 / 4] = 0;
    words[0x5C / 4] = 0;

    // 8 transform records at 0x60, stride 0x38: type/flags dword 0, position
    // from globals 0x11DC0F8/FC/100, scale 1.0f, matrix from 0x11DC110..0x11DC130.
    for (int i = 0; i < 8; ++i) {
        auto* rec = reinterpret_cast<uint32_t*>(bytes + 0x60 + i * 0x38);
        rec[0] = 0;
        rec[1] = ts3::ImageWord<uint32_t>(0x011DC0F8);
        rec[2] = ts3::ImageWord<uint32_t>(0x011DC0FC);
        rec[3] = ts3::ImageWord<uint32_t>(0x011DC100);
        rec[4] = ts3::ImageWord<uint32_t>(0x0107A538); // 1.0f for all 8 records
        for (int j = 0; j < 9; ++j) {
            rec[5 + j] = ts3::ImageWord<uint32_t>(0x011DC110 + static_cast<uintptr_t>(j) * 4);
        }
    }

    // Instance vector at 0x220..0x228 (begin/end/cap).
    words[0x220 / 4] = 0;
    words[0x224 / 4] = 0;
    words[0x228 / 4] = 0;
    words[0x22C / 4] = 0x010424E0u; // "Swarm/VisualEffect/ComponentRecs"
    words[0x230 / 4] = 0;
    words[0x234 / 4] = 0;
    words[0x238 / 4] = 0;
    words[0x23C / 4] = 0;

    // -1.0f x3 at 0x240..0x248 (loaded from 0xFAA210).
    words[0x240 / 4] = ts3::ImageWord<uint32_t>(0x00FAA210);
    words[0x244 / 4] = ts3::ImageWord<uint32_t>(0x00FAA210);
    words[0x248 / 4] = ts3::ImageWord<uint32_t>(0x00FAA210);
    // Corner points from runtime globals.
    words[0x24C / 4] = ts3::ImageWord<uint32_t>(0x011DC0F8);
    words[0x250 / 4] = ts3::ImageWord<uint32_t>(0x011DC0FC);
    words[0x254 / 4] = ts3::ImageWord<uint32_t>(0x011DC100);
    words[0x258 / 4] = ts3::ImageWord<uint32_t>(0x011DC0F8);
    words[0x25C / 4] = ts3::ImageWord<uint32_t>(0x011DC0FC);
    words[0x260 / 4] = ts3::ImageWord<uint32_t>(0x011DC100);
    words[0x264 / 4] = ts3::ImageWord<uint32_t>(0x011DC104);
    words[0x268 / 4] = ts3::ImageWord<uint32_t>(0x011DC108);
    words[0x26C / 4] = ts3::ImageWord<uint32_t>(0x011DC10C);
    words[0x270 / 4] = 0;
    words[0x274 / 4] = 0;
    words[0x278 / 4] = 0;
    bytes[0x27C] = 0;
    return s;
}

// ---------------------------------------------------------------------------
// 0x009302F0 — VisualEffect::Destruct (thiscall, ret)
//
// Releases the two owned objects at +0x238 then +0x230 (vtable slot 2), tears
// down the instance vector via 0x0091A440, frees its buffer via 0x004E3F70,
// frees the name when heap-owned (span = nameCap(+0x20) - nameBegin(+0x18),
// signed > 1, begin non-null — the original compares against the CAP, not the
// end), then resets the vtables. Returns this + 4 (null for a null this).
// ---------------------------------------------------------------------------
uint8_t* VisualEffect::Destruct()
{
    VisualEffect* s = this;
    if (!s) {
        return nullptr;
    }
    auto* words = reinterpret_cast<uint32_t*>(s);
    void* obj238 = s->obj238;
    if (obj238) {
        ts3::CallGameVirtual<void, 2>(obj238);
    }
    void* obj230 = s->obj230;
    if (obj230) {
        ts3::CallGameVirtual<void, 2>(obj230);
    }
    DestroyInstanceVector(&s->instBegin, s->instBegin, s->instEnd);
    void* begin = s->instBegin;
    if (begin) {
        GameFree(begin);
    }
    // Name string: free when heap-owned (cap - begin > 1, begin != null).
    const auto nb = reinterpret_cast<intptr_t>(s->nameBegin);
    const auto nc = reinterpret_cast<intptr_t>(s->nameCap);
    if ((nc - nb) > 1 && nb != 0) {
        GameFree(const_cast<char*>(s->nameBegin));
    }
    // Reset the vtables: this+4 then this+8 twice.
    words[1] = 0x0103B988u;
    words[2] = 0x0103B980u;
    words[2] = 0x0107396Cu;
    return reinterpret_cast<uint8_t*>(s) + 4;
}

} // namespace ts3::speedtree
