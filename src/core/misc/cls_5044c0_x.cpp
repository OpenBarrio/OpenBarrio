// cls_5044c0_x.cpp - portable rewrite of the TS3W.exe unit `cls_5044c0_x` (subsystem misc/,
// 40 entry points, 1757 bytes): unrelated small handlers, adjustor thunks and vector
// deleting destructors that the game reaches through vtables.  Each function's original
// address is in the comment above it; the assembly in the original disassembly
// is the source of truth.
//
// System boundaries (unrewritten game code) go through ts3/layout.h:
//   CallGameFunction  __cdecl targets (the caller cleans the stack in the original)
//   CallGameMethod    __thiscall targets with the game's own stack cleanup
//   CallGameVirtual   vtable slots of objects this unit does not own
#include "ts3/misc/cls_5044c0_x.h"

#include <intrin.h>
#include <emmintrin.h>

namespace ts3 {
namespace misc {

// Game function addresses.  On x64 the bridge in ts3/layout.h aborts if one is reached;
// the address is only an immediate there, never a call target.
constexpr unsigned kFree4E3F70 = 0x004E3F70;      // free(ptr)
constexpr unsigned kFree76AE00 = 0x0076AE00;      // free(ptr), different allocator
constexpr unsigned kPoll4DDBD0 = 0x004DDBD0;      // char (int, int, int)
constexpr unsigned kSet7D77C0 = 0x007D77C0;       // thiscall (this, int)
constexpr unsigned kReset5657A0 = 0x005657A0;     // (void*, void*)
constexpr unsigned kSet7D7960 = 0x007D7960;       // thiscall (this, void*)
constexpr unsigned kQuery4DD700 = 0x004DD700;     // (int, int*, int)
constexpr unsigned kReset565C80 = 0x00565C80;     // (void*, void*)
constexpr unsigned kResetCF3C70 = 0x00CF3C70;     // thiscall (this, int)
constexpr unsigned kSet7D7930 = 0x007D7930;       // thiscall (this, void*)
constexpr unsigned kDetach566170 = 0x00566170;    // thiscall (this, void*, int)
constexpr unsigned kShrink5815F0 = 0x005815F0;    // thiscall (this)
constexpr unsigned kDestroy581D80 = 0x00581D80;   // thiscall (this)
constexpr unsigned kRelease5E44C0 = 0x005E44C0;   // thiscall (this)
constexpr unsigned kDestroy5FDE50 = 0x005FDE50;   // thiscall (this)
constexpr unsigned kRelease5A9B80 = 0x005A9B80;   // thiscall (this)
constexpr unsigned kDestroy613F30 = 0x00613F30;   // thiscall (this)
constexpr unsigned kDestroy613F00 = 0x00613F00;   // thiscall (this)
constexpr unsigned kDestroy616D30 = 0x00616D30;   // thiscall (this)
constexpr unsigned kDestroy616C40 = 0x00616C40;   // thiscall (this)
constexpr unsigned kNotify618020 = 0x00618020;    // thiscall (this)
constexpr unsigned kDestroy570550 = 0x00570550;   // thiscall (this)

namespace {

// The original's EAX after a helper call, with AL replaced by `byte` (the original instruction sequence):
// the game's message handlers leave the upper three bytes of EAX from the last callee.
int32_t WithLowByte(int32_t lastCallResult, uint8_t byte)
{
    return (lastCallResult & ~0xFF) | static_cast<int32_t>(byte);
}

void* VtableWord(uintptr_t address)
{
    return reinterpret_cast<void*>(address);
}

}  // namespace

// -----------------------------------------------------------------------------------------
// 0x005044C0: the original instruction sequence.
int32_t ResourceBaseSec::AddRef()
{
    return static_cast<int32_t>(_InterlockedIncrement(reinterpret_cast<volatile long*>(&f_04)));
}

// 0x0078E8D0: the original instruction sequence.
int32_t ResourceBaseSec::AddRefAdjust8()
{
    return reinterpret_cast<ResourceBaseSec*>(reinterpret_cast<uint8_t*>(this) - 8)->AddRef();
}

// -----------------------------------------------------------------------------------------
// 0x00570080: the 4DDBD0 result is tested by the game's trigger helper (7D77C0) before
// a0[0x14] is re-read; on both bail-outs EAX keeps the helper's result with AL = 0.
int32_t ProceduralMotionController::HandleMessage570080(void* a0)
{
    uint8_t* msg = static_cast<uint8_t*>(a0);
    int32_t r = CallGameFunction<int32_t, kPoll4DDBD0>(
        *reinterpret_cast<int32_t*>(msg + 8), 1, *reinterpret_cast<int32_t*>(msg + 16));
    r = CallGameMethod<int32_t, kSet7D77C0>(a0, r);
    if (msg[0x14] == 0)
        return WithLowByte(r, 0);
    r = CallGameFunction<int32_t, kReset5657A0>(f_2C, a0);
    if (msg[0x14] == 0)
        return WithLowByte(r, 0);
    r = CallGameMethod<int32_t, kSet7D7960>(a0, reinterpret_cast<uint8_t*>(this) + 0x30);
    return WithLowByte(r, msg[0x14]);
}

// 0x005700E0: the original instruction sequence.
int32_t ProceduralMotionController::ClassId5700E0()
{
    return 0x0BA185DF;
}

// 0x005700F0 (ret 4): only the two ids the class reports map to the sub-object at -8.
void* ProceduralMotionController::QueryInterface5700F0(int32_t id)
{
    if (id == 0x0BA185DF || id == 0x7489D72B)
        return reinterpret_cast<uint8_t*>(this) - 8;
    return nullptr;
}

// 0x00570110 (ret 4): adjustor thunk the original instruction sequence.
void* ProceduralMotionController::DestroyingDtorAdjust570110(uint32_t flags)
{
    return reinterpret_cast<ProceduralMotionController*>(reinterpret_cast<uint8_t*>(this) - 0xC)
        ->DestroyingDtor570850(flags);
}

// 0x00570200 (ret 4).  The SSE sequence is kept instruction-for-instruction with
// intrinsics: v7 negates the x/y/z of the vector the game's slot +0x10 wrote, `two` is
// 2.0f (0x00F98A58), the rsqrt refinement uses 1.0f (0x0107A538) and 0.5f (0x00F9A5AC).
// The signed `comiss` result means a NaN distance takes the "non-negative" branch, so the
// test is `!(d < 0)`.
int32_t ProceduralMotionController::HandleMessage570200(float arg)
{
    if (this)
        ++f_04;
    void* source = f_1C;
    if (source != nullptr) {
        void* queried = CallGameVirtual<void*, 1>(source, static_cast<int32_t>(0x549B548B));
        if (queried != nullptr) {
            CallGameVirtual<void, 10>(queried, 0.0f);  // vtable +0x28
            CallGameVirtual<void, 14>(queried, 0.0f);  // vtable +0x38
            CallGameVirtual<void, 12>(queried, 1.0f);  // vtable +0x30
            CallGameVirtual<void, 16>(queried, 1.0f);  // vtable +0x40
            alignas(16) float in[4] = {0.0f, 0.0f, 0.0f, 1.0f};
            CallGameVirtual<void, 4>(queried, in);  // vtable +0x10 writes the vector
            const __m128 v6 = _mm_load_ps(f_30);
            const __m128 v1 = _mm_load_ps(in);
            alignas(16) static const float kSign[4] = {-0.0f, -0.0f, -0.0f, 0.0f};
            const __m128 v7 = _mm_xor_ps(_mm_load_ps(kSign), v1);
            const __m128 v8 = _mm_shuffle_ps(v7, v7, 0x12);
            const __m128 v9 = _mm_shuffle_ps(v7, v7, 0x09);
            const __m128 v31 = _mm_shuffle_ps(v7, v7, 0xFF);
            const __m128 two = _mm_set1_ps(2.0f);
            const __m128 v10 = _mm_add_ps(
                _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v6, v6, 0x12), v9),
                           _mm_mul_ps(_mm_shuffle_ps(v6, v6, 0x09), v8)),
                _mm_mul_ps(v31, v6));
            __m128 v30 = _mm_mul_ps(
                _mm_add_ps(
                    _mm_mul_ps(_mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v10, v10, 0x12), v9),
                                          _mm_mul_ps(_mm_shuffle_ps(v10, v10, 0x09), v8)),
                               two),
                    v6),
                _mm_set1_ps(arg));
            const __m128 sq = _mm_mul_ps(v30, v30);
            const __m128 dot = _mm_add_ps(
                _mm_add_ps(_mm_shuffle_ps(sq, sq, 2), _mm_shuffle_ps(sq, sq, 1)), sq);
            const float distance =
                _mm_cvtss_f32(_mm_sqrt_ps(_mm_shuffle_ps(dot, dot, 0)));
            const float remaining = f_40 - distance;
            if (!(remaining < 0.0f)) {
                f_40 = remaining;
            } else {
                const __m128 t = _mm_add_ps(
                    _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(v6, v6, 0x12), v9),
                               _mm_mul_ps(_mm_shuffle_ps(v6, v6, 0x09), v8)),
                    _mm_mul_ps(v6, v31));
                const __m128 v17 = _mm_add_ps(
                    _mm_mul_ps(_mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(t, t, 0x12), v9),
                                          _mm_mul_ps(_mm_shuffle_ps(t, t, 0x09), v8)),
                               two),
                    v6);
                const __m128 sq2 = _mm_mul_ps(v17, v17);
                const __m128 dot2 = _mm_add_ps(
                    _mm_add_ps(_mm_shuffle_ps(sq2, sq2, 2), _mm_shuffle_ps(sq2, sq2, 1)), sq2);
                const __m128 s2 = _mm_shuffle_ps(dot2, dot2, 0);
                const __m128 r = _mm_rsqrt_ps(s2);
                const __m128 one = _mm_set1_ps(1.0f);
                const __m128 half = _mm_set1_ps(0.5f);
                const __m128 r1 = _mm_add_ps(
                    _mm_mul_ps(_mm_sub_ps(one, _mm_mul_ps(_mm_mul_ps(r, r), s2)),
                               _mm_mul_ps(half, r)),
                    r);
                const __m128 r2 = _mm_add_ps(
                    _mm_mul_ps(_mm_sub_ps(one, _mm_mul_ps(_mm_mul_ps(r1, r1), s2)),
                               _mm_mul_ps(half, r1)),
                    r1);
                v30 = _mm_mul_ps(_mm_mul_ps(r2, v17), _mm_set1_ps(f_40));
                f_40 = 0.0f;
            }
            alignas(16) float out[4];
            _mm_store_ps(out, v30);
            CallGameVirtual<void, 6>(queried, out);  // vtable +0x18
        }
    }
    CallGameVirtual<void, 2>(f_2C, arg);  // vtable +8
    const int32_t rc = f_04 - 1;
    f_04 = rc;
    if (rc == 0) {
        f_04 = 1;
        return CallGameVirtual<int32_t, 0>(this, 1);
    }
    return rc;
}

// 0x005704B0 (ret 4): the 4DD700 result word must be <= 1 (unsigned compare in the
// original's `ja`) and a0[0x14] must stay non-zero after every step.  EAX follows the same
// last-call rule as 0x00570080.
int32_t ProceduralMotionController::HandleMessage5704B0(void* a0)
{
    uint8_t* msg = static_cast<uint8_t*>(a0);
    int32_t slot = -1;
    int32_t r = CallGameFunction<int32_t, kQuery4DD700>(
        *reinterpret_cast<int32_t*>(msg + 8), &slot, *reinterpret_cast<int32_t*>(msg + 16));
    r = CallGameMethod<int32_t, kSet7D77C0>(a0, r);
    if (msg[0x14] == 0 || static_cast<uint32_t>(slot) > 1u)
        return WithLowByte(r, 0);
    r = CallGameFunction<int32_t, kReset565C80>(f_1C, a0);
    r = CallGameMethod<int32_t, kResetCF3C70>(reinterpret_cast<uint8_t*>(this) + 0x2C, r);
    if (f_2C == nullptr)
        return WithLowByte(r, 0);
    r = CallGameMethod<int32_t, kSet7D7930>(a0, reinterpret_cast<uint8_t*>(this) + 0x30);
    if (msg[0x14] == 0)
        return WithLowByte(r, 0);
    r = CallGameMethod<int32_t, kDetach566170>(
        reinterpret_cast<uint8_t*>(f_2C) + 0xC, reinterpret_cast<uint8_t*>(this) + 0x28, -1);
    return WithLowByte(r, msg[0x14]);
}

// 0x00570850 (ret 4, vtable slot 0): base destructor 0x00570550 then an optional free;
// EAX is `this` on every path.
void* ProceduralMotionController::DestroyingDtor570850(uint32_t flags)
{
    CallGameMethod<void, kDestroy570550>(this);
    if (flags & 1)
        CallGameFunction<void, kFree4E3F70>(this);
    return this;
}

// -----------------------------------------------------------------------------------------
// 0x00581EA0 (ret 8): `this` = entity+8.  The id mismatch path leaves the upper EAX bytes
// to the caller (the original instruction sequence only), the taken path sets EAX = 1.
int32_t SceneEntitySub::HandleMessage581EA0(int32_t id, int32_t)
{
    if (id != 0x7829873A)
        return 0;
    const uint32_t held = f_08 | f_0C;
    if (held != 0) {
        if ((f_04 & 2) == 2)
            CallGameMethod<void, kShrink5815F0>(reinterpret_cast<uint8_t*>(this) - 8);
        f_04 &= ~2u;
        f_08 = 0;
        f_0C = 0;
    }
    return 1;
}

// 0x005818D0 (ret 4): adjustor thunk the original instruction sequence.
void* SceneEntitySub::DestroyingDtorAdjust5818D0(uint32_t flags)
{
    return reinterpret_cast<SceneEntity*>(reinterpret_cast<uint8_t*>(this) - 8)
        ->DestroyingDtor581FC0(flags);
}

// 0x00581FC0 (ret 4, vtable slot 0): derived destructor; the second vtable word is
// restored to the base class's before the optional free.  Returns `this`.
void* SceneEntity::DestroyingDtor581FC0(uint32_t flags)
{
    f_00 = VtableWord(0x00FCB7EC);
    sub.f_00 = VtableWord(0x00FCB7DC);
    CallGameMethod<void, kDestroy581D80>(this);
    if (sub.f_10 != nullptr)
        CallGameFunction<void, kFree4E3F70>(sub.f_10);
    sub.f_00 = VtableWord(0x00FB2CBC);
    f_00 = VtableWord(0x00FF39F8);
    if (flags & 1)
        CallGameFunction<void, kFree4E3F70>(this);
    return this;
}

// -----------------------------------------------------------------------------------------
// 0x005A7120: the original instruction sequence.
int64_t StdFpos::SeekPos()
{
    return f_08;
}

// -----------------------------------------------------------------------------------------
// 0x005A71D0 (ret 4): release the optional object at +0x18 through vtable slot +4.
void* ManagedOwner5A71D0::DestroyingDtor(uint32_t flags)
{
    f_00 = VtableWord(0x00FDE6B0);
    if (f_18 != nullptr)
        CallGameVirtual<void, 1>(f_18);
    f_00 = VtableWord(0x00F9A44C);
    if (flags & 1)
        CallGameFunction<void, kFree76AE00>(this);
    return this;
}

// -----------------------------------------------------------------------------------------
// 0x005A8E00 (ret 4): all three vtable words are rewritten, the last two to the base
// class's; the second store pair crosses, exactly like the original.
void* TripleVtable5A8E00::DestroyingDtor(uint32_t flags)
{
    f_04 = VtableWord(0x00FDE70C);
    f_08 = VtableWord(0x00FDE6FC);
    f_08 = VtableWord(0x00FF39F8);
    f_04 = VtableWord(0x00FB2CBC);
    f_00 = VtableWord(0x00F9A44C);
    if (flags & 1)
        CallGameFunction<void, kFree4E3F70>(this);
    return this;
}

// 0x005A8DE0 (ret 4): adjustor thunk the original instruction sequence.
void* TripleVtable5A8E00::DestroyingDtorAdjust4(uint32_t flags)
{
    return reinterpret_cast<TripleVtable5A8E00*>(reinterpret_cast<uint8_t*>(this) - 4)
        ->DestroyingDtor(flags);
}

// 0x005A8DF0 (ret 4): adjustor thunk the original instruction sequence.
void* TripleVtable5A8E00::DestroyingDtorAdjust8(uint32_t flags)
{
    return reinterpret_cast<TripleVtable5A8E00*>(reinterpret_cast<uint8_t*>(this) - 8)
        ->DestroyingDtor(flags);
}

// -----------------------------------------------------------------------------------------
// 0x005A9BE0 (ret 4): release the sub-object through the game's 0x005A9B80 first.
void* Object5A9BE0::DestroyingDtor(uint32_t flags)
{
    CallGameMethod<void, kRelease5A9B80>(this);
    if (flags & 1)
        CallGameFunction<void, kFree76AE00>(this);
    return this;
}

// -----------------------------------------------------------------------------------------
// 0x005AD500 (ret 8): only id 0x1FC9AD87 is handled; AL = 1 on every handled path, the
// upper EAX bytes come from the free (0x004E3F70) when the pointer existed.
int32_t CasBuilder5AD500::HandleMessage5AD500(int32_t id, int32_t)
{
    if (id != 0x1FC9AD87)
        return 0;
    int32_t last = 0;
    if (f_0C != nullptr) {
        CallGameMethod<void, kRelease5E44C0>(f_0C);
        last = CallGameFunction<int32_t, kFree4E3F70>(f_0C);
        f_0C = nullptr;
    }
    return WithLowByte(last, 1);
}

// -----------------------------------------------------------------------------------------
// 0x005ADD20 (ret 4): base vtable then optional free; returns `this`.
void* SimpleDtor5ADD20::DestroyingDtor(uint32_t flags)
{
    f_00 = VtableWord(0x00F9A44C);
    if (flags & 1)
        CallGameFunction<void, kFree76AE00>(this);
    return this;
}

// 0x005FE250 (ret 4): the game's 0x005FDE50 body runs before the optional free.
void* SimpleDtor5FE250::DestroyingDtor(uint32_t flags)
{
    CallGameMethod<void, kDestroy5FDE50>(this);
    if (flags & 1)
        CallGameFunction<void, kFree4E3F70>(this);
    return this;
}

// 0x00743850 (ret 4): base vtable then optional free; returns `this`.
void* SimpleDtor743850::DestroyingDtor(uint32_t flags)
{
    f_00 = VtableWord(0x00FDE658);
    if (flags & 1)
        CallGameFunction<void, kFree4E3F70>(this);
    return this;
}

// -----------------------------------------------------------------------------------------
// 0x00616C20: vtable slot +0xC plus 36 (the original instruction sequence).
int32_t AlphaTexturePartial::GetOffset616C20()
{
    return CallGameVirtual<int32_t, 3>(this) + 36;
}

// 0x00616C30: `shl` uses CL & 31; the add is a full 32-bit add on the dword at +0x18.
int32_t AlphaTexturePartial::ComputeShift616C30()
{
    const uint32_t count = static_cast<uint8_t>(f_18 + 1) & 31u;
    return static_cast<int32_t>(f_10 << count);
}

// 0x00616EB0: a zero word at +0xC forwards to 0x00616C40 on the base sub-object; the other
// path's the original instruction sequence leaves the upper EAX bytes to the caller.
int32_t AlphaTexturePartial::Poll616EB0()
{
    if (f_0C == 0)
        return CallGameMethod<int32_t, kDestroy616C40>(reinterpret_cast<uint8_t*>(this) - 8);
    return 1;
}

// 0x00616ED0: bit 0 of the byte at +0x14 forwards to 0x00616D30 on the base sub-object;
// otherwise the original returns with EAX untouched (the caller's residue).
int32_t AlphaTexturePartial::Notify616ED0()
{
    if ((f_14 & 1) != 0)
        return CallGameMethod<int32_t, kDestroy616D30>(reinterpret_cast<uint8_t*>(this) - 8);
    return 0;
}

// 0x00616F20 (ret 4): adjustor thunk the original instruction sequence.
void* AlphaTexturePartial::DestroyingDtorAdjust616F20(uint32_t flags)
{
    return reinterpret_cast<AlphaTexturePartial*>(reinterpret_cast<uint8_t*>(this) - 8)
        ->DestroyingDtor616F70(flags);
}

// 0x00616F70 (ret 4): both vtable words, three game destructor helpers, then the optional
// free; returns `this`.
void* AlphaTexturePartial::DestroyingDtor616F70(uint32_t flags)
{
    f_00 = VtableWord(0x00FE8318);
    f_08 = VtableWord(0x00FE8328);
    CallGameMethod<void, kDestroy613F30>(reinterpret_cast<uint8_t*>(this) + 8);
    CallGameMethod<void, kDestroy616D30>(this);
    CallGameMethod<void, kDestroy613F00>(reinterpret_cast<uint8_t*>(this) + 8);
    f_00 = VtableWord(0x00FF39F8);
    if (flags & 1)
        CallGameFunction<void, kFree4E3F70>(this);
    return this;
}

// -----------------------------------------------------------------------------------------
// 0x00617AF0: the original instruction sequence.
int32_t VideoDownsampleMaterial::NumErrors617AF0()
{
    return 5;
}

// 0x00617CE0: adjustor thunk the original instruction sequence, `this` = base+8.
int32_t VideoDownsampleMaterial::Process618020Adjust8()
{
    return CallGameMethod<int32_t, kNotify618020>(reinterpret_cast<uint8_t*>(this) - 8);
}

// -----------------------------------------------------------------------------------------
// 0x00B742C0: the original instruction sequence.
int32_t GDEBufferView::KindB742C0()
{
    return 2;
}

// -----------------------------------------------------------------------------------------
// 0x00C0FD60: a one-byte `ret` script bridge stub; it returns whatever EAX held.
void TextureResourceStub::ScubaDivingModeStub()
{
}

// -----------------------------------------------------------------------------------------
// 0x008ABE50: the original instruction sequence.
int32_t RefCounted08::AddRef()
{
    return static_cast<int32_t>(_InterlockedIncrement(reinterpret_cast<volatile long*>(&f_0C)));
}

// 0x008ABE60: release; the refcount is rewound to 1 before the vtable slot 0 call.  The
// original's the original instruction sequence after the original instruction sequence can only fail for `this == (void*)-8`, which
// no live object is, so the guard is kept as an address comparison.
int32_t RefCounted08::Release()
{
    const int32_t rc = static_cast<int32_t>(
        _InterlockedDecrement(reinterpret_cast<volatile long*>(&f_0C)));
    if (rc != 0)
        return rc;
    _InterlockedExchange(reinterpret_cast<volatile long*>(&f_0C), 1);
    uint8_t* const slot = reinterpret_cast<uint8_t*>(this) + 8;
    if (slot != nullptr)
        CallGameVirtual<void, 0>(slot, 1);
    return 0;
}

// 0x008711F0: adjustor thunk the original instruction sequence.
int32_t RefCounted08::ReleaseAdjust4()
{
    return reinterpret_cast<RefCounted08*>(reinterpret_cast<uint8_t*>(this) - 4)->Release();
}

// 0x009C0440: adjustor thunk the original instruction sequence.
int32_t RefCounted08::ReleaseAdjust8()
{
    return reinterpret_cast<RefCounted08*>(reinterpret_cast<uint8_t*>(this) - 8)->Release();
}

// 0x00EC2350: adjustor thunk the original instruction sequence.
int32_t RefCounted08::AddRefAdjust4()
{
    return reinterpret_cast<RefCounted08*>(reinterpret_cast<uint8_t*>(this) - 4)->AddRef();
}

// 0x009C0450: adjustor thunk the original instruction sequence.
int32_t RefCounted08::AddRefAdjust8()
{
    return reinterpret_cast<RefCounted08*>(reinterpret_cast<uint8_t*>(this) - 8)->AddRef();
}

// 0x00EC17A0: the same release protocol with the vtable at +0 and the refcount at +4.
int32_t RefCounted00::Release()
{
    const int32_t rc = static_cast<int32_t>(
        _InterlockedDecrement(reinterpret_cast<volatile long*>(&f_04)));
    if (rc != 0)
        return rc;
    _InterlockedExchange(reinterpret_cast<volatile long*>(&f_04), 1);
    if (this != nullptr)
        CallGameVirtual<void, 0>(this, 1);
    return 0;
}

// 0x0078E8E0: adjustor thunk the original instruction sequence.
int32_t RefCounted00::ReleaseAdjust8()
{
    return reinterpret_cast<RefCounted00*>(reinterpret_cast<uint8_t*>(this) - 8)->Release();
}

}  // namespace misc
}  // namespace ts3