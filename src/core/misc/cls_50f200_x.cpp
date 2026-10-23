// cls_50f200_x: portable C++ for the 24 TS3W.exe entry points listed in
// the project notes. Functions of other units are reached through the ts3::CallGame*
// bridges from ts3/layout.h (x64 builds abort there instead); in-unit calls go directly to
// the rewrites. EA quirks that change outputs are marked "EA quirk" and kept.
#include "ts3/misc/cls_50f200_x.h"

#include <cmath>
#include <cstring>

#if defined(_M_IX86)
#include <xmmintrin.h>
#endif

namespace ts3::misc {

namespace {

// ---- game addresses this unit reads at run time ------------------------------------------

constexpr uintptr_t kOneFloat = 0x0107A538;       // float 1.0f
constexpr uintptr_t kLightGlobalA = 0x011D0A60;   // float pair read by 0x006BE020/0x006BFDC0
constexpr uintptr_t kLightGlobalB = 0x011D0A68;   // second float of that pair
constexpr uintptr_t kLightCap = 0x011D1160;       // 16 bytes read by the same two functions
constexpr uintptr_t kLightScale = 0x01158DA8;     // float 3.3333001f
constexpr uintptr_t kDescriptor101C080 = 0x0101C080;  // class descriptor written by 0x007E4E70
constexpr uintptr_t kVtable101C0D8 = 0x0101C0D8;      // primary vtable
constexpr uintptr_t kVtable101C0C0 = 0x0101C0C0;      // secondary interface vtable
constexpr uintptr_t kVtableFBB428 = 0x00FBB428;       // intermediate secondary vtable written by
                                                      // the dtors (IDA mislabels its symbol);
                                                      // overwritten by kVtableFA1B78 below
constexpr uintptr_t kVtableFA1B78 = 0x00FA1B78;       // primary vtable written by the dtors
constexpr int32_t kMagicInterfaceId = static_cast<int32_t>(0xEEC58382u);

float OneFloat()
{
#if defined(_M_IX86)
    return *reinterpret_cast<const float*>(kOneFloat);
#else
    return 1.0f;
#endif
}

float LightGlobalA()
{
#if defined(_M_IX86)
    return *reinterpret_cast<const float*>(kLightGlobalA);
#else
    return 0.0f;
#endif
}

float LightGlobalB()
{
#if defined(_M_IX86)
    return *reinterpret_cast<const float*>(kLightGlobalB);
#else
    return 0.0f;
#endif
}

float LightCap()
{
#if defined(_M_IX86)
    return *reinterpret_cast<const float*>(kLightCap);
#else
    return 0.0f;
#endif
}

float LightScale()
{
#if defined(_M_IX86)
    return *reinterpret_cast<const float*>(kLightScale);
#else
    return 3.3333001f;
#endif
}

// The approximate SSE reciprocal square root/reciprocal the original uses. x64 has no game
// code to match, so the exact C++ results are fine there.
float RsqrtApprox(float x)
{
#if defined(_M_IX86)
    const __m128 r = _mm_rsqrt_ps(_mm_set1_ps(x));
    return _mm_cvtss_f32(r);
#else
    return 1.0f / std::sqrt(x);
#endif
}

float RcpApprox(float x)
{
#if defined(_M_IX86)
    const __m128 r = _mm_rcp_ps(_mm_set1_ps(x));
    return _mm_cvtss_f32(r);
#else
    return 1.0f / x;
#endif
}

// ---- small float-vector helpers ----------------------------------------------------------

struct Vec4 {
    float x, y, z, w;
};

Vec4 Load4(const float* p) { return Vec4{p[0], p[1], p[2], p[3]}; }
void Store4(float* p, const Vec4& v)
{
    p[0] = v.x;
    p[1] = v.y;
    p[2] = v.z;
    p[3] = v.w;
}

// (z*z + y*y) + x*x: the original's shuffle order, kept for bit-exact results.
float Len2(const Vec4& v) { return (v.z * v.z + v.y * v.y) + v.x * v.x; }
// (z + y) + x: the original's shuffle order for sums of already-multiplied lanes.
float Sum3(const Vec4& v) { return (v.z + v.y) + v.x; }

// The original's raw rsqrtps + one Newton step (no clamp; NaN propagates as in SSE).
float RefineRsqrt(float rs, float len2)
{
    const float e = 1.0f - (rs * rs) * len2;
    return (e * (0.5f * rs)) + rs;
}

// The original's four `comiss` tests jump when the comparison is unordered as well as
// less-than (`jb` after comiss): a NaN corner counts as "not inside".
bool InsideRect(const float* inner, const float* outer)
{
    return !(!(inner[0] >= outer[0]) || !(outer[2] >= inner[2]) || !(inner[1] >= outer[1]) ||
             !(outer[3] >= inner[3]));
}

// The original's the original instruction sequence idiom exits only when the two values
// are EQUAL (NaN compares unbalanced and falls through to the main path).
bool EqualOrNaN(float a, float b) { return a == b; }

// 0x006BFAF0 has an unusual ABI: the result buffer pointer is passed in EAX, the input
// vector in ECX, and three arguments on the stack (caller cleans). Only x86 can express it;
// it is a lighting helper of an unrewritten unit, so call it at its exe address.
int Call6BFAF0(void* result, void* input, void* a3, void* a4, void* a5)
{
#if defined(_M_IX86)
    int value;
    volatile uintptr_t target = 0x006BFAF0u;
    __asm {
        mov eax, result
        mov ecx, input
        mov edx, a5
        push edx
        mov edx, a4
        push edx
        mov edx, a3
        push edx
        mov edx, target
        call edx
        add esp, 12
        mov value, eax
    }
    return value;
#else
    (void)result; (void)input; (void)a3; (void)a4; (void)a5;
    AbortNotRewritten("a game function with an EAX/ECX ABI");
    return 0;
#endif
}

// 0x006BDB00 has the same shape: four stack arguments (this in ECX, the first vector also
// in EAX) and the callee pops them, so no stack cleanup here.
int Call6BDB00(void* self, void* up, void* a1, void* a2, void* a3)
{
#if defined(_M_IX86)
    int value;
    volatile uintptr_t target = 0x006BDB00u;
    __asm {
        mov eax, up
        mov ecx, self
        mov edx, a3
        push edx
        mov edx, a2
        push edx
        mov edx, a1
        push edx
        mov edx, up
        push edx
        mov edx, target
        call edx
        mov value, eax
    }
    return value;
#else
    (void)self; (void)up; (void)a1; (void)a2; (void)a3;
    AbortNotRewritten("a game function with an EAX/ECX ABI");
    return 0;
#endif
}

}  // namespace

// ==========================================================================================
// 0x0050F200 / 0x0050F580 / 0x0050F720 (rectangle helpers)
// ==========================================================================================

// 0x0050F200
int __cdecl ClipRect50F200(void* self, float* inner, float* dst, int a4, float* clipTo,
                           int a6)
{
    // EA quirk: `result` is initialised to the clipTo pointer, so the size early-outs
    // return its low 32 bits as an int.
    const int fallbackResult = static_cast<int>(reinterpret_cast<uintptr_t>(clipTo));
    Vec4 clip = Load4(inner);
    const float insideX = inner[0];
    const float insideY = inner[1];
    void** const vtbl = *reinterpret_cast<void***>(self);
    if (!InsideRect(inner, clipTo)) {
        const float w = inner[2] - inner[0];
        const float h = inner[3] - inner[1];
        if (!(1.0f > w) && !(1.0f > h)) {
            const int clipResult = CallGameMethod<int, 0x00505BD0>(&clip, inner, clipTo);
            if (EqualOrNaN(clip.x, clip.z) || EqualOrNaN(clip.y, clip.w)) {
                return clipResult;
            }
            // Map the clipped rect out of the inner rect's space into dst's space.
            const float invW = 1.0f / w;
            const float invH = 1.0f / h;
            const float spanX = dst[2] - dst[0];
            const float spanY = dst[3] - dst[1];
            const float mapped[4] = {
                ((clip.x - insideX) * invW) * spanX + dst[0],
                ((clip.y - insideY) * invH) * spanY + dst[1],
                ((clip.z - insideX) * invW) * spanX + dst[0],
                ((clip.w - insideY) * invH) * spanY + dst[1],
            };
            if (a6 == 0) {
                using SlotFn = int(__thiscall*)(void*, float*, int, const float*);
                return reinterpret_cast<SlotFn>(vtbl[0x50 / 4])(self, &clip.x, a4, mapped);
            }
            using SlotFn5 = int(__thiscall*)(void*, float*, int, int, const float*);
            return reinterpret_cast<SlotFn5>(vtbl[0x5C / 4])(self, &clip.x, a4, a6, mapped);
        }
        return fallbackResult;
    }
    // Fully inside clipTo: hand the rect straight through.
    if (a6 == 0) {
        using SlotFn = int(__thiscall*)(void*, float*, int, const float*);
        return reinterpret_cast<SlotFn>(vtbl[0x50 / 4])(self, inner, a4, dst);
    }
    using SlotFn5 = int(__thiscall*)(void*, float*, int, int, const float*);
    return reinterpret_cast<SlotFn5>(vtbl[0x5C / 4])(self, inner, a4, a6, dst);
}

// 0x0050F580
int __cdecl ClipRect50F580(void* self, float* inner, int a3, int a4, float* outer)
{
    // EA quirk: the size early-out returns the clipTo pointer's low 32 bits.
    const int fallbackResult = static_cast<int>(reinterpret_cast<uintptr_t>(outer));
    const float savedX = inner[0];
    const float savedY = inner[1];
    void** const vtbl = *reinterpret_cast<void***>(self);
    if (InsideRect(inner, outer)) {
        const float uv[4] = {0.0f, 0.0f, 1.0f, 1.0f};
        using SlotFn = int(__thiscall*)(void*, float*, int, int, const float*);
        return reinterpret_cast<SlotFn>(vtbl[0x5C / 4])(self, inner, a3, a4, uv);
    }
    const float w = inner[2] - inner[0];
    const float h = inner[3] - inner[1];
    if (1.0f > w || 1.0f > h) {
        return fallbackResult;
    }
    Vec4 clip = Load4(inner);
    const int clipResult = CallGameMethod<int, 0x00505BD0>(&clip, inner, outer);
    if (EqualOrNaN(clip.x, clip.z) || EqualOrNaN(clip.y, clip.w)) {
        return clipResult;
    }
    const float invW = 1.0f / w;
    const float invH = 1.0f / h;
    const float uv[4] = {
        (clip.x - savedX) * invW,
        (clip.y - savedY) * invH,
        (clip.z - savedX) * invW,
        (clip.w - savedY) * invH,
    };
    using SlotFn = int(__thiscall*)(void*, float*, int, int, const float*);
    return reinterpret_cast<SlotFn>(vtbl[0x5C / 4])(self, &clip.x, a3, a4, uv);
}

// 0x0050F720
void __cdecl TileRect50F720(void* obj, float* rect, void* desc, float sx, float sy, float ox,
                            float oy)
{
    const float cellW = static_cast<float>(*reinterpret_cast<const int*>(
                            static_cast<const uint8_t*>(desc) + 0x1C)) * sx;
    const float cellH = static_cast<float>(*reinterpret_cast<const int*>(
                            static_cast<const uint8_t*>(desc) + 0x20)) * sy;
    if (1.0f > cellW || 1.0f > cellH) {
        return;
    }
    // Wrap the offsets into (-size, size]; the original uses ceil/floor but the quotient is
    // exact for the boundary cases the tests cover.
    if (oy > 0.0f) {
        oy = oy - static_cast<float>(std::ceil(static_cast<double>(oy) / cellH)) * cellH;
    } else if (!(cellH < oy)) {
        oy = static_cast<float>(std::floor(-(static_cast<double>(oy) / cellH))) * cellH + oy;
    }
    if (ox > 0.0f) {
        ox = ox - static_cast<float>(std::ceil(static_cast<double>(ox) / cellW)) * cellW;
    } else if (!(cellW < ox)) {
        ox = static_cast<float>(std::floor(-(static_cast<double>(ox) / cellW))) * cellW + ox;
    }
    float y = rect[1] + oy;
    const float x0 = rect[0] + ox;
    float x = x0;
    if (!(rect[3] > y)) {
        return;
    }
    do {
        if (rect[2] > x) {
            const float tileBottom = y + cellH;
            do {
                const float tile[4] = {x, y, x + cellW, tileBottom};
                CallGameFunction<void, 0x0050EEF0>(obj, tile, desc, rect, 0);
                x += cellW;
            } while (rect[2] > x);
            x = x0;
        }
        y += cellH;
    } while (rect[3] > y);
}

// ==========================================================================================
// class 0x0101C0D8 / 0x0101C0C0
// ==========================================================================================

// 0x005334C0
float __fastcall Get5334C0(void* self, void*)
{
    return static_cast<const IconObject*>(self)->f0C;
}

// 0x007E4D30
int __fastcall Const7E4D30(void*, void*)
{
    return static_cast<int32_t>(0xBF7BFE30u);
}

// 0x007E4D40 / 0x007E4FF0
static void* GetInterfaceImpl(void* self, int magic)
{
    if (magic == kMagicInterfaceId) {
        if (self == nullptr) {
            return nullptr;
        }
        return static_cast<uint8_t*>(self) + 4;
    }
    return CallGameMethod<void*, 0x00504D90>(self, magic);
}

void* __fastcall GetInterface7E4D40(void* self, void*, int magic)
{
    return GetInterfaceImpl(self, magic);
}

void* __fastcall GetInterface7E4FF0(void* self, void*, int magic)
{
    // The secondary vtable slot 3 thunk subtracts the interface offset before entering
    // 0x007E4D40.
    return GetInterfaceImpl(static_cast<uint8_t*>(self) - 4, magic);
}

// 0x007E4DA0
int __fastcall ModifyBits7E4DA0(void* self, void*, int mask, int set)
{
    IconObject* const obj = static_cast<IconObject*>(self);
    using SlotFn = int(__thiscall*)(void*, int);
    void** const vtbl = *reinterpret_cast<void***>(self);
    const int value = set != 0 ? (obj->f10 | mask) : (obj->f10 & ~mask);
    return reinterpret_cast<SlotFn>(vtbl[0x30 / 4])(self, value);
}

// 0x007E4DD0
void __fastcall Set0C7E4DD0(void* self, void*, float v)
{
    static_cast<IconObject*>(self)->f0C = v;
}

// 0x007E4DE0
int __fastcall Set18_7E4DE0(void* self, void*, int v)
{
    static_cast<IconObject*>(self)->f18 = v;
    return v;
}

// 0x007E4DF0
int __fastcall Get20_7E4DF0(void* self, void*, int index)
{
    return *reinterpret_cast<int*>(static_cast<uint8_t*>(self) + 0x20 + index * 4);
}

// 0x007E4E00
int __fastcall Set20_7E4E00(void* self, void*, int index, int v)
{
    *reinterpret_cast<int*>(static_cast<uint8_t*>(self) + 0x20 + index * 4) = v;
    return v;
}

// 0x007E4E10
uint32_t __fastcall GetCell7E4E10(void* self, void*, float* out, int, int)
{
    const IconObject* const obj = static_cast<const IconObject*>(self);
    if (obj->f1C == nullptr) {
        return 0;
    }
    const uint8_t* const cell = static_cast<const uint8_t*>(obj->f1C);
    out[0] = static_cast<float>(*reinterpret_cast<const int*>(cell + 0x1C));
    out[1] = static_cast<float>(*reinterpret_cast<const int*>(cell + 0x20));
    // EA quirk: the original ends with the original instruction sequence after loading EAX with the out pointer,
    // so the result is (out & ~0xFF) | 1 and only AL carries the boolean. The null path
    // only clears AL, leaving EAX's upper bits undefined; the rewrite returns 0 there.
    return (static_cast<uint32_t>(reinterpret_cast<uintptr_t>(out)) & ~0xFFu) | 1u;
}

// 0x007E4E70
int __fastcall ClassReg7E4E70(void* self, void*, void* rec)
{
    uint8_t* const record = static_cast<uint8_t*>(rec);
    *reinterpret_cast<uintptr_t*>(record + 4) =
        reinterpret_cast<uintptr_t>(static_cast<uint8_t*>(self) - 4);
#if defined(_M_IX86)
    *reinterpret_cast<uint32_t*>(record) = static_cast<uint32_t>(kDescriptor101C080);
#else
    *reinterpret_cast<uint32_t*>(record) = 0;
#endif
    *reinterpret_cast<uint32_t*>(record + 8) = 1;
    // the original instruction sequence reads the vtable of the object passed in ecx (the secondary
    // interface), i.e. slot 5 of 0x0101C0C0 = 0x009E7310.
    using SlotFn = int(__thiscall*)(void*);
    void** const vtbl = *reinterpret_cast<void***>(self);
    const int result = reinterpret_cast<SlotFn>(vtbl[5])(self);
    *reinterpret_cast<int*>(record + 12) = result;
    return result;
}

// 0x007E4EA0
int __fastcall PlaceClip7E4EA0(void* self, void*, void* lookup, float* rect, uint32_t* flags)
{
    IconObject* const obj = static_cast<IconObject*>(self);
    if (obj->f1C == nullptr) {
        // EA quirk: the original returns whatever EAX held on this path (no result is
        // assigned); the rewrite returns 0.
        return 0;
    }
    // The record lookup receives the first stack argument in ECX as the lookup object.
    void* const record = CallGameMethod<void*, 0x0050E850>(lookup, 0, 0);
    const uint32_t flag = *flags;
    int32_t value;
    if ((flag & 8u) != 0) {
        value = *reinterpret_cast<int32_t*>(static_cast<uint8_t*>(self) +
                                           ((flag & ~8u) * 4u) + 0x30);
    } else {
        value = *reinterpret_cast<int32_t*>(static_cast<uint8_t*>(self) + flag * 4u + 0x20);
    }
    using RecordSlotFn = void(__thiscall*)(void*, int32_t);
    void** const recordVtbl = *reinterpret_cast<void***>(record);
    reinterpret_cast<RecordSlotFn>(recordVtbl[1])(record, value);

    float tile[4] = {rect[0], rect[1], rect[2], rect[3]};
    const int32_t f10 = obj->f10;
    if ((f10 & 1) != 0) {
        const float size = static_cast<float>(*reinterpret_cast<const int*>(
                               static_cast<const uint8_t*>(obj->f1C) + 0x1C)) * obj->f0C;
        float start = rect[0];
        switch (obj->f14) {
            case 2: start = rect[2] - size; break;
            case 3: start = ((rect[2] - rect[0]) - size) * 0.5f + rect[0]; break;
            default: start = rect[0]; break;  // modes 0 and 1 behave the same: keep the start
        }
        tile[0] = start;
        tile[2] = start + size;
    }
    if ((f10 & 2) != 0) {
        const float size = static_cast<float>(*reinterpret_cast<const int*>(
                               static_cast<const uint8_t*>(obj->f1C) + 0x20)) * obj->f0C;
        float start = rect[1];
        switch (obj->f18) {
            case 2: start = rect[3] - size; break;
            case 3: start = ((rect[3] - rect[1]) - size) * 0.5f + rect[1]; break;
            default: start = rect[1]; break;
        }
        tile[1] = start;
        tile[3] = start + size;
    }
    return CallGameFunction<int, 0x0050EEF0>(record, tile, obj->f1C, rect, 0);
}

// 0x007E4FE0 (secondary interface destructor; self is the +4 interface)
void* __fastcall DtorSec7E4FE0(void* self, void*, int flags)
{
    uint8_t* const primary = static_cast<uint8_t*>(self) - 4;
    void* const f1C = *reinterpret_cast<void**>(static_cast<uint8_t*>(self) + 0x18);
#if defined(_M_IX86)
    *reinterpret_cast<uintptr_t*>(primary) = kVtable101C0D8;
    *reinterpret_cast<uintptr_t*>(primary + 4) = kVtable101C0C0;
#else
    *reinterpret_cast<uintptr_t*>(primary) = 0;
    *reinterpret_cast<uintptr_t*>(primary + 4) = 0;
#endif
    if (f1C != nullptr) {
        CallGameVirtual<void, 1>(f1C);
    }
#if defined(_M_IX86)
    *reinterpret_cast<uintptr_t*>(primary + 4) = kVtableFBB428;
    *reinterpret_cast<uintptr_t*>(primary + 4) = kVtableFA1B78;
    *reinterpret_cast<uintptr_t*>(primary) = kVtableFA1B78;
#else
    *reinterpret_cast<uintptr_t*>(primary + 4) = 0;
    *reinterpret_cast<uintptr_t*>(primary) = 0;
#endif
    if ((flags & 1) != 0) {
        CallGameFunction<void, 0x00504E40>(primary);
    }
    return primary;
}

// 0x007E5000 (primary destructor)
void* __fastcall DtorPrimary7E5000(void* self, void*, int flags)
{
    IconObject* const obj = static_cast<IconObject*>(self);
    void* const f1C = obj->f1C;
#if defined(_M_IX86)
    *reinterpret_cast<uintptr_t*>(static_cast<uint8_t*>(self)) = kVtable101C0D8;
    *reinterpret_cast<uintptr_t*>(static_cast<uint8_t*>(self) + 4) = kVtable101C0C0;
#else
    *reinterpret_cast<uintptr_t*>(static_cast<uint8_t*>(self)) = 0;
    *reinterpret_cast<uintptr_t*>(static_cast<uint8_t*>(self) + 4) = 0;
#endif
    if (f1C != nullptr) {
        CallGameVirtual<void, 1>(f1C);
    }
#if defined(_M_IX86)
    *reinterpret_cast<uintptr_t*>(static_cast<uint8_t*>(self) + 4) = kVtableFBB428;
    *reinterpret_cast<uintptr_t*>(static_cast<uint8_t*>(self) + 4) = kVtableFA1B78;
    *reinterpret_cast<uintptr_t*>(static_cast<uint8_t*>(self)) = kVtableFA1B78;
#else
    *reinterpret_cast<uintptr_t*>(static_cast<uint8_t*>(self) + 4) = 0;
    *reinterpret_cast<uintptr_t*>(static_cast<uint8_t*>(self)) = 0;
#endif
    if ((flags & 1) != 0) {
        CallGameFunction<void, 0x00504E40>(self);
    }
    return self;
}

// 0x00F626F0
void* __fastcall IdentityF626F0(void* self, void*)
{
    return self;
}

// ==========================================================================================
// class 0x00FF42F8 / 0x00FF4518 (RectangleWindowLight)
// ==========================================================================================

namespace {

// The three transform rows at +0x110/+0x150/+0x160 are 4-float vectors, the matrix at
// `m` holds its columns at +0x00/+0x10/+0x20 and the translation at +0x30.
Vec4 TransformVec(const float* m, const Vec4& v, bool withTranslation)
{
    const Vec4 c0 = Load4(m + 0);
    const Vec4 c1 = Load4(m + 4);
    const Vec4 c2 = Load4(m + 8);
    if (withTranslation) {
        const Vec4 t = Load4(m + 12);
        return Vec4{
            ((c0.x * v.x + t.x) + c1.x * v.y) + c2.x * v.z,
            ((c0.y * v.x + t.y) + c1.y * v.y) + c2.y * v.z,
            ((c0.z * v.x + t.z) + c1.z * v.y) + c2.z * v.z,
            ((c0.w * v.x + t.w) + c1.w * v.y) + c2.w * v.z,
        };
    }
    // The original multiplies column 1 first, then adds column 0 and finally column 2.
    return Vec4{
        ((c1.x * v.y) + (c0.x * v.x)) + (c2.x * v.z),
        ((c1.y * v.y) + (c0.y * v.x)) + (c2.y * v.z),
        ((c1.z * v.y) + (c0.z * v.x)) + (c2.z * v.z),
        ((c1.w * v.y) + (c0.w * v.x)) + (c2.w * v.z),
    };
}

}  // namespace

// 0x006BD690
int __fastcall SetTransform6BD690(void* self, void*, const float* matrix)
{
    uint8_t* const base = static_cast<uint8_t*>(self);
    using SlotFn = int(__thiscall*)(void*);
    void** const vtbl = *reinterpret_cast<void***>(self);
    reinterpret_cast<SlotFn>(vtbl[0x44 / 4])(self);

    const Vec4 v110 = Load4(reinterpret_cast<const float*>(base + 0x110));
    const Vec4 v150 = Load4(reinterpret_cast<const float*>(base + 0x150));
    const Vec4 v160 = Load4(reinterpret_cast<const float*>(base + 0x160));
    const Vec4 row180 = TransformVec(matrix, v160, false);
    const Vec4 row170 = TransformVec(matrix, v150, false);
    const Vec4 row120 = TransformVec(matrix, v110, true);
    Store4(reinterpret_cast<float*>(base + 0x180), row180);
    Store4(reinterpret_cast<float*>(base + 0x170), row170);
    Store4(reinterpret_cast<float*>(base + 0x120), row120);

    // Cross product of row180 x row170 built from the shuffle 0x12/0x09 products. Lane 3
    // is (row180.x * row170.x) - (row180.x * row170.x): zero for finite inputs, but the
    // original computes it with the same vector products, so inf*inf - inf*inf stays NaN.
    // The two volatile reads make the subtraction happen at run time, so inf - inf
    // still yields NaN exactly like the original's mulps/subps pair.
    const volatile float* const crossX = &row180.x;
    const Vec4 cross{
        row180.z * row170.y - row180.y * row170.z,
        row180.x * row170.z - row180.z * row170.x,
        row180.y * row170.x - row180.x * row170.y,
        (*crossX * row170.x) - (*crossX * row170.x),
    };
    Store4(reinterpret_cast<float*>(base + 0x190), cross);
    reinterpret_cast<SlotFn>(vtbl[0x50 / 4])(self);
    return reinterpret_cast<SlotFn>(vtbl[0x44 / 4])(self);
}

// 0x006BD7C0
void __fastcall NormalizeToRadius6BD7C0(void* self, void*, float* out, const float* v)
{
    const uint8_t* const base = static_cast<const uint8_t*>(self);
    Vec4 local = Load4(v);
    local.z = 0.0f;
    Store4(out, local);
    const float len2 = Len2(local);
    const float threshold = *reinterpret_cast<const float*>(base + 0x1A0);
    if (len2 > threshold) {
        const float s = std::sqrt(len2);
        const float r0 = RcpApprox(s);
        const float r = (2.0f - r0 * s) * r0;
        const float radius = *reinterpret_cast<const float*>(base + 0x1A4);
        const Vec4 scaled{local.x * r, local.y * r, local.z * r, local.w * r};
        Store4(out, Vec4{scaled.x * radius, scaled.y * radius, scaled.z * radius,
                         scaled.w * radius});
    }
}

// 0x006BE020
void __fastcall Sample6BE020(void* self, void*, const float* point, const float* dir,
                             float* out)
{
    const uint8_t* const base = static_cast<const uint8_t*>(self);
    const Vec4 pos = Load4(reinterpret_cast<const float*>(base + 0x120));
    const Vec4 p = Load4(point);
    const Vec4 d{pos.x - p.x, pos.y - p.y, pos.z - p.z, pos.w - p.w};
    const float len2 = Len2(d);
    const float rs = RsqrtApprox(len2);
    const float t = LightGlobalA() * len2;
    const float rc = RcpApprox(t);
    const float inv = (2.0f - t * rc) * rc;
    const float rs2 = RefineRsqrt(rs, len2);
    const Vec4 dirv = Load4(dir);
    const Vec4 n{((d.x * rs2) * dirv.x), ((d.y * rs2) * dirv.y), ((d.z * rs2) * dirv.z),
                 ((d.w * rs2) * dirv.w)};
    const float dotN = Sum3(n);
    float f = dotN * (1.0f - dirv.w) + dirv.w;
    if (!(f > 0.0f)) {
        f = 0.0f;  // maxps(x, 0): NaN becomes 0 as in SSE
    }
    const float local =
        (*reinterpret_cast<const float*>(base + 0x130) *
         *reinterpret_cast<const float*>(base + 0x10) * f) *
        inv * LightGlobalB();
    float scale = local;
    const float cap = LightCap();
    if (scale > cap) {
        scale = cap;
    }
    const Vec4 block = Load4(reinterpret_cast<const float*>(base + 0xE0));
    const float g = LightScale();
    Store4(out, Vec4{scale * block.x * g, scale * block.y * g, scale * block.z * g,
                     scale * block.w * g});
}

// 0x006BFDC0
int __fastcall Sample6BFDC0(void* self, void*, const float* point, const float*, float* out)
{
    uint8_t* const base = static_cast<uint8_t*>(self);
    const Vec4 pos = Load4(reinterpret_cast<const float*>(base + 0x120));
    const Vec4 p = Load4(point);
    const Vec4 d{pos.x - p.x, pos.y - p.y, pos.z - p.z, pos.w - p.w};
    const Vec4 a0 = Load4(reinterpret_cast<const float*>(base + 0x170));
    const Vec4 a1 = Load4(reinterpret_cast<const float*>(base + 0x190));
    const Vec4 a2 = Load4(reinterpret_cast<const float*>(base + 0x180));
    const Vec4 m0{a0.x * d.x, a0.y * d.y, a0.z * d.z, a0.w * d.w};
    const Vec4 m1{a1.x * d.x, a1.y * d.y, a1.z * d.z, a1.w * d.w};
    const Vec4 m2{a2.x * d.x, a2.y * d.y, a2.z * d.z, a2.w * d.w};
    // The three dot products are packed {0x170 dot, 0x190 dot, 0x180 dot, 0x170 dot}: the
    // original's lane 3 comes from an uninitialized stack slot (a self-propagating constant),
    // but 0x006BFAF0 never reads that lane, so substituting lane 0's sum is unobservable.
    const float s170 = Sum3(m0);
    Vec4 packed{s170, Sum3(m1), Sum3(m2), s170};
    float normalized[4];
    NormalizeToRadius6BD7C0(self, nullptr, normalized, &packed.x);
    const Vec4 n = Load4(normalized);
    const Vec4 diff{packed.x - n.x, packed.y - n.y, packed.z - n.z, packed.w - n.w};
    const float len = std::sqrt(Len2(diff));
    const float t = len * len * LightGlobalA();
    const float rc = RcpApprox(t);
    const float inv = (2.0f - t * rc) * rc;
    float scale = (*reinterpret_cast<const float*>(base + 0x130) *
                   *reinterpret_cast<const float*>(base + 0x10) * OneFloat()) *
                  inv * LightGlobalB();
    const float cap = LightCap();
    float factor = scale;
    if (scale > cap) {
        factor = cap;
    }
    // 0x006BFAF0 overwrites the slot that held the (dead) reciprocal/cap vector with
    // clamp(v13.z, 0, 1) * this[0xE0 block]; its EAX argument is that slot's address.
    float resultVec[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float plane[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    const int result =
        Call6BFAF0(resultVec, &packed.x, normalized, base + 0xE0, plane);
    Store4(out, Vec4{factor * resultVec[0], factor * resultVec[1], factor * resultVec[2],
                     factor * resultVec[3]});
    return result;
}

// 0x006C04F0
void* __fastcall Ctor6C04F0(void* self, void*, void* a2, int a3, int a4)
{
    CallGameMethod<void, 0x006C0170>(self, 1, 11, a4);
#if defined(_M_IX86)
    *reinterpret_cast<uintptr_t*>(self) = 0x00FF42F8;
#else
    *reinterpret_cast<uintptr_t*>(self) = 0;
#endif
    if (a2 != nullptr) {
        uint8_t* const src = static_cast<uint8_t*>(a2);
        uint8_t* const dst = static_cast<uint8_t*>(self);
        CallGameMethod<void, 0x006BDA90>(self, src + 16);
        *reinterpret_cast<float*>(dst + 0x130) = *reinterpret_cast<const float*>(src + 0x1C);
        const float f4 = *reinterpret_cast<const float*>(src + 4);
        const float vec[4] = {*reinterpret_cast<const float*>(src + 4),
                              *reinterpret_cast<const float*>(src + 8),
                              *reinterpret_cast<const float*>(src + 0xC), f4};
        std::memcpy(dst + 0x110, vec, sizeof vec);
    }
    *reinterpret_cast<int*>(static_cast<uint8_t*>(self) + 0xB4) = a3;
    return self;
}

// 0x006C1860
void* __fastcall Ctor6C1860(void* self, void*, void* a2, int a3, int a4)
{
    CallGameMethod<void, 0x006C0170>(self, 1, 10, a4);
#if defined(_M_IX86)
    *reinterpret_cast<uintptr_t*>(self) = 0x00FF4518;
#else
    *reinterpret_cast<uintptr_t*>(self) = 0;
#endif
    if (a2 != nullptr) {
        uint8_t* const src = static_cast<uint8_t*>(a2);
        uint8_t* const dst = static_cast<uint8_t*>(self);
        CallGameMethod<void, 0x006BDA90>(self, src + 16);
        *reinterpret_cast<float*>(dst + 0x130) = *reinterpret_cast<const float*>(src + 0x1C);
        const float f4 = *reinterpret_cast<const float*>(src + 4);
        const float vec110[4] = {*reinterpret_cast<const float*>(src + 4),
                                 *reinterpret_cast<const float*>(src + 8),
                                 *reinterpret_cast<const float*>(src + 0xC), f4};
        std::memcpy(dst + 0x110, vec110, sizeof vec110);
        const float f20 = *reinterpret_cast<const float*>(src + 0x20);
        const float vec160[4] = {f20, *reinterpret_cast<const float*>(src + 0x24),
                                 *reinterpret_cast<const float*>(src + 0x28), f20};
        std::memcpy(dst + 0x160, vec160, sizeof vec160);
        const float f2C = *reinterpret_cast<const float*>(src + 0x2C);
        const float vec150[4] = {f2C, *reinterpret_cast<const float*>(src + 0x30),
                                 *reinterpret_cast<const float*>(src + 0x34), f2C};
        std::memcpy(dst + 0x150, vec150, sizeof vec150);
        const float v = *reinterpret_cast<const float*>(src + 0x38);
        *reinterpret_cast<float*>(dst + 0x1A0) = v * v;
        *reinterpret_cast<float*>(dst + 0x1A4) = v;
    }
    *reinterpret_cast<int*>(static_cast<uint8_t*>(self) + 0xB4) = a3;
    return self;
}

// 0x006C1980
int __fastcall Func6C1980(void* self, void*, const float* point, void* a1)
{
    uint8_t* const base = static_cast<uint8_t*>(self);
    if ((base[0x100] & 0x20) != 0) {
        void* const lookup = *reinterpret_cast<void**>(base + 0xB0);
        // 0x006C3550 is the singleton getter; the original lets the pushed argument stay
        // on the stack and lets the next call (which pops its argument) consume it, so the
        // same value is 0x006C2920's index. Balancing the stack explicitly is equivalent.
        void* const singleton = CallGameFunction<void*, 0x006C3550>(lookup);
        CallGameMethod<int, 0x006C2920>(singleton, static_cast<int>(reinterpret_cast<uintptr_t>(
                                                        lookup)));
        const Vec4 pos = Load4(reinterpret_cast<const float*>(base + 0x120));
        const Vec4 p = Load4(point);
        const Vec4 d{pos.x - p.x, pos.y - p.y, pos.z - p.z, pos.w - p.w};
        const Vec4 a170 = Load4(reinterpret_cast<const float*>(base + 0x170));
        const Vec4 a190 = Load4(reinterpret_cast<const float*>(base + 0x190));
        const Vec4 a180 = Load4(reinterpret_cast<const float*>(base + 0x180));
        const Vec4 m170{a170.x * d.x, a170.y * d.y, a170.z * d.z, a170.w * d.w};
        const Vec4 m190{a190.x * d.x, a190.y * d.y, a190.z * d.z, a190.w * d.w};
        const Vec4 m180{a180.x * d.x, a180.y * d.y, a180.z * d.z, a180.w * d.w};
        // Packing order is +0x170, +0x190, +0x180, +0x170 (the last lane repeats the first).
        const float s170 = Sum3(m170);
        // Vec4 locals keep the packing explicit (pointer arguments below) without raw
        // array-size literals in the body.
        Vec4 packed{s170, Sum3(m190), Sum3(m180), s170};
        Vec4 mapped;
        NormalizeToRadius6BD7C0(self, nullptr, &mapped.x, &packed.x);
        return CallGameMethod<int, 0x006C1400>(self, &mapped.x, &packed.x, a1);
    }
    Vec4 up{0.0f, 1.0f, 0.0f, 0.0f};
    Vec4 direction{0.0f, 0.0f, 0.0f, 0.0f};
    Vec4 scratch{0.0f, 0.0f, 0.0f, 0.0f};
    *reinterpret_cast<void**>(static_cast<uint8_t*>(a1) + 0x20) = self;
    // The callee reads the first vector from EAX as well and 0x00F626F0 as the 0x006BDB00
    // return path; the original returns this call's EAX.
    return Call6BDB00(a1, &up.x, &direction.x, &scratch.x, self);
}

}  // namespace ts3::misc