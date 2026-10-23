// Unit clu_848d20_Route (Route): the cost-chooser leaf 0x00848D20 and the thiscall
// entry 0x0084AF90 that feeds it from the Route tuner object. Semantics from the
// TS3W.exe 1.67.2 listings (the original addresses are noted on each function).
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <cmath>
#include <cstdint>
#include <cstring>

#include "ts3/route/clu_848d20_Route.h"

namespace ts3::route {
namespace {

// The float globals, each read from the exact address the original reads. On x86 the
// game's image is mapped there (the .data weights are writable tunables, so the x86
// build must not fold them into literals); x64 has no game image and uses the values
// TS3W.exe 1.67.2 ships.
#if defined(_M_IX86)
inline float LoadGlobal(float* cell)
{
    float value;
    std::memcpy(&value, cell, sizeof value);
    return value;
}
#define TS3_ROUTE_GLOBAL(addr) LoadGlobal(reinterpret_cast<float*>(static_cast<uintptr_t>(addr)))
inline float MaxFloatCell() { return TS3_ROUTE_GLOBAL(0x01026FD0u); }  // FLT_MAX
inline float EpsilonCell() { return TS3_ROUTE_GLOBAL(0x0106D6CCu); }   // 0.01f
inline float OneCell() { return TS3_ROUTE_GLOBAL(0x0107A538u); }       // 1.0f
inline float WeightNearCell() { return TS3_ROUTE_GLOBAL(0x01166494u); }  // 1.0f (W4)
inline float WeightDeltaCell() { return TS3_ROUTE_GLOBAL(0x01166498u); }  // 100.0f (W2)
inline float WeightBaseCell() { return TS3_ROUTE_GLOBAL(0x011D9270u); }   // 0.0f (W3)
inline float WeightIterCell() { return TS3_ROUTE_GLOBAL(0x011D9274u); }   // 0.0f (W1)
#else
inline float MaxFloatCell() { return 3.402823466e+38f; }
inline float EpsilonCell() { return 0.01f; }
inline float OneCell() { return 1.0f; }
inline float WeightNearCell() { return 1.0f; }
inline float WeightDeltaCell() { return 100.0f; }
inline float WeightBaseCell() { return 0.0f; }
inline float WeightIterCell() { return 0.0f; }
#endif

// The refcount dword at +4 of the 32-bit target pointer, bumped/released with the
// Interlocked ops the original uses (lock xadd / lock dec). The pointer is widened
// the way the game holds it: a 32-bit address.
inline volatile LONG* RefCountAt(int32_t target)
{
    const uintptr_t address =
        static_cast<uintptr_t>(static_cast<uint32_t>(target)) + 4;
    return reinterpret_cast<volatile LONG*>(address);
}

inline void* TargetObject(int32_t target)
{
    return reinterpret_cast<void*>(static_cast<uintptr_t>(static_cast<uint32_t>(target)));
}

}  // namespace

// TS3W.exe 0x00848D20 - one candidate per iteration, i = 0..iter inclusive. The
// iteration count is (num / (step * lo)) truncated toward zero with a floor of 1,
// except mode 4 which runs the single iteration i = 0 (iter 0, loop still runs once).
int32_t __cdecl SelectValue848D20(int32_t mode, int32_t unused, float num, float step,
                                  float bonus, float extra, int32_t arrA, int32_t arrB,
                                  uint32_t count, uint32_t cutoff, float* inout, float lo,
                                  float hi, int8_t flag)
{
    (void)unused;  // a2 is passed at the 84AF90 call (the target pointer) and never read
    // The original loads the 0.01f cell into a frame slot once, before the loop.
    const float epsilon = EpsilonCell();
    const float v15 = *inout;  // read once for the cost, and once more for the clamp
    int32_t iter;
    if (mode == 4) {
        iter = 0;
    } else {
        const int32_t raw = static_cast<int32_t>(num / (step * lo));
        iter = raw <= 1 ? 1 : raw;
    }
    float bestVal = *inout;
    float bestCost = MaxFloatCell();
    int32_t bestIdx = 0;
    const uint8_t* arrABytes =
        reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(static_cast<uint32_t>(arrA)));
    const uint8_t* arrBBytes =
        reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(static_cast<uint32_t>(arrB)));
    // The raw 32-bit a8 - a7 the vectorized quads add to each arrA address (the
    // original keeps it in a frame slot; it is loop-invariant).
    const int32_t delta = static_cast<int32_t>(static_cast<uint32_t>(arrB) -
                                               static_cast<uint32_t>(arrA));
    int32_t i = 0;
    if (iter >= 0) {  // the loop is entered only when iter >= 0 (cvttss2si can give INT_MIN)
        for (;;) {
            // Candidate offset: the flag selects the a6 column, mode 2 adds a5.
            float offset = flag ? extra : 0.0f;
            if (mode == 2) {
                offset = offset + bonus;
                if (count > cutoff && arrA != 0 && arrB != 0) {
                    uint32_t j = 1;
                    if (static_cast<int32_t>(count) >= 4) {
                        // 4-wide quads: (a7[j] + (flag ? a8[j] : 0)) + running, one quad
                        // at a time, starting at index 1 (the a7[0]/a8[0] column is
                        // never summed - see the notes on the EA loop bounds).
                        const float* va = reinterpret_cast<const float*>(arrABytes + 8);
                        const float* vb = reinterpret_cast<const float*>(arrBBytes + 16);
                        uint32_t quads = count >> 2;
                        j = 4 * quads + 1;
                        do {
                            const float b1 = flag ? *(vb - 3) : 0.0f;
                            const float t = (va[-1] + b1) + offset;
                            const float b2 = flag ? *reinterpret_cast<const float*>(
                                                        reinterpret_cast<const uint8_t*>(va) + delta)
                                                  : 0.0f;
                            const float t2 = (b2 + *va) + t;
                            const float b3 = flag ? *(vb - 1) : 0.0f;
                            const float t3 = (va[1] + b3) + t2;
                            const float b4 = flag ? *vb : 0.0f;
                            offset = (va[2] + b4) + t3;
                            va += 4;
                            vb += 4;
                            --quads;
                        } while (quads != 0);
                    }
                    if (j <= count) {  // tail over the rest (j = 4*(count>>2)+1 .. count)
                        const float* tail = reinterpret_cast<const float*>(arrABytes + 4 * j);
                        uint32_t n = count - j + 1;
                        do {
                            const float b = flag ? *reinterpret_cast<const float*>(
                                                       reinterpret_cast<const uint8_t*>(tail) + delta)
                                                 : 0.0f;
                            const float a = *tail;
                            ++tail;
                            offset = (a + b) + offset;
                            --n;
                        } while (n != 0);
                    }
                }
            } else if (mode == 3) {
                if (count > cutoff && arrA != 0 && arrB != 0) {
                    uint32_t j = 1;
                    if (static_cast<int32_t>(count) >= 4) {
                        // 4-wide quads, each element gated separately: the a8 column
                        // only when flag is set, the a7 column only past the cutoff
                        // (index > cutoff, unsigned).
                        const uint32_t lastQuadStart = count - 3;
                        const float* va = reinterpret_cast<const float*>(arrABytes + 8);
                        const float* vb = reinterpret_cast<const float*>(arrBBytes + 16);
                        uint32_t idx = 3;  // the a7 index of the 2nd element of the quad
                        for (;;) {
                            if (flag) offset = *(vb - 3) + offset;
                            if (j > cutoff) offset = *(va - 1) + offset;
                            if (flag)
                                offset = *reinterpret_cast<const float*>(
                                             reinterpret_cast<const uint8_t*>(va) + delta) +
                                         offset;
                            if (idx - 1 > cutoff) offset = offset + *va;
                            if (flag) offset = *(vb - 1) + offset;
                            if (idx > cutoff) offset = va[1] + offset;
                            if (flag) offset = *vb + offset;
                            if (idx + 1 > cutoff) offset = offset + va[2];
                            j += 4;
                            va += 4;
                            vb += 4;
                            idx += 4;
                            if (j > lastQuadStart) break;
                        }
                    }
                    if (j <= count) {  // tail, same per-element gates
                        const float* tail = reinterpret_cast<const float*>(arrABytes + 4 * j);
                        do {
                            if (flag)
                                offset = *reinterpret_cast<const float*>(
                                             reinterpret_cast<const uint8_t*>(tail) + delta) +
                                         offset;
                            if (j > cutoff) offset = offset + *tail;
                            ++j;
                            ++tail;
                        } while (j <= count);
                    }
                } else if (cutoff != 0 && arrA != 0 && arrB != 0) {
                    // The small-count path: only a8 (the flag column of the second
                    // array) is summed - first a8[cutoff .. count+1] descending when
                    // the cutoff is past the count (NO flag gate here), then, with the
                    // flag set, a8[count .. 1] descending.
                    if (cutoff > count) {
                        uint32_t co = cutoff;
                        do {
                            const float a = *reinterpret_cast<const float*>(
                                arrBBytes + 4 * co);
                            --co;
                            offset = a + offset;
                        } while (co > count);
                    }
                    if (flag) {
                        uint32_t j = count;
                        while (j != 0) {
                            --j;
                            offset = *reinterpret_cast<const float*>(arrBBytes + 4 * j + 4) +
                                     offset;
                        }
                    }
                }
            }
            const float v47 = static_cast<float>(i) * step + offset;
            // The original picks the denominator by the SIGN BIT of (0.01f - v47):
            // negative -> v47, else 0.01f - a mask select, exact for -0.0 and NaN too.
            uint32_t diffBits;
            const float diff = epsilon - v47;
            std::memcpy(&diffBits, &diff, sizeof diffBits);
            const float denom = (diffBits & 0x80000000u) != 0 ? v47 : epsilon;
            const float ratio = num / denom;
            // Clamp violation: the hi term first, then the lo term OVERWRITES it
            // (neither is a max - the original's separate stores).
            float v50 = 0.0f;
            if (ratio > hi) v50 = ratio - hi;
            if (lo > ratio) v50 = lo - ratio;
            const float cost = static_cast<float>(i) * WeightIterCell() +
                               v50 * WeightDeltaCell() +
                               std::fabs(ratio - v15) * WeightBaseCell() +
                               std::fabs(ratio - OneCell()) * WeightNearCell();
            if (bestCost > cost) {  // strict comiss/jbe: equal costs keep the earlier i
                bestCost = cost;
                bestVal = ratio;
                bestIdx = i;
            }
            if (++i > iter) break;
        }
    }
    // Select-then-clamp against the original read. The original compiles the first
    // select to `comiss pick, lo; jbe takeLo` - jbe is taken on UNORDERED too, so lo
    // wins whenever !(pick > lo) holds (equal zeros, NaN lo/NaN pick) - and the second
    // to `comiss sel, hi; ja takeHi` (ordered greater only). Written through a volatile
    // so MSVC cannot fuse the selects into maxss/minss whose operand order changes the
    // stored bits on -0.0/+0.0 and NaN.
    volatile float sel = bestVal;
    if (!(sel > lo)) sel = lo;
    if (sel > hi) sel = hi;
    *inout = sel;
    return bestIdx;
}

// TS3W.exe 0x0084AF90 - every offset below is the game's own.
int32_t RouteTuner::Update84AF90(int32_t mode, int8_t force)
{
    // 0x00C93970: return this->canvas's dword at +8 (the ref target), via the bridge.
    int32_t target = CallGameMethod<int32_t, 0x00C93970>(canvas);
    if (target != 0) InterlockedIncrement(RefCountAt(target));
    const int32_t v9 = field124;   // +0x7C
    const int32_t v10 = field132;  // +0x84
    // The walker object at +0xC4, vtable slot 4: a 64-bit id in edx:eax, no stack args.
    const int64_t id = CallGameVirtual<int64_t, 4>(walker);
    if (target != 0) InterlockedIncrement(RefCountAt(target));
    const int32_t idLo = static_cast<int32_t>(static_cast<uint64_t>(id) & 0xFFFFFFFFu);
    const int32_t idHi = static_cast<int32_t>(static_cast<uint64_t>(id) >> 32);
    // 0x00849810, cdecl, six args pushed (1, v10, v9, idHi, idLo, target); the result
    // comes back in ST0 and the original stores it with one fstp dword - declared
    // float here so the conversion happens exactly once, the same way.
    const float base =
        CallGameFunction<float, 0x00849810>(target, idLo, idHi, v9, v10, 1);
    const int32_t r = SelectValue848D20(mode, target, base, step, bonus, extra, arrA,
                                        arrB, count, cutoff, &chosen, lo, hi, flag);
    if (force != 0 ||
        r != static_cast<int32_t>(static_cast<uint32_t>(lastIndex) -
                                  static_cast<uint32_t>(clear372))) {
        lastIndex = r;
        clear372 = 0;
        clear368 = 0;
    }
    if (target != 0) {
        // The original decrements with lock dec and, on reaching 0, writes 1 back
        // before calling vtable slot 0 (the release) with 1.
        volatile LONG* rc = RefCountAt(target);
        if (InterlockedDecrement(rc) == 0) {
            InterlockedExchange(rc, 1);
            CallGameVirtual<void, 0>(TargetObject(target), 1);
        }
    }
    return r;
}

// The x86 object layout this unit's entry point runs on.
TS3_X86_OFFSET(RouteTuner, canvas, 0x14);
TS3_X86_OFFSET(RouteTuner, field124, 0x7C);
TS3_X86_OFFSET(RouteTuner, field132, 0x84);
TS3_X86_OFFSET(RouteTuner, walker, 0xC4);
TS3_X86_OFFSET(RouteTuner, step, 0x12C);
TS3_X86_OFFSET(RouteTuner, bonus, 0x130);
TS3_X86_OFFSET(RouteTuner, extra, 0x134);
TS3_X86_OFFSET(RouteTuner, arrA, 0x13C);
TS3_X86_OFFSET(RouteTuner, arrB, 0x140);
TS3_X86_OFFSET(RouteTuner, lo, 0x15C);
TS3_X86_OFFSET(RouteTuner, hi, 0x160);
TS3_X86_OFFSET(RouteTuner, chosen, 0x164);
TS3_X86_OFFSET(RouteTuner, lastIndex, 0x16C);
TS3_X86_OFFSET(RouteTuner, clear368, 0x170);
TS3_X86_OFFSET(RouteTuner, clear372, 0x174);
TS3_X86_OFFSET(RouteTuner, cutoff, 0x248);
TS3_X86_OFFSET(RouteTuner, count, 0x24C);
TS3_X86_OFFSET(RouteTuner, flag, 0x253);
TS3_X86_SIZE(RouteTuner, 0x254);

}  // namespace ts3::route
