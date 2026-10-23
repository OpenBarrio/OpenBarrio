// Unit clu_5a00a0_Routing: TS3W.exe 0x005A00A0 / 0x006E7B60 / 0x00731ED0 / 0x007377C0 /
// 0x00AE0380 / 0x00AE0400 (see the header for the map). Every function but the getter
// does the same two things: fetch the object at the global 0x11CE3F8, and when it is
// non-null make one or more virtual calls on it. The originals reach the object and the
// float/string constants by touching the mapped image directly; here those go through
// GameGlobal*, which are no-ops on x64 - so there the registrations fail closed (the
// getter returns null and every `if (obj)` is false), exactly like a game that has not
// built the object yet.
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <cstdint>
#include <cstring>

#include "ts3/layout.h"
#include "ts3/routing/clu_5a00a0_Routing.h"

namespace ts3::routing {
namespace {

// ---- game globals and image constants ---------------------------------------

constexpr uint32_t kTuningObject = 0x11CE3F8;  // the object every call below goes through
constexpr uint32_t kLodBindings = 0x11D1814;   // five 12-byte cells, owner dword first
constexpr uint32_t kHideFactor = 0x11D1850;    // float cell, owned by initialiser 0x00F828E0
constexpr uint32_t kLotFlag = 0x11EB610;       // byte: the Lot Renderer tweak is installed
constexpr uint32_t kLotTarget = 0x11EB650;     // 16-byte cell bound to the tweak

void* GameGlobalPointer(uint32_t address)
{
#if defined(_M_IX86)
    return *reinterpret_cast<void* const*>(static_cast<uintptr_t>(address));
#else
    (void)address;
    return nullptr;
#endif
}

void GameGlobalSet32(uint32_t address, uint32_t value)
{
#if defined(_M_IX86)
    *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(address)) = value;
#else
    (void)address;
    (void)value;
#endif
}

uint8_t GameGlobal8(uint32_t address)
{
#if defined(_M_IX86)
    return *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(address));
#else
    (void)address;
    return 0;
#endif
}

void GameGlobalSet8(uint32_t address, uint8_t value)
{
#if defined(_M_IX86)
    *reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(address)) = value;
#else
    (void)address;
    (void)value;
#endif
}

// The originals load these with the original instruction sequence, so reading the same cell keeps
// the pushed bits identical without transcribing the constants by hand.
float GameGlobalF32(uint32_t address)
{
#if defined(_M_IX86)
    float f;
    std::memcpy(&f, reinterpret_cast<const void*>(static_cast<uintptr_t>(address)), sizeof f);
    return f;
#else
    (void)address;
    return 0.0f;
#endif
}

// An address the original pushes as an immediate (the wide names live in the image).
template <unsigned kAddress>
const void* GamePtr()
{
    return reinterpret_cast<const void*>(static_cast<uintptr_t>(kAddress));
}

// ---- the object's vtable slots this unit uses --------------------------------
//
//  6  (vtable +0x18): (const void* name, int32_t value)
//  7  (vtable +0x1C): (const void* target, const void* name, float min, float max, float step)
// 12  (vtable +0x30): same shape as 7
// 17  (vtable +0x44): (int32_t kind, const void* bind, const void* name, const void* get,
//                     const void* set, float min, float max, float step)
// 18  (vtable +0x48): (const void* name)
// 19  (vtable +0x4C): (const void* target)
//
// All are __thiscall with the callee popping the stack arguments; CallGameVirtual does
// that. Nothing here allocates or writes, so every call is a plain statement.

// The five {cell, index} bindings 0x006E7B60 writes: cell = owner dword, index at +4.
struct Binding {
    uint32_t cell;
    int32_t index;
};
constexpr Binding kLodBindingCells[] = {
    {0x11D1814, 0}, {0x11D1820, 1}, {0x11D182C, 2}, {0x11D1838, 3}, {0x11D1844, 4},
};

}  // namespace

// ---------------------------------------------------------------------------
// 0x005A00A0 GetTuningObject
// ---------------------------------------------------------------------------

void* GetTuningObject()
{
    return GameGlobalPointer(kTuningObject);  // 0x005A00A0
}

// ---------------------------------------------------------------------------
// 0x006E7B60 RegisterLodSettings
// ---------------------------------------------------------------------------

int32_t RegisterLodSettings(LodSettingsHost* self)
{
    // 006E7B63..006E7BA9: owner then index, one pair per 12-byte cell, 0 through 4.
    const uint32_t owner = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(self));
    for (const Binding& b : kLodBindingCells) {
        GameGlobalSet32(b.cell, owner);
        GameGlobalSet32(b.cell + 4, static_cast<uint32_t>(b.index));
    }

    // 006E7BB3 slot 6: the streaming budget, 10000.
    if (void* o = GetTuningObject())
        CallGameVirtual<int32_t, 6>(o, GamePtr<0x00FF829C>(), 10000);

    // 006E7BCF..006E7D0D slot 17: the five Model LOD radii. The bind cells are NOT in
    // index order - Fences (0x11D1838, index 3) is registered before Portals (0x11D182C,
    // index 2) - and the maxima come from three different image cells (2500, 2500, 2500,
    // 1000, 500). min is fldz (0.0), step is fld1 (1.0).
    if (void* o = GetTuningObject())
        CallGameVirtual<int32_t, 17>(o, 2, GamePtr<0x11D1814>(), GamePtr<0x00FF8270>(),
                                     GamePtr<0x006E7560>(), GamePtr<0x006E7820>(), 0.0f,
                                     GameGlobalF32(0x00FF80A4), 1.0f);
    if (void* o = GetTuningObject())
        CallGameVirtual<int32_t, 17>(o, 2, GamePtr<0x11D1820>(), GamePtr<0x00FF8228>(),
                                     GamePtr<0x006E7560>(), GamePtr<0x006E7820>(), 0.0f,
                                     GameGlobalF32(0x00FF80A4), 1.0f);
    if (void* o = GetTuningObject())
        CallGameVirtual<int32_t, 17>(o, 2, GamePtr<0x11D1838>(), GamePtr<0x00FF81EC>(),
                                     GamePtr<0x006E7560>(), GamePtr<0x006E7820>(), 0.0f,
                                     GameGlobalF32(0x00FF80A4), 1.0f);
    if (void* o = GetTuningObject())
        CallGameVirtual<int32_t, 17>(o, 2, GamePtr<0x11D182C>(), GamePtr<0x00FF81B0>(),
                                     GamePtr<0x006E7560>(), GamePtr<0x006E7820>(), 0.0f,
                                     GameGlobalF32(0x00F98970), 1.0f);
    if (void* o = GetTuningObject())
        CallGameVirtual<int32_t, 17>(o, 2, GamePtr<0x11D1844>(), GamePtr<0x00FF8178>(),
                                     GamePtr<0x006E7560>(), GamePtr<0x006E7820>(), 0.0f,
                                     GameGlobalF32(0x00FBCFE4), 1.0f);

    // 006E7D0F slot 12: Model LOD Adjust, bound to this+0x2C (the original adds 0x2C to
    // its saved this just for this push). -4.0 / 4.0 / 0.1.
    if (void* o = GetTuningObject())
        CallGameVirtual<int32_t, 12>(o, static_cast<uint8_t*>(static_cast<void*>(self)) + 0x2C,
                                     GamePtr<0x00FF8154>(), GameGlobalF32(0x00FED4D8),
                                     GameGlobalF32(0x00FB2F6C), GameGlobalF32(0x00FF8008));

    // 006E7D4A slot 17: Object Size Hide Factor, bound to the float cell 0x11D1850 - a
    // different kind of binding from the five above (the static initialiser 0x00F828E0
    // writes that cell; this function never does), with its own getter/setter pair.
    if (void* o = GetTuningObject())
        CallGameVirtual<int32_t, 17>(o, 2, GamePtr<0x11D1850>(), GamePtr<0x00FF8124>(),
                                     GamePtr<0x006E7580>(), GamePtr<0x006E7B50>(), 0.0f,
                                     GameGlobalF32(0x00F98970), 1.0f);

    // 006E7D8B slot 6: (null, INT_MAX). Its result is the function's return value - the
    // original returns 0 when the object is null (the `call 5A00A0` left eax = 0).
    void* last = GetTuningObject();
    return last ? CallGameVirtual<int32_t, 6>(last, static_cast<const void*>(nullptr), 0x7FFFFFFF) : 0;
}

// ---------------------------------------------------------------------------
// 0x00731ED0 RegisterCacheMenu
// ---------------------------------------------------------------------------

int32_t RegisterCacheMenu()
{
    // 00731ED0..00731F13: three slot-18 calls; only the last result is returned.
    if (void* o = GetTuningObject())
        CallGameVirtual<int32_t, 18>(o, GamePtr<0x00FFDD78>());  // L"Shrink cache"
    if (void* o = GetTuningObject())
        CallGameVirtual<int32_t, 18>(o, GamePtr<0x00FFDD50>());  // L"Cache overflow size"
    void* last = GetTuningObject();
    return last ? CallGameVirtual<int32_t, 18>(last, GamePtr<0x00FFDD38>()) : 0;  // L"Cache size"
}

// ---------------------------------------------------------------------------
// 0x007377C0 RoutingHandoff
// ---------------------------------------------------------------------------

int32_t RoutingHandoff(RoutingHandoffTarget* self)
{
    // 007377C3..007377D7: slot 19 with this+0x14 when the object exists.
    if (void* o = GetTuningObject())
        CallGameVirtual<int32_t, 19>(o, static_cast<uint8_t*>(static_cast<void*>(self)) + 0x14);

    // 007377D9: the original instruction sequence - a tail call, so 0x00737060
    // (not part of this unit) gets this+0x18 in ecx and its return goes straight back to
    // 0x007377C0's caller. CallGameMethod makes that thiscall.
    return CallGameMethod<int32_t, 0x00737060>(static_cast<uint8_t*>(static_cast<void*>(self)) + 0x18);
}

// ---------------------------------------------------------------------------
// 0x00AE0380 InitLotRenderer / 0x00AE0400 ReleaseLotRenderer
// ---------------------------------------------------------------------------

uint32_t InitLotRenderer()
{
    uint32_t last = 0;
    if (GameGlobal8(kLotFlag) == 0) {  // 0x00AE0380
        GameGlobalSet8(kLotFlag, 1);

        // 00AE0390 slot 6: L"Lot Renderer", 16000.
        if (void* o = GetTuningObject())
            last = static_cast<uint32_t>(CallGameVirtual<int32_t, 6>(o, GamePtr<0x01068BF4>(), 16000));
        else
            last = 0;  // the `call 5A00A0` itself put 0 in eax

        // 00AE03AC slot 7: the 16-byte cell 0x11EB650 bound to L"WallTop Color (r,g,b)",
        // 0.0 / 1.0 / 0.001 (min fldz, step fld1, max from 0x01010960).
        if (void* o = GetTuningObject())
            last = static_cast<uint32_t>(CallGameVirtual<int32_t, 7>(o, GamePtr<0x11EB650>(),
                                                                    GamePtr<0x01068BC8>(), 0.0f, 1.0f,
                                                                    GameGlobalF32(0x01010960)));
        else
            last = 0;

        // 00AE03E0 slot 6: (null, INT_MAX).
        if (void* o = GetTuningObject())
            last = static_cast<uint32_t>(CallGameVirtual<int32_t, 6>(o, static_cast<const void*>(nullptr),
                                                                    0x7FFFFFFF));
        else
            last = 0;
    }
    // 00AE03F9: the original instruction sequence - only AL is written. The upper 24 bits are the last virtual
    // call's result, and on the early-out they are the caller's stale EAX; returning 0
    // there is the deterministic reading of "undefined". The low byte is always 1.
    return (last & 0xFFFFFF00u) | 1u;
}

int32_t ReleaseLotRenderer()
{
    if (GameGlobal8(kLotFlag) != 0) {  // 0x00AE0400
        GameGlobalSet8(kLotFlag, 0);
        void* o = GetTuningObject();
        // 00AE0410 slot 18: L"WallTop Color (r,g,b)" - the same name 0x00AE0380 registered.
        // On the no-op path the original leaves EAX untouched, so the rewrite returns 0
        // there and the test skips the return comparison on that path.
        return o ? CallGameVirtual<int32_t, 18>(o, GamePtr<0x01068BC8>()) : 0;
    }
    return 0;
}

}  // namespace ts3::routing
