// Reimplementation of RouteManager unit cls_857220_RouteManager (TS3W.exe
// 0x00857220..0x00953710): the constructor/destructor and two small methods of the
// vehicle-body route manager object plus the axis-angle quaternion helper 0x008607C0 its
// route code uses. Layout evidence in the header. Differentially tested in
// src/exehost/cls_857220_RouteManager_test.cpp.
//
// 0x008607C0 is a 1:1 transcription of the game's instruction sequence (register names
// kept: xmm0..xmm7; the game's esp-relative 16-byte slots are st[0..20]; every line
// carries the original address and instruction): the function is full of rsqrt/refinement
// and four-branch quaternion extraction where every rounding and NaN path matters. On
// x86 the transcription is inline assembly, because MSVC /O2 reorders equivalent C++
// intrinsics enough to change which NaN payload survives (see the comment above the
// function); the x64 build uses the plain C++ form.
#include "ts3/routemanager/cls_857220_RouteManager.h"

#include <cmath>
#include <cstring>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace ts3::routemanager {
namespace {

constexpr uintptr_t kVtableMain = 0x01027690;
constexpr uintptr_t kVtableSub04 = 0x0102768C;
constexpr uintptr_t kVtableSub18 = 0x0102767C;

// The C++ fallback's stand-ins for the x87 fsin/fcos pair of 0x008607C0
// (0x00860923..0x0086094A): fld theta; fsin/fcos; fstp. Only the x64 build takes this
// path (x86 is inline assembly and runs the real x87 instructions).
float X87Sin(float v) { return std::sin(v); }
float X87Cos(float v) { return std::cos(v); }

}  // namespace

// TS3W.exe 0x00857220.
RouteManagerObject* RouteManagerObject::Construct(uint32_t paramA, uint32_t paramB)
{
    // 0x0085A100 (unit cls_839750_x) runs FIRST, on the not-yet-initialized object; the
    // game discards its return (xor eax, eax right after the call).
    CallGameMethod<void, 0x0085A100>(this, paramA, paramB);
    vtableMain = reinterpret_cast<void*>(kVtableMain);
    vtableSub04 = reinterpret_cast<void*>(kVtableSub04);
    vtableSub18 = reinterpret_cast<void*>(kVtableSub18);
    childD0 = nullptr;
    field128 = 0;
    field12C = 0;
    return this;
}

// TS3W.exe 0x008583D0.
RouteManagerObject* RouteManagerObject::Destruct(uint32_t flags)
{
    vtableMain = reinterpret_cast<void*>(kVtableMain);
    vtableSub04 = reinterpret_cast<void*>(kVtableSub04);
    vtableSub18 = reinterpret_cast<void*>(kVtableSub18);
    // 0x00856F10 (unit clu_83ba40_Scratch).
    CallGameMethod<void, 0x00856F10>(this);
    if (childD0) {
        auto* child = static_cast<RouteManagerChild*>(childD0);
        // Release at exactly 1 -> 0; the game re-arms the count to 1 before the vtable
        // call (the game's self-resurrect guard) and never touches the pointer after.
        if (--child->refcount == 0) {
            child->refcount = 1;
            CallGameVirtual<void, 0>(child, 1u);
        }
    }
    // 0x00859890 (unit cls_839750_x).
    CallGameMethod<void, 0x00859890>(this);
    if (flags & 1u)
        CallGameFunction<void, 0x004E3F70>(static_cast<void*>(this));
    return this;
}

// TS3W.exe 0x008591F0.
uint32_t RouteManagerObject::ReleaseHandle()
{
    // Null handle: the game returns the 0 it just loaded without touching anything (+0xC8
    // is already 0, so clearing it unconditionally below is unobservable either way).
    // Non-null: the game dereferences request14 without a null check; so does this.
    const uint32_t handle = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(handleC8));
    uint32_t result = 0;
    if (handle)
    {
        result = CallGameVirtual<uint32_t, 30>(request14, handle, 0u);
        handleC8 = nullptr;
    }
    return result;
}

// TS3W.exe 0x00953710.
uint16_t RouteManagerObject::SetWordFloat(uint8_t value, float speed)
{
    word106 = value;
    float108 = speed;
    return value;  // the game leaves the zero-extended byte in AX
}

// TS3W.exe 0x008607C0, cdecl: three 16-byte-aligned vector pointers, char return in AL.
// Builds the rotation matrix Rot(u, theta) with u = normalize(a1) (rsqrt with two
// Newton-Raphson refinements; the dot product uses only the x/y/z lanes, so a1's w is
// ignored) and theta = a2[2] (x87 fsin/fcos), then extracts the quaternion of R*M (M's
// columns (u.z,0,-u.x), (-u.x*u.y,0,-u.z*u.y), u) with the game's four-way branch.
// Writes 16 bytes to *out (movaps: a1/a2/out must be 16-byte aligned, like the original)
// and returns 1; returns 0 without touching out when a1's x/y/z squared length is <= 0.
//
// The x86 build is a 1:1 inline-assembly transcription of the original instruction
// sequence: MSVC /O2 reorders and merges the equivalent C++ intrinsics (movss/shufps
// pairs become broadcasts, stores get forwarded), and with a signaling-NaN angle that
// changes which NaN payload survives into the four-way quaternion extraction, so the
// results stopped being bit-identical to the game (found by the random bit-pattern
// differential cases; /Od gave the game's bytes, /O2 did not). Inline assembly is opaque
// to the optimizer, so this path is exactly the game's instructions, x87 fsin/fcos
// included. stk[i] mirrors the game's [esp + i] frame slot i (0x150 bytes, 16-aligned
// for the movaps pairs); kOne/kHalf replace the constants the game reads at
// [0x0107A538] and [0x00F9A5AC]. The x64 build keeps the plain C++ transcription: bit
// -exact for all finite inputs and for quiet-NaN angles; a signaling-NaN angle may pick
// a different extraction lane there (the x64 build is for the future port, where this
// becomes native SSE anyway).
#if defined(_M_IX86)
int8_t QuaternionFromAxisAngle(const void* a1, const void* a2, void* out)
{
    // 0x150 slots + up to 15 bytes the block needs to 16-align esi
    uint8_t stk[0x150 + 0xF];
    static const float kOne = 1.0f;   // TS3W.exe 0x0107A538
    static const float kHalf = 0.5f;  // TS3W.exe 0x00F9A5AC
    const void* arg1 = a1;
    const void* arg2 = a2;
    void* arg3 = out;
    int8_t result = 0x00;
    __asm {
        push esi
        lea esi, dword ptr [stk]
        add esi, 0xF
        and esi, -16                      ; esi = 16-aligned slot base (the game's aligned esp)
        mov eax, dword ptr [arg1]                   ; 0x008607CC
        movaps xmm4, xmmword ptr [eax]              ; 0x008607CF
        xorps xmm5, xmm5                            ; 0x008607D2
        movaps xmm1, xmm4                           ; 0x008607D5
        mulps xmm1, xmm4                            ; 0x008607D8
        movaps xmm0, xmm1                           ; 0x008607DB
        shufps xmm0, xmm1, 2                        ; 0x008607DE
        movaps xmm2, xmm1                           ; 0x008607E2
        shufps xmm2, xmm1, 1                        ; 0x008607E5
        addps xmm0, xmm2                            ; 0x008607E9
        addps xmm0, xmm1                            ; 0x008607EC
        shufps xmm0, xmm0, 0                        ; 0x008607EF
        movss dword ptr [esi + 0xa0], xmm0          ; 0x008607F3
        movss xmm0, xmm5                            ; 0x008607FC
        shufps xmm0, xmm0, 0                        ; 0x00860800
        movss dword ptr [esi + 0x30], xmm0          ; 0x00860804
        movaps xmmword ptr [esi + 0x110], xmm0      ; 0x0086080A
        movss xmm0, dword ptr [esi + 0xa0]          ; 0x00860812
        comiss xmm0, dword ptr [esi + 0x30]         ; 0x0086081B
        jbe L_00860DDC                              ; 0x00860820
        movss xmm6, dword ptr [kOne]                ; 0x00860826
        movaps xmm1, xmm4                           ; 0x0086082E
        mulps xmm1, xmm4                            ; 0x00860831
        movaps xmm0, xmm1                           ; 0x00860834
        shufps xmm0, xmm1, 2                        ; 0x00860837
        movaps xmm2, xmm1                           ; 0x0086083B
        shufps xmm2, xmm1, 1                        ; 0x0086083E
        addps xmm0, xmm2                            ; 0x00860842
        movss xmm2, dword ptr [kHalf]               ; 0x00860845
        addps xmm0, xmm1                            ; 0x0086084D
        shufps xmm0, xmm0, 0                        ; 0x00860850
        rsqrtps xmm1, xmm0                          ; 0x00860854
        movaps xmm7, xmm1                           ; 0x00860857
        mulps xmm7, xmm1                            ; 0x0086085A
        mulps xmm7, xmm0                            ; 0x0086085D
        mov ecx, dword ptr [arg2]                   ; 0x00860860
        shufps xmm6, xmm6, 0                        ; 0x00860863
        movaps xmm3, xmm6                           ; 0x00860867
        subps xmm3, xmm7                            ; 0x0086086A
        shufps xmm2, xmm2, 0                        ; 0x0086086D
        movaps xmm7, xmm2                           ; 0x00860871
        mulps xmm7, xmm1                            ; 0x00860874
        mulps xmm3, xmm7                            ; 0x00860877
        addps xmm3, xmm1                            ; 0x0086087A
        movaps xmm7, xmm3                           ; 0x0086087D
        mulps xmm7, xmm3                            ; 0x00860880
        mulps xmm7, xmm0                            ; 0x00860883
        movss dword ptr [esi + 0xc], xmm5           ; 0x00860886
        movss dword ptr [esi + 8], xmm5             ; 0x0086088C
        movss dword ptr [esi], xmm5                 ; 0x00860892
        movaps xmm1, xmm6                           ; 0x00860897
        subps xmm1, xmm7                            ; 0x0086089A
        movaps xmm0, xmm2                           ; 0x0086089D
        mulps xmm0, xmm3                            ; 0x008608A0
        mulps xmm1, xmm0                            ; 0x008608A3
        movss xmm0, dword ptr [kOne]                ; 0x008608A6
        addps xmm1, xmm3                            ; 0x008608AE
        mulps xmm1, xmm4                            ; 0x008608B1
        movss dword ptr [esi + 4], xmm0             ; 0x008608B4
        movaps xmm0, xmmword ptr [esi]              ; 0x008608BA
        movaps xmm3, xmm1                           ; 0x008608BE
        shufps xmm3, xmm1, 0x12                     ; 0x008608C1
        movaps xmm4, xmm1                           ; 0x008608C5
        shufps xmm4, xmm1, 9                        ; 0x008608C8
        movaps xmm5, xmm0                           ; 0x008608CC
        shufps xmm5, xmm0, 0x12                     ; 0x008608CF
        mulps xmm5, xmm4                            ; 0x008608D3
        shufps xmm0, xmm0, 9                        ; 0x008608D6
        mulps xmm0, xmm3                            ; 0x008608DA
        subps xmm0, xmm5                            ; 0x008608DD
        movaps xmm5, xmm0                           ; 0x008608E0
        shufps xmm5, xmm0, 0x12                     ; 0x008608E3
        mulps xmm5, xmm4                            ; 0x008608E7
        movaps xmmword ptr [esi + 0x60], xmm1       ; 0x008608EA
        movaps xmmword ptr [esi + 0x140], xmm1      ; 0x008608EF
        movaps xmm4, xmm0                           ; 0x008608F7
        shufps xmm4, xmm0, 9                        ; 0x008608FA
        mulps xmm4, xmm3                            ; 0x008608FE
        movaps xmmword ptr [esi + 0x120], xmm0      ; 0x00860901
        subps xmm5, xmm4                            ; 0x00860909
        movaps xmmword ptr [esi + 0x130], xmm5      ; 0x0086090C
        movaps xmm1, xmmword ptr [ecx]              ; 0x00860914
        shufps xmm1, xmm1, 0xaa                     ; 0x00860917
        movaps xmm3, xmm1                           ; 0x0086091B
        movaps xmmword ptr [esi + 0x20], xmm3       ; 0x0086091E
        fld dword ptr [esi + 0x20]                  ; 0x00860923
        movaps xmmword ptr [esi + 0x20], xmm3       ; 0x00860927
        fsin                                        ; 0x0086092C
        movaps xmmword ptr [esi + 0x90], xmm6       ; 0x0086092E
        fstp dword ptr [esi + 0x1c]                 ; 0x00860936
        fld dword ptr [esi + 0x20]                  ; 0x0086093A
        fcos                                        ; 0x0086093E
        movss xmm1, dword ptr [esi + 0x1c]          ; 0x00860940
        shufps xmm1, xmm1, 0                        ; 0x00860946
        fstp dword ptr [esi + 0x1c]                 ; 0x0086094A
        movss xmm3, dword ptr [esi + 0x1c]          ; 0x0086094E
        movaps xmm5, xmmword ptr [esi + 0x60]       ; 0x00860954
        shufps xmm3, xmm3, 0                        ; 0x00860959
        movaps xmmword ptr [esi + 0x70], xmm3       ; 0x0086095D
        subps xmm6, xmm3                            ; 0x00860962
        movaps xmm3, xmm5                           ; 0x00860965
        movaps xmm4, xmm6                           ; 0x00860968
        shufps xmm3, xmm5, 0                        ; 0x0086096B
        movaps xmm7, xmm6                           ; 0x0086096F
        mulps xmm4, xmm3                            ; 0x00860972
        movaps xmmword ptr [esi + 0x40], xmm4       ; 0x00860975
        movaps xmm4, xmm5                           ; 0x0086097A
        shufps xmm4, xmm5, 0x55                     ; 0x0086097D
        shufps xmm5, xmm5, 0xaa                     ; 0x00860981
        mulps xmm6, xmm5                            ; 0x00860985
        movaps xmmword ptr [esi], xmm6              ; 0x00860988
        movaps xmm6, xmm1                           ; 0x0086098C
        mulps xmm6, xmm3                            ; 0x0086098F
        movaps xmmword ptr [esi + 0x20], xmm6       ; 0x00860992
        movaps xmm6, xmm1                           ; 0x00860997
        mulps xmm1, xmm5                            ; 0x0086099A
        movaps xmmword ptr [esi + 0x60], xmm1       ; 0x0086099D
        movaps xmm1, xmmword ptr [esi]              ; 0x008609A2
        mulps xmm1, xmm5                            ; 0x008609A6
        addps xmm1, xmmword ptr [esi + 0x70]        ; 0x008609A9
        movaps xmmword ptr [esi + 0xb0], xmm1       ; 0x008609AE
        movaps xmm1, xmmword ptr [esi]              ; 0x008609B6
        mulps xmm1, xmm4                            ; 0x008609BA
        subps xmm1, xmmword ptr [esi + 0x20]        ; 0x008609BD
        movaps xmmword ptr [esi + 0x80], xmm1       ; 0x008609C2
        movaps xmm1, xmmword ptr [esi]              ; 0x008609CA
        mulps xmm1, xmm3                            ; 0x008609CE
        mulps xmm7, xmm4                            ; 0x008609D1
        mulps xmm6, xmm4                            ; 0x008609D4
        movaps xmmword ptr [esi + 0x30], xmm6       ; 0x008609D7
        addps xmm1, xmmword ptr [esi + 0x30]        ; 0x008609DC
        movaps xmm6, xmmword ptr [esi + 0x60]       ; 0x008609E1
        movaps xmmword ptr [esi + 0xa0], xmm1       ; 0x008609E6
        movaps xmm1, xmm7                           ; 0x008609EE
        mulps xmm1, xmm5                            ; 0x008609F1
        addps xmm1, xmmword ptr [esi + 0x20]        ; 0x008609F4
        movaps xmmword ptr [esi + 0xc0], xmm1       ; 0x008609F9
        movaps xmm1, xmm7                           ; 0x00860A01
        mulps xmm1, xmm4                            ; 0x00860A04
        addps xmm1, xmmword ptr [esi + 0x70]        ; 0x00860A07
        movaps xmmword ptr [esi + 0x50], xmm1       ; 0x00860A0C
        mulps xmm7, xmm3                            ; 0x00860A11
        subps xmm7, xmm6                            ; 0x00860A14
        movaps xmmword ptr [esi + 0x20], xmm7       ; 0x00860A17
        movaps xmm7, xmmword ptr [esi + 0x40]       ; 0x00860A1C
        movaps xmm1, xmm7                           ; 0x00860A21
        mulps xmm1, xmm5                            ; 0x00860A24
        subps xmm1, xmmword ptr [esi + 0x30]        ; 0x00860A27
        movaps xmmword ptr [esi + 0x60], xmm1       ; 0x00860A2C
        movaps xmm1, xmm7                           ; 0x00860A31
        mulps xmm1, xmm4                            ; 0x00860A34
        addps xmm1, xmm6                            ; 0x00860A37
        movss xmm6, dword ptr [esi + 0x60]          ; 0x00860A3A
        movaps xmmword ptr [esi + 0x30], xmm1       ; 0x00860A40
        mulps xmm7, xmm3                            ; 0x00860A45
        addps xmm7, xmmword ptr [esi + 0x70]        ; 0x00860A48
        movaps xmmword ptr [esi + 0x40], xmm7       ; 0x00860A4D
        movss xmm1, dword ptr [esi + 0x40]          ; 0x00860A52
        movss xmm7, dword ptr [esi + 0xc0]          ; 0x00860A58
        movss dword ptr [esi + 0xc], xmm1           ; 0x00860A61
        movss dword ptr [esi], xmm1                 ; 0x00860A67
        movss xmm1, dword ptr [esi + 0x20]          ; 0x00860A6C
        movss dword ptr [esi + 8], xmm6             ; 0x00860A72
        movss xmm6, dword ptr [esi + 0x30]          ; 0x00860A78
        movss dword ptr [esi + 4], xmm6             ; 0x00860A7E
        movaps xmm6, xmmword ptr [esi]              ; 0x00860A84
        movss dword ptr [esi + 0xc], xmm1           ; 0x00860A88
        movss dword ptr [esi], xmm1                 ; 0x00860A8E
        movss xmm1, dword ptr [esi + 0xa0]          ; 0x00860A93
        movss dword ptr [esi + 8], xmm7             ; 0x00860A9C
        movss xmm7, dword ptr [esi + 0x50]          ; 0x00860AA2
        movss dword ptr [esi + 4], xmm7             ; 0x00860AA8
        movaps xmm7, xmmword ptr [esi]              ; 0x00860AAE
        movss dword ptr [esi + 0xc], xmm1           ; 0x00860AB2
        movss xmm1, dword ptr [esi + 0xb0]          ; 0x00860AB8
        movss dword ptr [esi + 8], xmm1             ; 0x00860AC1
        movss xmm1, dword ptr [esi + 0x80]          ; 0x00860AC7
        movss dword ptr [esi + 4], xmm1             ; 0x00860AD0
        movss xmm1, dword ptr [esi + 0xa0]          ; 0x00860AD6
        movaps xmmword ptr [esi + 0xe0], xmm6       ; 0x00860ADF
        movaps xmmword ptr [esi + 0xf0], xmm7       ; 0x00860AE7
        movss dword ptr [esi], xmm1                 ; 0x00860AEF
        movaps xmm1, xmm0                           ; 0x00860AF4
        shufps xmm1, xmm0, 0x55                     ; 0x00860AF7
        mulps xmm1, xmm7                            ; 0x00860AFB
        movaps xmm7, xmm0                           ; 0x00860AFE
        shufps xmm7, xmm0, 0                        ; 0x00860B01
        mulps xmm7, xmm6                            ; 0x00860B05
        movaps xmm6, xmm0                           ; 0x00860B08
        shufps xmm6, xmm0, 0xaa                     ; 0x00860B0B
        movaps xmm0, xmmword ptr [esi]              ; 0x00860B0F
        mulps xmm6, xmm0                            ; 0x00860B13
        addps xmm1, xmm7                            ; 0x00860B16
        addps xmm1, xmm6                            ; 0x00860B19
        movaps xmm6, xmmword ptr [esi + 0x130]      ; 0x00860B1C
        movaps xmm0, xmm6                           ; 0x00860B24
        shufps xmm0, xmm6, 0x55                     ; 0x00860B27
        mulps xmm0, xmmword ptr [esi + 0xf0]        ; 0x00860B2B
        movaps xmm7, xmm6                           ; 0x00860B33
        shufps xmm7, xmm6, 0                        ; 0x00860B36
        mulps xmm7, xmmword ptr [esi + 0xe0]        ; 0x00860B3A
        movaps xmm6, xmmword ptr [esi + 0x130]      ; 0x00860B42
        addps xmm0, xmm7                            ; 0x00860B4A
        movaps xmm7, xmm6                           ; 0x00860B4D
        shufps xmm7, xmm6, 0xaa                     ; 0x00860B50
        movaps xmmword ptr [esi + 0x80], xmm7       ; 0x00860B54
        movaps xmm7, xmmword ptr [esi]              ; 0x00860B5C
        movaps xmm6, xmmword ptr [esi + 0x80]       ; 0x00860B60
        mulps xmm6, xmm7                            ; 0x00860B68
        addps xmm0, xmm6                            ; 0x00860B6B
        movaps xmm6, xmmword ptr [esi + 0xe0]       ; 0x00860B6E
        mulps xmm6, xmm3                            ; 0x00860B76
        movaps xmm3, xmmword ptr [esi + 0xf0]       ; 0x00860B79
        mulps xmm3, xmm4                            ; 0x00860B81
        addps xmm6, xmm3                            ; 0x00860B84
        mulps xmm5, xmm7                            ; 0x00860B87
        addps xmm6, xmm5                            ; 0x00860B8A
        movaps xmm3, xmm6                           ; 0x00860B8D
        movaps xmm5, xmm0                           ; 0x00860B90
        movaps xmm7, xmm1                           ; 0x00860B93
        shufps xmm5, xmm0, 0x55                     ; 0x00860B96
        shufps xmm7, xmm1, 0                        ; 0x00860B9A
        movaps xmm6, xmm5                           ; 0x00860B9E
        addps xmm6, xmm7                            ; 0x00860BA1
        movaps xmm4, xmm3                           ; 0x00860BA4
        shufps xmm4, xmm3, 0xaa                     ; 0x00860BA7
        addps xmm6, xmm4                            ; 0x00860BAB
        movaps xmmword ptr [esi + 0x50], xmm6       ; 0x00860BAE
        movss dword ptr [esi + 0x80], xmm6          ; 0x00860BB3
        movss xmm6, dword ptr [esi + 0x110]         ; 0x00860BBC
        movss dword ptr [esi + 0xb0], xmm6          ; 0x00860BC5
        movss xmm6, dword ptr [esi + 0x80]          ; 0x00860BCE
        comiss xmm6, dword ptr [esi + 0xb0]         ; 0x00860BD7
        jbe L_00860C48                              ; 0x00860BDF
        movaps xmm4, xmmword ptr [esi + 0x50]       ; 0x00860BE1
        addps xmm4, xmmword ptr [esi + 0x90]        ; 0x00860BE6
        mov edx, dword ptr [arg3]                   ; 0x00860BEE
        sqrtps xmm7, xmm4                           ; 0x00860BF1
        movaps xmm5, xmm0                           ; 0x00860BF4
        shufps xmm5, xmm0, 0                        ; 0x00860BF7
        movaps xmm4, xmm1                           ; 0x00860BFB
        shufps xmm4, xmm1, 0x55                     ; 0x00860BFE
        subps xmm4, xmm5                            ; 0x00860C02
        movaps xmm5, xmm3                           ; 0x00860C05
        shufps xmm5, xmm3, 0                        ; 0x00860C08
        shufps xmm0, xmm0, 0xaa                     ; 0x00860C0C
        shufps xmm1, xmm1, 0xaa                     ; 0x00860C10
        movaps xmm6, xmm2                           ; 0x00860C14
        divps xmm6, xmm7                            ; 0x00860C17
        subps xmm5, xmm1                            ; 0x00860C1A
        shufps xmm3, xmm3, 0x55                     ; 0x00860C1D
        subps xmm0, xmm3                            ; 0x00860C21
        mulps xmm0, xmm6                            ; 0x00860C24
        mulps xmm5, xmm6                            ; 0x00860C27
        mulps xmm4, xmm6                            ; 0x00860C2A
        mulps xmm7, xmm2                            ; 0x00860C2D
        shufps xmm0, xmm5, 0x10                     ; 0x00860C30
        shufps xmm4, xmm7, 0x32                     ; 0x00860C34
        shufps xmm0, xmm4, 0x88                     ; 0x00860C38
        movaps xmm1, xmm0                           ; 0x00860C3C
        movaps xmmword ptr [edx], xmm1              ; 0x00860C3F
        mov byte ptr [result], 1                    ; 0x00860C42
        jmp L_done
        L_00860C48:
        movss dword ptr [esi + 0x50], xmm7          ; 0x00860C48
        movss xmm6, dword ptr [esi + 0x50]          ; 0x00860C4E
        movss dword ptr [esi + 0xc0], xmm5          ; 0x00860C54
        comiss xmm6, dword ptr [esi + 0xc0]         ; 0x00860C5D
        jbe L_00860CEF                              ; 0x00860C65
        movss dword ptr [esi + 0x40], xmm7          ; 0x00860C6B
        movss xmm6, dword ptr [esi + 0x40]          ; 0x00860C71
        movss dword ptr [esi + 0x70], xmm4          ; 0x00860C77
        comiss xmm6, dword ptr [esi + 0x70]         ; 0x00860C7D
        jbe L_00860CEF                              ; 0x00860C82
        mov edx, dword ptr [arg3]                   ; 0x00860C84
        addps xmm4, xmm5                            ; 0x00860C87
        subps xmm7, xmm4                            ; 0x00860C8A
        movaps xmm4, xmmword ptr [esi + 0x90]       ; 0x00860C8D
        addps xmm4, xmm7                            ; 0x00860C95
        sqrtps xmm7, xmm4                           ; 0x00860C98
        movaps xmm4, xmm3                           ; 0x00860C9B
        shufps xmm4, xmm3, 0x55                     ; 0x00860C9E
        movaps xmm5, xmm0                           ; 0x00860CA2
        shufps xmm5, xmm0, 0xaa                     ; 0x00860CA5
        subps xmm5, xmm4                            ; 0x00860CA9
        movaps xmm4, xmm1                           ; 0x00860CAC
        shufps xmm4, xmm1, 0xaa                     ; 0x00860CAF
        movaps xmm6, xmm2                           ; 0x00860CB3
        shufps xmm1, xmm1, 0x55                     ; 0x00860CB6
        divps xmm6, xmm7                            ; 0x00860CBA
        shufps xmm3, xmm3, 0                        ; 0x00860CBD
        addps xmm4, xmm3                            ; 0x00860CC1
        shufps xmm0, xmm0, 0                        ; 0x00860CC4
        addps xmm1, xmm0                            ; 0x00860CC8
        mulps xmm2, xmm7                            ; 0x00860CCB
        mulps xmm1, xmm6                            ; 0x00860CCE
        shufps xmm2, xmm1, 0x10                     ; 0x00860CD1
        mulps xmm4, xmm6                            ; 0x00860CD5
        mulps xmm5, xmm6                            ; 0x00860CD8
        shufps xmm4, xmm5, 0x32                     ; 0x00860CDB
        shufps xmm2, xmm4, 0x88                     ; 0x00860CDF
        movaps xmm1, xmm2                           ; 0x00860CE3
        movaps xmmword ptr [edx], xmm1              ; 0x00860CE6
        mov byte ptr [result], 1                    ; 0x00860CE9
        jmp L_done
        L_00860CEF:
        movss dword ptr [esi], xmm5                 ; 0x00860CEF
        movss xmm6, dword ptr [esi]                 ; 0x00860CF4
        movss dword ptr [esi + 0xd0], xmm4          ; 0x00860CF9
        comiss xmm6, dword ptr [esi + 0xd0]         ; 0x00860D02
        jbe L_00860D74                              ; 0x00860D0A
        mov edx, dword ptr [arg3]                   ; 0x00860D0C
        addps xmm4, xmm7                            ; 0x00860D0F
        subps xmm5, xmm4                            ; 0x00860D12
        movaps xmm4, xmmword ptr [esi + 0x90]       ; 0x00860D15
        addps xmm4, xmm5                            ; 0x00860D1D
        sqrtps xmm6, xmm4                           ; 0x00860D20
        movaps xmm7, xmm1                           ; 0x00860D23
        shufps xmm7, xmm1, 0xaa                     ; 0x00860D26
        movaps xmm4, xmm3                           ; 0x00860D2A
        shufps xmm4, xmm3, 0                        ; 0x00860D2D
        subps xmm4, xmm7                            ; 0x00860D31
        movaps xmm7, xmm0                           ; 0x00860D34
        shufps xmm1, xmm1, 0x55                     ; 0x00860D37
        shufps xmm3, xmm3, 0x55                     ; 0x00860D3B
        shufps xmm7, xmm0, 0xaa                     ; 0x00860D3F
        movaps xmm5, xmm2                           ; 0x00860D43
        divps xmm5, xmm6                            ; 0x00860D46
        addps xmm3, xmm7                            ; 0x00860D49
        shufps xmm0, xmm0, 0                        ; 0x00860D4C
        addps xmm1, xmm0                            ; 0x00860D50
        mulps xmm1, xmm5                            ; 0x00860D53
        mulps xmm2, xmm6                            ; 0x00860D56
        mulps xmm3, xmm5                            ; 0x00860D59
        mulps xmm4, xmm5                            ; 0x00860D5C
        shufps xmm1, xmm2, 0x10                     ; 0x00860D5F
        shufps xmm3, xmm4, 0x32                     ; 0x00860D63
        shufps xmm1, xmm3, 0x88                     ; 0x00860D67
        movaps xmmword ptr [edx], xmm1              ; 0x00860D6B
        mov byte ptr [result], 1                    ; 0x00860D6E
        jmp L_done
        L_00860D74:
        mov edx, dword ptr [arg3]                   ; 0x00860D74
        addps xmm5, xmm7                            ; 0x00860D77
        subps xmm4, xmm5                            ; 0x00860D7A
        movaps xmm5, xmmword ptr [esi + 0x90]       ; 0x00860D7D
        addps xmm5, xmm4                            ; 0x00860D85
        sqrtps xmm7, xmm5                           ; 0x00860D88
        movaps xmm4, xmm1                           ; 0x00860D8B
        shufps xmm4, xmm1, 0x55                     ; 0x00860D8E
        movaps xmm5, xmm0                           ; 0x00860D92
        shufps xmm5, xmm0, 0                        ; 0x00860D95
        subps xmm4, xmm5                            ; 0x00860D99
        movaps xmm5, xmm3                           ; 0x00860D9C
        shufps xmm5, xmm3, 0x55                     ; 0x00860D9F
        shufps xmm1, xmm1, 0xaa                     ; 0x00860DA3
        movaps xmm6, xmm2                           ; 0x00860DA7
        divps xmm6, xmm7                            ; 0x00860DAA
        shufps xmm0, xmm0, 0xaa                     ; 0x00860DAD
        shufps xmm3, xmm3, 0                        ; 0x00860DB1
        addps xmm1, xmm3                            ; 0x00860DB5
        addps xmm5, xmm0                            ; 0x00860DB8
        mulps xmm1, xmm6                            ; 0x00860DBB
        mulps xmm5, xmm6                            ; 0x00860DBE
        mulps xmm2, xmm7                            ; 0x00860DC1
        mulps xmm4, xmm6                            ; 0x00860DC4
        shufps xmm1, xmm5, 0x10                     ; 0x00860DC7
        shufps xmm2, xmm4, 0x32                     ; 0x00860DCB
        shufps xmm1, xmm2, 0x88                     ; 0x00860DCF
        movaps xmmword ptr [edx], xmm1              ; 0x00860DD3
        mov byte ptr [result], 1                    ; 0x00860DD6
        jmp L_done
        L_00860DDC:
        mov byte ptr [result], 0                    ; 0x00860DDC
        jmp L_done
        pop esi
L_done:
    }
    return result;
}
#else  // x64: portable C++ transcription (same op order as the listing)
int8_t QuaternionFromAxisAngle(const void* a1, const void* a2, void* out)
{
    __m128 st[21];
    std::memset(st, 0, sizeof st);
    __m128 xmm0 = _mm_setzero_ps(), xmm1 = _mm_setzero_ps(), xmm2 = _mm_setzero_ps(),
           xmm3 = _mm_setzero_ps(), xmm4 = _mm_setzero_ps(), xmm5 = _mm_setzero_ps(),
           xmm6 = _mm_setzero_ps(), xmm7 = _mm_setzero_ps();
    float x87 = 0.0f;
    (void)x87;

// clang-format off
    /* 0x008607CC*/
    xmm4 = _mm_load_ps(reinterpret_cast<const float*>(a1)); /* 0x008607CF*/
    xmm5 = _mm_setzero_ps(); /* 0x008607D2  xorps xmm5, xmm5 */
    xmm1 = xmm4; /* 0x008607D5*/
    xmm1 = _mm_mul_ps(xmm1, xmm4); /* 0x008607D8  mulps xmm1, xmm4 */
    xmm0 = xmm1; /* 0x008607DB*/
    xmm0 = _mm_shuffle_ps(xmm0, xmm1, 2); /* 0x008607DE  shufps xmm0, xmm1, 2 */
    xmm2 = xmm1; /* 0x008607E2*/
    xmm2 = _mm_shuffle_ps(xmm2, xmm1, 1); /* 0x008607E5  shufps xmm2, xmm1, 1 */
    xmm0 = _mm_add_ps(xmm0, xmm2); /* 0x008607E9  addps xmm0, xmm2 */
    xmm0 = _mm_add_ps(xmm0, xmm1); /* 0x008607EC  addps xmm0, xmm1 */
    xmm0 = _mm_shuffle_ps(xmm0, xmm0, 0); /* 0x008607EF  shufps xmm0, xmm0, 0 */
    st[10].m128_f32[0] = xmm0.m128_f32[0]; /* 0x008607F3*/
    xmm0.m128_f32[0] = xmm5.m128_f32[0]; /* 0x008607FC*/
    xmm0 = _mm_shuffle_ps(xmm0, xmm0, 0); /* 0x00860800  shufps xmm0, xmm0, 0 */
    st[3].m128_f32[0] = xmm0.m128_f32[0]; /* 0x00860804*/
    st[17] = xmm0; /* 0x0086080A*/
    xmm0.m128_f32[0] = st[10].m128_f32[0]; /* 0x00860812*/
    // The game branches with jbe after comiss: taken on less, equal AND unordered (NaN).
    // MSVC's _mm_comile_ss is an ordered comparison (0 on unordered, like !comigt with
    // swapped operands), so jbe is expressed exactly as !_mm_comigt_ss (CF=0 && ZF=0 is
    // precisely ja): for every input pair, including NaNs, both agree with the original.
    if (!_mm_comigt_ss(xmm0, _mm_set1_ps(st[3].m128_f32[0]))) goto L_860DDC; /* 0x0086081B*/
    xmm6.m128_f32[0] = 1.0f; /* 0x00860826*/
    xmm1 = xmm4; /* 0x0086082E*/
    xmm1 = _mm_mul_ps(xmm1, xmm4); /* 0x00860831  mulps xmm1, xmm4 */
    xmm0 = xmm1; /* 0x00860834*/
    xmm0 = _mm_shuffle_ps(xmm0, xmm1, 2); /* 0x00860837  shufps xmm0, xmm1, 2 */
    xmm2 = xmm1; /* 0x0086083B*/
    xmm2 = _mm_shuffle_ps(xmm2, xmm1, 1); /* 0x0086083E  shufps xmm2, xmm1, 1 */
    xmm0 = _mm_add_ps(xmm0, xmm2); /* 0x00860842  addps xmm0, xmm2 */
    xmm2.m128_f32[0] = 0.5f; /* 0x00860845*/
    xmm0 = _mm_add_ps(xmm0, xmm1); /* 0x0086084D  addps xmm0, xmm1 */
    xmm0 = _mm_shuffle_ps(xmm0, xmm0, 0); /* 0x00860850  shufps xmm0, xmm0, 0 */
    xmm1 = _mm_rsqrt_ps(xmm0); /* 0x00860854  rsqrtps xmm1, xmm0 */
    xmm7 = xmm1; /* 0x00860857*/
    xmm7 = _mm_mul_ps(xmm7, xmm1); /* 0x0086085A  mulps xmm7, xmm1 */
    xmm7 = _mm_mul_ps(xmm7, xmm0); /* 0x0086085D  mulps xmm7, xmm0 */
    /* 0x00860860*/
    xmm6 = _mm_shuffle_ps(xmm6, xmm6, 0); /* 0x00860863  shufps xmm6, xmm6, 0 */
    xmm3 = xmm6; /* 0x00860867*/
    xmm3 = _mm_sub_ps(xmm3, xmm7); /* 0x0086086A  subps xmm3, xmm7 */
    xmm2 = _mm_shuffle_ps(xmm2, xmm2, 0); /* 0x0086086D  shufps xmm2, xmm2, 0 */
    xmm7 = xmm2; /* 0x00860871*/
    xmm7 = _mm_mul_ps(xmm7, xmm1); /* 0x00860874  mulps xmm7, xmm1 */
    xmm3 = _mm_mul_ps(xmm3, xmm7); /* 0x00860877  mulps xmm3, xmm7 */
    xmm3 = _mm_add_ps(xmm3, xmm1); /* 0x0086087A  addps xmm3, xmm1 */
    xmm7 = xmm3; /* 0x0086087D*/
    xmm7 = _mm_mul_ps(xmm7, xmm3); /* 0x00860880  mulps xmm7, xmm3 */
    xmm7 = _mm_mul_ps(xmm7, xmm0); /* 0x00860883  mulps xmm7, xmm0 */
    st[0].m128_f32[3] = xmm5.m128_f32[0]; /* 0x00860886*/
    st[0].m128_f32[2] = xmm5.m128_f32[0]; /* 0x0086088C*/
    st[0].m128_f32[0] = xmm5.m128_f32[0]; /* 0x00860892*/
    xmm1 = xmm6; /* 0x00860897*/
    xmm1 = _mm_sub_ps(xmm1, xmm7); /* 0x0086089A  subps xmm1, xmm7 */
    xmm0 = xmm2; /* 0x0086089D*/
    xmm0 = _mm_mul_ps(xmm0, xmm3); /* 0x008608A0  mulps xmm0, xmm3 */
    xmm1 = _mm_mul_ps(xmm1, xmm0); /* 0x008608A3  mulps xmm1, xmm0 */
    xmm0.m128_f32[0] = 1.0f; /* 0x008608A6*/
    xmm1 = _mm_add_ps(xmm1, xmm3); /* 0x008608AE  addps xmm1, xmm3 */
    xmm1 = _mm_mul_ps(xmm1, xmm4); /* 0x008608B1  mulps xmm1, xmm4 */
    st[0].m128_f32[1] = xmm0.m128_f32[0]; /* 0x008608B4*/
    xmm0 = st[0]; /* 0x008608BA*/
    xmm3 = xmm1; /* 0x008608BE*/
    xmm3 = _mm_shuffle_ps(xmm3, xmm1, 0x12); /* 0x008608C1  shufps xmm3, xmm1, 0x12 */
    xmm4 = xmm1; /* 0x008608C5*/
    xmm4 = _mm_shuffle_ps(xmm4, xmm1, 9); /* 0x008608C8  shufps xmm4, xmm1, 9 */
    xmm5 = xmm0; /* 0x008608CC*/
    xmm5 = _mm_shuffle_ps(xmm5, xmm0, 0x12); /* 0x008608CF  shufps xmm5, xmm0, 0x12 */
    xmm5 = _mm_mul_ps(xmm5, xmm4); /* 0x008608D3  mulps xmm5, xmm4 */
    xmm0 = _mm_shuffle_ps(xmm0, xmm0, 9); /* 0x008608D6  shufps xmm0, xmm0, 9 */
    xmm0 = _mm_mul_ps(xmm0, xmm3); /* 0x008608DA  mulps xmm0, xmm3 */
    xmm0 = _mm_sub_ps(xmm0, xmm5); /* 0x008608DD  subps xmm0, xmm5 */
    xmm5 = xmm0; /* 0x008608E0*/
    xmm5 = _mm_shuffle_ps(xmm5, xmm0, 0x12); /* 0x008608E3  shufps xmm5, xmm0, 0x12 */
    xmm5 = _mm_mul_ps(xmm5, xmm4); /* 0x008608E7  mulps xmm5, xmm4 */
    st[6] = xmm1; /* 0x008608EA*/
    st[20] = xmm1; /* 0x008608EF*/
    xmm4 = xmm0; /* 0x008608F7*/
    xmm4 = _mm_shuffle_ps(xmm4, xmm0, 9); /* 0x008608FA  shufps xmm4, xmm0, 9 */
    xmm4 = _mm_mul_ps(xmm4, xmm3); /* 0x008608FE  mulps xmm4, xmm3 */
    st[18] = xmm0; /* 0x00860901*/
    xmm5 = _mm_sub_ps(xmm5, xmm4); /* 0x00860909  subps xmm5, xmm4 */
    st[19] = xmm5; /* 0x0086090C*/
    xmm1 = _mm_load_ps(reinterpret_cast<const float*>(a2)); /* 0x00860914*/
    xmm1 = _mm_shuffle_ps(xmm1, xmm1, 0xaa); /* 0x00860917  shufps xmm1, xmm1, 0xaa */
    xmm3 = xmm1; /* 0x0086091B*/
    st[2] = xmm3; /* 0x0086091E*/
    x87 = st[2].m128_f32[0]; /* 0x00860923*/
    st[2] = xmm3; /* 0x00860927*/
    x87 = X87Sin(x87);
    st[9] = xmm6; /* 0x0086092E*/
    st[1].m128_f32[3] = x87; /* 0x00860936*/
    x87 = st[2].m128_f32[0]; /* 0x0086093A*/
    x87 = X87Cos(x87);
    xmm1.m128_f32[0] = st[1].m128_f32[3]; /* 0x00860940*/
    xmm1 = _mm_shuffle_ps(xmm1, xmm1, 0); /* 0x00860946  shufps xmm1, xmm1, 0 */
    st[1].m128_f32[3] = x87; /* 0x0086094A*/
    xmm3.m128_f32[0] = st[1].m128_f32[3]; /* 0x0086094E*/
    xmm5 = st[6]; /* 0x00860954*/
    xmm3 = _mm_shuffle_ps(xmm3, xmm3, 0); /* 0x00860959  shufps xmm3, xmm3, 0 */
    st[7] = xmm3; /* 0x0086095D*/
    xmm6 = _mm_sub_ps(xmm6, xmm3); /* 0x00860962  subps xmm6, xmm3 */
    xmm3 = xmm5; /* 0x00860965*/
    xmm4 = xmm6; /* 0x00860968*/
    xmm3 = _mm_shuffle_ps(xmm3, xmm5, 0); /* 0x0086096B  shufps xmm3, xmm5, 0 */
    xmm7 = xmm6; /* 0x0086096F*/
    xmm4 = _mm_mul_ps(xmm4, xmm3); /* 0x00860972  mulps xmm4, xmm3 */
    st[4] = xmm4; /* 0x00860975*/
    xmm4 = xmm5; /* 0x0086097A*/
    xmm4 = _mm_shuffle_ps(xmm4, xmm5, 0x55); /* 0x0086097D  shufps xmm4, xmm5, 0x55 */
    xmm5 = _mm_shuffle_ps(xmm5, xmm5, 0xaa); /* 0x00860981  shufps xmm5, xmm5, 0xaa */
    xmm6 = _mm_mul_ps(xmm6, xmm5); /* 0x00860985  mulps xmm6, xmm5 */
    st[0] = xmm6; /* 0x00860988*/
    xmm6 = xmm1; /* 0x0086098C*/
    xmm6 = _mm_mul_ps(xmm6, xmm3); /* 0x0086098F  mulps xmm6, xmm3 */
    st[2] = xmm6; /* 0x00860992*/
    xmm6 = xmm1; /* 0x00860997*/
    xmm1 = _mm_mul_ps(xmm1, xmm5); /* 0x0086099A  mulps xmm1, xmm5 */
    st[6] = xmm1; /* 0x0086099D*/
    xmm1 = st[0]; /* 0x008609A2*/
    xmm1 = _mm_mul_ps(xmm1, xmm5); /* 0x008609A6  mulps xmm1, xmm5 */
    xmm1 = _mm_add_ps(xmm1, st[7]); /* 0x008609A9  addps xmm1, xmmword ptr [esp + 0x70] */
    st[11] = xmm1; /* 0x008609AE*/
    xmm1 = st[0]; /* 0x008609B6*/
    xmm1 = _mm_mul_ps(xmm1, xmm4); /* 0x008609BA  mulps xmm1, xmm4 */
    xmm1 = _mm_sub_ps(xmm1, st[2]); /* 0x008609BD  subps xmm1, xmmword ptr [esp + 0x20] */
    st[8] = xmm1; /* 0x008609C2*/
    xmm1 = st[0]; /* 0x008609CA*/
    xmm1 = _mm_mul_ps(xmm1, xmm3); /* 0x008609CE  mulps xmm1, xmm3 */
    xmm7 = _mm_mul_ps(xmm7, xmm4); /* 0x008609D1  mulps xmm7, xmm4 */
    xmm6 = _mm_mul_ps(xmm6, xmm4); /* 0x008609D4  mulps xmm6, xmm4 */
    st[3] = xmm6; /* 0x008609D7*/
    xmm1 = _mm_add_ps(xmm1, st[3]); /* 0x008609DC  addps xmm1, xmmword ptr [esp + 0x30] */
    xmm6 = st[6]; /* 0x008609E1*/
    st[10] = xmm1; /* 0x008609E6*/
    xmm1 = xmm7; /* 0x008609EE*/
    xmm1 = _mm_mul_ps(xmm1, xmm5); /* 0x008609F1  mulps xmm1, xmm5 */
    xmm1 = _mm_add_ps(xmm1, st[2]); /* 0x008609F4  addps xmm1, xmmword ptr [esp + 0x20] */
    st[12] = xmm1; /* 0x008609F9*/
    xmm1 = xmm7; /* 0x00860A01*/
    xmm1 = _mm_mul_ps(xmm1, xmm4); /* 0x00860A04  mulps xmm1, xmm4 */
    xmm1 = _mm_add_ps(xmm1, st[7]); /* 0x00860A07  addps xmm1, xmmword ptr [esp + 0x70] */
    st[5] = xmm1; /* 0x00860A0C*/
    xmm7 = _mm_mul_ps(xmm7, xmm3); /* 0x00860A11  mulps xmm7, xmm3 */
    xmm7 = _mm_sub_ps(xmm7, xmm6); /* 0x00860A14  subps xmm7, xmm6 */
    st[2] = xmm7; /* 0x00860A17*/
    xmm7 = st[4]; /* 0x00860A1C*/
    xmm1 = xmm7; /* 0x00860A21*/
    xmm1 = _mm_mul_ps(xmm1, xmm5); /* 0x00860A24  mulps xmm1, xmm5 */
    xmm1 = _mm_sub_ps(xmm1, st[3]); /* 0x00860A27  subps xmm1, xmmword ptr [esp + 0x30] */
    st[6] = xmm1; /* 0x00860A2C*/
    xmm1 = xmm7; /* 0x00860A31*/
    xmm1 = _mm_mul_ps(xmm1, xmm4); /* 0x00860A34  mulps xmm1, xmm4 */
    xmm1 = _mm_add_ps(xmm1, xmm6); /* 0x00860A37  addps xmm1, xmm6 */
    xmm6.m128_f32[0] = st[6].m128_f32[0]; /* 0x00860A3A*/
    st[3] = xmm1; /* 0x00860A40*/
    xmm7 = _mm_mul_ps(xmm7, xmm3); /* 0x00860A45  mulps xmm7, xmm3 */
    xmm7 = _mm_add_ps(xmm7, st[7]); /* 0x00860A48  addps xmm7, xmmword ptr [esp + 0x70] */
    st[4] = xmm7; /* 0x00860A4D*/
    xmm1.m128_f32[0] = st[4].m128_f32[0]; /* 0x00860A52*/
    xmm7.m128_f32[0] = st[12].m128_f32[0]; /* 0x00860A58*/
    st[0].m128_f32[3] = xmm1.m128_f32[0]; /* 0x00860A61*/
    st[0].m128_f32[0] = xmm1.m128_f32[0]; /* 0x00860A67*/
    xmm1.m128_f32[0] = st[2].m128_f32[0]; /* 0x00860A6C*/
    st[0].m128_f32[2] = xmm6.m128_f32[0]; /* 0x00860A72*/
    xmm6.m128_f32[0] = st[3].m128_f32[0]; /* 0x00860A78*/
    st[0].m128_f32[1] = xmm6.m128_f32[0]; /* 0x00860A7E*/
    xmm6 = st[0]; /* 0x00860A84*/
    st[0].m128_f32[3] = xmm1.m128_f32[0]; /* 0x00860A88*/
    st[0].m128_f32[0] = xmm1.m128_f32[0]; /* 0x00860A8E*/
    xmm1.m128_f32[0] = st[10].m128_f32[0]; /* 0x00860A93*/
    st[0].m128_f32[2] = xmm7.m128_f32[0]; /* 0x00860A9C*/
    xmm7.m128_f32[0] = st[5].m128_f32[0]; /* 0x00860AA2*/
    st[0].m128_f32[1] = xmm7.m128_f32[0]; /* 0x00860AA8*/
    xmm7 = st[0]; /* 0x00860AAE*/
    st[0].m128_f32[3] = xmm1.m128_f32[0]; /* 0x00860AB2*/
    xmm1.m128_f32[0] = st[11].m128_f32[0]; /* 0x00860AB8*/
    st[0].m128_f32[2] = xmm1.m128_f32[0]; /* 0x00860AC1*/
    xmm1.m128_f32[0] = st[8].m128_f32[0]; /* 0x00860AC7*/
    st[0].m128_f32[1] = xmm1.m128_f32[0]; /* 0x00860AD0*/
    xmm1.m128_f32[0] = st[10].m128_f32[0]; /* 0x00860AD6*/
    st[14] = xmm6; /* 0x00860ADF*/
    st[15] = xmm7; /* 0x00860AE7*/
    st[0].m128_f32[0] = xmm1.m128_f32[0]; /* 0x00860AEF*/
    xmm1 = xmm0; /* 0x00860AF4*/
    xmm1 = _mm_shuffle_ps(xmm1, xmm0, 0x55); /* 0x00860AF7  shufps xmm1, xmm0, 0x55 */
    xmm1 = _mm_mul_ps(xmm1, xmm7); /* 0x00860AFB  mulps xmm1, xmm7 */
    xmm7 = xmm0; /* 0x00860AFE*/
    xmm7 = _mm_shuffle_ps(xmm7, xmm0, 0); /* 0x00860B01  shufps xmm7, xmm0, 0 */
    xmm7 = _mm_mul_ps(xmm7, xmm6); /* 0x00860B05  mulps xmm7, xmm6 */
    xmm6 = xmm0; /* 0x00860B08*/
    xmm6 = _mm_shuffle_ps(xmm6, xmm0, 0xaa); /* 0x00860B0B  shufps xmm6, xmm0, 0xaa */
    xmm0 = st[0]; /* 0x00860B0F*/
    xmm6 = _mm_mul_ps(xmm6, xmm0); /* 0x00860B13  mulps xmm6, xmm0 */
    xmm1 = _mm_add_ps(xmm1, xmm7); /* 0x00860B16  addps xmm1, xmm7 */
    xmm1 = _mm_add_ps(xmm1, xmm6); /* 0x00860B19  addps xmm1, xmm6 */
    xmm6 = st[19]; /* 0x00860B1C*/
    xmm0 = xmm6; /* 0x00860B24*/
    xmm0 = _mm_shuffle_ps(xmm0, xmm6, 0x55); /* 0x00860B27  shufps xmm0, xmm6, 0x55 */
    xmm0 = _mm_mul_ps(xmm0, st[15]); /* 0x00860B2B  mulps xmm0, xmmword ptr [esp + 0xf0] */
    xmm7 = xmm6; /* 0x00860B33*/
    xmm7 = _mm_shuffle_ps(xmm7, xmm6, 0); /* 0x00860B36  shufps xmm7, xmm6, 0 */
    xmm7 = _mm_mul_ps(xmm7, st[14]); /* 0x00860B3A  mulps xmm7, xmmword ptr [esp + 0xe0] */
    xmm6 = st[19]; /* 0x00860B42*/
    xmm0 = _mm_add_ps(xmm0, xmm7); /* 0x00860B4A  addps xmm0, xmm7 */
    xmm7 = xmm6; /* 0x00860B4D*/
    xmm7 = _mm_shuffle_ps(xmm7, xmm6, 0xaa); /* 0x00860B50  shufps xmm7, xmm6, 0xaa */
    st[8] = xmm7; /* 0x00860B54*/
    xmm7 = st[0]; /* 0x00860B5C*/
    xmm6 = st[8]; /* 0x00860B60*/
    xmm6 = _mm_mul_ps(xmm6, xmm7); /* 0x00860B68  mulps xmm6, xmm7 */
    xmm0 = _mm_add_ps(xmm0, xmm6); /* 0x00860B6B  addps xmm0, xmm6 */
    xmm6 = st[14]; /* 0x00860B6E*/
    xmm6 = _mm_mul_ps(xmm6, xmm3); /* 0x00860B76  mulps xmm6, xmm3 */
    xmm3 = st[15]; /* 0x00860B79*/
    xmm3 = _mm_mul_ps(xmm3, xmm4); /* 0x00860B81  mulps xmm3, xmm4 */
    xmm6 = _mm_add_ps(xmm6, xmm3); /* 0x00860B84  addps xmm6, xmm3 */
    xmm5 = _mm_mul_ps(xmm5, xmm7); /* 0x00860B87  mulps xmm5, xmm7 */
    xmm6 = _mm_add_ps(xmm6, xmm5); /* 0x00860B8A  addps xmm6, xmm5 */
    xmm3 = xmm6; /* 0x00860B8D*/
    xmm5 = xmm0; /* 0x00860B90*/
    xmm7 = xmm1; /* 0x00860B93*/
    xmm5 = _mm_shuffle_ps(xmm5, xmm0, 0x55); /* 0x00860B96  shufps xmm5, xmm0, 0x55 */
    xmm7 = _mm_shuffle_ps(xmm7, xmm1, 0); /* 0x00860B9A  shufps xmm7, xmm1, 0 */
    xmm6 = xmm5; /* 0x00860B9E*/
    xmm6 = _mm_add_ps(xmm6, xmm7); /* 0x00860BA1  addps xmm6, xmm7 */
    xmm4 = xmm3; /* 0x00860BA4*/
    xmm4 = _mm_shuffle_ps(xmm4, xmm3, 0xaa); /* 0x00860BA7  shufps xmm4, xmm3, 0xaa */
    xmm6 = _mm_add_ps(xmm6, xmm4); /* 0x00860BAB  addps xmm6, xmm4 */
    st[5] = xmm6; /* 0x00860BAE*/
    st[8].m128_f32[0] = xmm6.m128_f32[0]; /* 0x00860BB3*/
    xmm6.m128_f32[0] = st[17].m128_f32[0]; /* 0x00860BBC*/
    st[11].m128_f32[0] = xmm6.m128_f32[0]; /* 0x00860BC5*/
    xmm6.m128_f32[0] = st[8].m128_f32[0]; /* 0x00860BCE*/
    if (!_mm_comigt_ss(xmm6, _mm_set1_ps(st[11].m128_f32[0]))) goto L_860C48; /* 0x00860BD7*/
    xmm4 = st[5]; /* 0x00860BE1*/
    xmm4 = _mm_add_ps(xmm4, st[9]); /* 0x00860BE6  addps xmm4, xmmword ptr [esp + 0x90] */
    /* 0x00860BEE*/
    xmm7 = _mm_sqrt_ps(xmm4); /* 0x00860BF1  sqrtps xmm7, xmm4 */
    xmm5 = xmm0; /* 0x00860BF4*/
    xmm5 = _mm_shuffle_ps(xmm5, xmm0, 0); /* 0x00860BF7  shufps xmm5, xmm0, 0 */
    xmm4 = xmm1; /* 0x00860BFB*/
    xmm4 = _mm_shuffle_ps(xmm4, xmm1, 0x55); /* 0x00860BFE  shufps xmm4, xmm1, 0x55 */
    xmm4 = _mm_sub_ps(xmm4, xmm5); /* 0x00860C02  subps xmm4, xmm5 */
    xmm5 = xmm3; /* 0x00860C05*/
    xmm5 = _mm_shuffle_ps(xmm5, xmm3, 0); /* 0x00860C08  shufps xmm5, xmm3, 0 */
    xmm0 = _mm_shuffle_ps(xmm0, xmm0, 0xaa); /* 0x00860C0C  shufps xmm0, xmm0, 0xaa */
    xmm1 = _mm_shuffle_ps(xmm1, xmm1, 0xaa); /* 0x00860C10  shufps xmm1, xmm1, 0xaa */
    xmm6 = xmm2; /* 0x00860C14*/
    xmm6 = _mm_div_ps(xmm6, xmm7); /* 0x00860C17  divps xmm6, xmm7 */
    xmm5 = _mm_sub_ps(xmm5, xmm1); /* 0x00860C1A  subps xmm5, xmm1 */
    xmm3 = _mm_shuffle_ps(xmm3, xmm3, 0x55); /* 0x00860C1D  shufps xmm3, xmm3, 0x55 */
    xmm0 = _mm_sub_ps(xmm0, xmm3); /* 0x00860C21  subps xmm0, xmm3 */
    xmm0 = _mm_mul_ps(xmm0, xmm6); /* 0x00860C24  mulps xmm0, xmm6 */
    xmm5 = _mm_mul_ps(xmm5, xmm6); /* 0x00860C27  mulps xmm5, xmm6 */
    xmm4 = _mm_mul_ps(xmm4, xmm6); /* 0x00860C2A  mulps xmm4, xmm6 */
    xmm7 = _mm_mul_ps(xmm7, xmm2); /* 0x00860C2D  mulps xmm7, xmm2 */
    xmm0 = _mm_shuffle_ps(xmm0, xmm5, 0x10); /* 0x00860C30  shufps xmm0, xmm5, 0x10 */
    xmm4 = _mm_shuffle_ps(xmm4, xmm7, 0x32); /* 0x00860C34  shufps xmm4, xmm7, 0x32 */
    xmm0 = _mm_shuffle_ps(xmm0, xmm4, 0x88); /* 0x00860C38  shufps xmm0, xmm4, 0x88 */
    xmm1 = xmm0; /* 0x00860C3C*/
    _mm_store_ps(reinterpret_cast<float*>(out), xmm1); /* 0x00860C3F*/
    return 1;
  L_860C48:
    st[5].m128_f32[0] = xmm7.m128_f32[0]; /* 0x00860C48*/
    xmm6.m128_f32[0] = st[5].m128_f32[0]; /* 0x00860C4E*/
    st[12].m128_f32[0] = xmm5.m128_f32[0]; /* 0x00860C54*/
    if (!_mm_comigt_ss(xmm6, _mm_set1_ps(st[12].m128_f32[0]))) goto L_860CEF; /* 0x00860C5D*/
    st[4].m128_f32[0] = xmm7.m128_f32[0]; /* 0x00860C6B*/
    xmm6.m128_f32[0] = st[4].m128_f32[0]; /* 0x00860C71*/
    st[7].m128_f32[0] = xmm4.m128_f32[0]; /* 0x00860C77*/
    if (!_mm_comigt_ss(xmm6, _mm_set1_ps(st[7].m128_f32[0]))) goto L_860CEF; /* 0x00860C7D*/
    /* 0x00860C84*/
    xmm4 = _mm_add_ps(xmm4, xmm5); /* 0x00860C87  addps xmm4, xmm5 */
    xmm7 = _mm_sub_ps(xmm7, xmm4); /* 0x00860C8A  subps xmm7, xmm4 */
    xmm4 = st[9]; /* 0x00860C8D*/
    xmm4 = _mm_add_ps(xmm4, xmm7); /* 0x00860C95  addps xmm4, xmm7 */
    xmm7 = _mm_sqrt_ps(xmm4); /* 0x00860C98  sqrtps xmm7, xmm4 */
    xmm4 = xmm3; /* 0x00860C9B*/
    xmm4 = _mm_shuffle_ps(xmm4, xmm3, 0x55); /* 0x00860C9E  shufps xmm4, xmm3, 0x55 */
    xmm5 = xmm0; /* 0x00860CA2*/
    xmm5 = _mm_shuffle_ps(xmm5, xmm0, 0xaa); /* 0x00860CA5  shufps xmm5, xmm0, 0xaa */
    xmm5 = _mm_sub_ps(xmm5, xmm4); /* 0x00860CA9  subps xmm5, xmm4 */
    xmm4 = xmm1; /* 0x00860CAC*/
    xmm4 = _mm_shuffle_ps(xmm4, xmm1, 0xaa); /* 0x00860CAF  shufps xmm4, xmm1, 0xaa */
    xmm6 = xmm2; /* 0x00860CB3*/
    xmm1 = _mm_shuffle_ps(xmm1, xmm1, 0x55); /* 0x00860CB6  shufps xmm1, xmm1, 0x55 */
    xmm6 = _mm_div_ps(xmm6, xmm7); /* 0x00860CBA  divps xmm6, xmm7 */
    xmm3 = _mm_shuffle_ps(xmm3, xmm3, 0); /* 0x00860CBD  shufps xmm3, xmm3, 0 */
    xmm4 = _mm_add_ps(xmm4, xmm3); /* 0x00860CC1  addps xmm4, xmm3 */
    xmm0 = _mm_shuffle_ps(xmm0, xmm0, 0); /* 0x00860CC4  shufps xmm0, xmm0, 0 */
    xmm1 = _mm_add_ps(xmm1, xmm0); /* 0x00860CC8  addps xmm1, xmm0 */
    xmm2 = _mm_mul_ps(xmm2, xmm7); /* 0x00860CCB  mulps xmm2, xmm7 */
    xmm1 = _mm_mul_ps(xmm1, xmm6); /* 0x00860CCE  mulps xmm1, xmm6 */
    xmm2 = _mm_shuffle_ps(xmm2, xmm1, 0x10); /* 0x00860CD1  shufps xmm2, xmm1, 0x10 */
    xmm4 = _mm_mul_ps(xmm4, xmm6); /* 0x00860CD5  mulps xmm4, xmm6 */
    xmm5 = _mm_mul_ps(xmm5, xmm6); /* 0x00860CD8  mulps xmm5, xmm6 */
    xmm4 = _mm_shuffle_ps(xmm4, xmm5, 0x32); /* 0x00860CDB  shufps xmm4, xmm5, 0x32 */
    xmm2 = _mm_shuffle_ps(xmm2, xmm4, 0x88); /* 0x00860CDF  shufps xmm2, xmm4, 0x88 */
    xmm1 = xmm2; /* 0x00860CE3*/
    _mm_store_ps(reinterpret_cast<float*>(out), xmm1); /* 0x00860CE6*/
    return 1;
  L_860CEF:
    st[0].m128_f32[0] = xmm5.m128_f32[0]; /* 0x00860CEF*/
    xmm6.m128_f32[0] = st[0].m128_f32[0]; /* 0x00860CF4*/
    st[13].m128_f32[0] = xmm4.m128_f32[0]; /* 0x00860CF9*/
    if (!_mm_comigt_ss(xmm6, _mm_set1_ps(st[13].m128_f32[0]))) goto L_860D74; /* 0x00860D02*/
    /* 0x00860D0C*/
    xmm4 = _mm_add_ps(xmm4, xmm7); /* 0x00860D0F  addps xmm4, xmm7 */
    xmm5 = _mm_sub_ps(xmm5, xmm4); /* 0x00860D12  subps xmm5, xmm4 */
    xmm4 = st[9]; /* 0x00860D15*/
    xmm4 = _mm_add_ps(xmm4, xmm5); /* 0x00860D1D  addps xmm4, xmm5 */
    xmm6 = _mm_sqrt_ps(xmm4); /* 0x00860D20  sqrtps xmm6, xmm4 */
    xmm7 = xmm1; /* 0x00860D23*/
    xmm7 = _mm_shuffle_ps(xmm7, xmm1, 0xaa); /* 0x00860D26  shufps xmm7, xmm1, 0xaa */
    xmm4 = xmm3; /* 0x00860D2A*/
    xmm4 = _mm_shuffle_ps(xmm4, xmm3, 0); /* 0x00860D2D  shufps xmm4, xmm3, 0 */
    xmm4 = _mm_sub_ps(xmm4, xmm7); /* 0x00860D31  subps xmm4, xmm7 */
    xmm7 = xmm0; /* 0x00860D34*/
    xmm1 = _mm_shuffle_ps(xmm1, xmm1, 0x55); /* 0x00860D37  shufps xmm1, xmm1, 0x55 */
    xmm3 = _mm_shuffle_ps(xmm3, xmm3, 0x55); /* 0x00860D3B  shufps xmm3, xmm3, 0x55 */
    xmm7 = _mm_shuffle_ps(xmm7, xmm0, 0xaa); /* 0x00860D3F  shufps xmm7, xmm0, 0xaa */
    xmm5 = xmm2; /* 0x00860D43*/
    xmm5 = _mm_div_ps(xmm5, xmm6); /* 0x00860D46  divps xmm5, xmm6 */
    xmm3 = _mm_add_ps(xmm3, xmm7); /* 0x00860D49  addps xmm3, xmm7 */
    xmm0 = _mm_shuffle_ps(xmm0, xmm0, 0); /* 0x00860D4C  shufps xmm0, xmm0, 0 */
    xmm1 = _mm_add_ps(xmm1, xmm0); /* 0x00860D50  addps xmm1, xmm0 */
    xmm1 = _mm_mul_ps(xmm1, xmm5); /* 0x00860D53  mulps xmm1, xmm5 */
    xmm2 = _mm_mul_ps(xmm2, xmm6); /* 0x00860D56  mulps xmm2, xmm6 */
    xmm3 = _mm_mul_ps(xmm3, xmm5); /* 0x00860D59  mulps xmm3, xmm5 */
    xmm4 = _mm_mul_ps(xmm4, xmm5); /* 0x00860D5C  mulps xmm4, xmm5 */
    xmm1 = _mm_shuffle_ps(xmm1, xmm2, 0x10); /* 0x00860D5F  shufps xmm1, xmm2, 0x10 */
    xmm3 = _mm_shuffle_ps(xmm3, xmm4, 0x32); /* 0x00860D63  shufps xmm3, xmm4, 0x32 */
    xmm1 = _mm_shuffle_ps(xmm1, xmm3, 0x88); /* 0x00860D67  shufps xmm1, xmm3, 0x88 */
    _mm_store_ps(reinterpret_cast<float*>(out), xmm1); /* 0x00860D6B*/
    return 1;
  L_860D74:
    /* 0x00860D74*/
    xmm5 = _mm_add_ps(xmm5, xmm7); /* 0x00860D77  addps xmm5, xmm7 */
    xmm4 = _mm_sub_ps(xmm4, xmm5); /* 0x00860D7A  subps xmm4, xmm5 */
    xmm5 = st[9]; /* 0x00860D7D*/
    xmm5 = _mm_add_ps(xmm5, xmm4); /* 0x00860D85  addps xmm5, xmm4 */
    xmm7 = _mm_sqrt_ps(xmm5); /* 0x00860D88  sqrtps xmm7, xmm5 */
    xmm4 = xmm1; /* 0x00860D8B*/
    xmm4 = _mm_shuffle_ps(xmm4, xmm1, 0x55); /* 0x00860D8E  shufps xmm4, xmm1, 0x55 */
    xmm5 = xmm0; /* 0x00860D92*/
    xmm5 = _mm_shuffle_ps(xmm5, xmm0, 0); /* 0x00860D95  shufps xmm5, xmm0, 0 */
    xmm4 = _mm_sub_ps(xmm4, xmm5); /* 0x00860D99  subps xmm4, xmm5 */
    xmm5 = xmm3; /* 0x00860D9C*/
    xmm5 = _mm_shuffle_ps(xmm5, xmm3, 0x55); /* 0x00860D9F  shufps xmm5, xmm3, 0x55 */
    xmm1 = _mm_shuffle_ps(xmm1, xmm1, 0xaa); /* 0x00860DA3  shufps xmm1, xmm1, 0xaa */
    xmm6 = xmm2; /* 0x00860DA7*/
    xmm6 = _mm_div_ps(xmm6, xmm7); /* 0x00860DAA  divps xmm6, xmm7 */
    xmm0 = _mm_shuffle_ps(xmm0, xmm0, 0xaa); /* 0x00860DAD  shufps xmm0, xmm0, 0xaa */
    xmm3 = _mm_shuffle_ps(xmm3, xmm3, 0); /* 0x00860DB1  shufps xmm3, xmm3, 0 */
    xmm1 = _mm_add_ps(xmm1, xmm3); /* 0x00860DB5  addps xmm1, xmm3 */
    xmm5 = _mm_add_ps(xmm5, xmm0); /* 0x00860DB8  addps xmm5, xmm0 */
    xmm1 = _mm_mul_ps(xmm1, xmm6); /* 0x00860DBB  mulps xmm1, xmm6 */
    xmm5 = _mm_mul_ps(xmm5, xmm6); /* 0x00860DBE  mulps xmm5, xmm6 */
    xmm2 = _mm_mul_ps(xmm2, xmm7); /* 0x00860DC1  mulps xmm2, xmm7 */
    xmm4 = _mm_mul_ps(xmm4, xmm6); /* 0x00860DC4  mulps xmm4, xmm6 */
    xmm1 = _mm_shuffle_ps(xmm1, xmm5, 0x10); /* 0x00860DC7  shufps xmm1, xmm5, 0x10 */
    xmm2 = _mm_shuffle_ps(xmm2, xmm4, 0x32); /* 0x00860DCB  shufps xmm2, xmm4, 0x32 */
    xmm1 = _mm_shuffle_ps(xmm1, xmm2, 0x88); /* 0x00860DCF  shufps xmm1, xmm2, 0x88 */
    _mm_store_ps(reinterpret_cast<float*>(out), xmm1); /* 0x00860DD3*/
    return 1;
  L_860DDC:
    return 0;
    // clang-format on
}
#endif
}  // namespace ts3::routemanager
