// Unit cls_723c70_UI (UI): the "UI/TextGlyphCache" interface object. Semantics from the
// TS3W.exe 1.67.2 listings (the original addresses are noted on each function).
#include "ts3/ui/cls_723c70_UI.h"

namespace ts3::ui {
namespace {

// Addresses inside the game image the original pushes verbatim: the class-name strings,
// the 5-element table, and the vtable pointer the constructor stores. x86 only - in x64
// builds there is no game image (GamePtr yields nothing and the bridge aborts).
constexpr uintptr_t kStrVertexFormat = 0x00FFCF1C;  // "UI/ImmediateModeEx_TSS/VertexFormat"
constexpr uintptr_t kStrUishader = 0x00FFCF68;      // "uishader"
constexpr uintptr_t kStrWindata = 0x00FFCF60;       // "windata"
constexpr uintptr_t kStrTexMap = 0x00FFCF58;        // "texMap"
constexpr uintptr_t kStrTexDepthMap = 0x00FFCF4C;   // "texDepthMap"
constexpr uintptr_t kStrTexMaskMap = 0x00FFCF40;    // "texMaskMap"
constexpr uintptr_t kElementTable5 = 0x00FFCEF4;    // 5 dwords handed to 0x0061A010
constexpr uintptr_t kGameVtable = 0x00FFCFE8;

#if defined(_M_IX86)
template <class T>
T GamePtr(uintptr_t address)
{
    return reinterpret_cast<T>(address);
}
// The pointer stored in a game-image global slot (0x011D753C / 0x011D7530 / 0x011CECC8).
void* GameGlobal(uintptr_t slot)
{
    return *reinterpret_cast<void**>(slot);
}
#else
template <class T>
T GamePtr(uintptr_t)
{
    return T{};
}
void* GameGlobal(uintptr_t)
{
    return nullptr;
}
#endif

}  // namespace

bool UiFormatHolder::Create723C70()
{
    // TS3W.exe 0x00723C70 (thiscall, this = the holder at object+0xC30): eight cdecl
    // stack args to 0x004E3FB0 (size 0x24, align 4, flags 0, the class-name string, then
    // four zero dwords); a non-null result is wrapped by 0x0061A300 (thiscall, the raw
    // object as `this`); the wrap's vtable slot 1 (+4) runs with no arguments; finally
    // 0x0061A010 (thiscall, two stack args) installs the 5-element table at 0x00FFCEF4.
    // EA order: the pointer is STORED before its null check, and after the virtual call
    // the original reloads `this` from memory rather than reusing the local - kept.
    void* raw = CallGameFunction<void*, 0x004E3FB0>(
        static_cast<uint32_t>(0x24), static_cast<uint32_t>(4), static_cast<uint32_t>(0),
        GamePtr<const char*>(kStrVertexFormat), static_cast<uint32_t>(0),
        static_cast<uint32_t>(0), static_cast<uint32_t>(0), static_cast<uint32_t>(0));
    void* wrapped = raw ? CallGameMethod<void*, 0x0061A300>(raw) : nullptr;
    format = wrapped;
    if (!format)
        return false;
    CallGameVirtual<void, 1>(format);
    // the original instruction sequence: the original normalizes AL (upper EAX stays stale).
    return CallGameMethod<uint8_t, 0x0061A010>(format, GamePtr<const void*>(kElementTable5),
                                                static_cast<uint32_t>(5)) != 0;
}

UiTextGlyphCache* UiTextGlyphCache::Construct724840()
{
    // TS3W.exe 0x00724840 (plain ret, returns this): stores the game vtable 0x00FFCFE8
    // at +0, byte 0 at +4, zeroes the leading dword of each of the 64 slot records
    // (+0x128 + i*0x18, i < 64 via the original instruction sequence), then dwords 0 at +0xC3C/+0xC50.
    // Fields +8, +0x14, +0x18, +0xC30, +0xC40..+0xC4C and the slot payloads are never
    // touched (kept: the caller's bytes survive construction).
    vtable = GamePtr<void*>(kGameVtable);
    flag04 = 0;
    for (UiGlyphSlot& slot : slots)
        slot.head = 0;
    uishader = nullptr;
    refcount = 0;
    return this;
}

uint8_t UiTextGlyphCache::Acquire723CD0()
{
    // TS3W.exe 0x00723CD0: when the refcount at +0xC50 is zero, look the shader up via
    // the registry singleton at 0x011D753C (0x0079B840, thiscall, three stack args),
    // build the vertex format through UiFormatHolder::Create723C70, then fetch the four
    // shader param handles through the singleton at 0x011D7530 (0x0079A160, thiscall,
    // three stack args). EA order kept: windata -> +0xC40, texMap -> +0xC44,
    // texDepthMap -> +0xC4C, texMaskMap -> +0xC48 (the last two stores are swapped).
    // Returns AL only (1/0); the original leaves the upper EAX bits stale.
    if (refcount == 0) {
        void* registry = GameGlobal(0x011D753C);
        uishader = CallGameMethod<void*, 0x0079B840>(
            registry, GamePtr<const char*>(kStrUishader), static_cast<uint32_t>(0),
            static_cast<uint32_t>(0));
        if (!uishader)
            return 0;
        if (!formatHolder.Create723C70())
            return 0;
        void* params = GameGlobal(0x011D7530);
        windata = CallGameMethod<void*, 0x0079A160>(params, GamePtr<const char*>(kStrWindata),
                                                    static_cast<uint32_t>(80),
                                                    static_cast<uint32_t>(2));
        texMap = CallGameMethod<void*, 0x0079A160>(params, GamePtr<const char*>(kStrTexMap),
                                                   static_cast<uint32_t>(1),
                                                   static_cast<uint32_t>(5));
        texDepthMap = CallGameMethod<void*, 0x0079A160>(params,
                                                        GamePtr<const char*>(kStrTexDepthMap),
                                                        static_cast<uint32_t>(1),
                                                        static_cast<uint32_t>(5));
        texMaskMap = CallGameMethod<void*, 0x0079A160>(params,
                                                       GamePtr<const char*>(kStrTexMaskMap),
                                                       static_cast<uint32_t>(1),
                                                       static_cast<uint32_t>(5));
    }
    ++refcount;
    return 1;
}

int32_t UiTextGlyphCache::Release723D80()
{
    // TS3W.exe 0x00723D80: the original instruction sequence - the decrement wraps below
    // zero (a release at count 0 leaves -1 and skips cleanup, EA quirk kept). On the drop
    // to 0: virtual slot 2 (+8) on the format object if non-null, then +0xC30 and +0xC3C
    // are zeroed. The returned int is the virtual's result when it ran; on every other
    // path the original returns the caller's stale EAX (nothing can observe it - modelled
    // as 0).
    --refcount;
    if (refcount != 0)
        return 0;
    int32_t result = 0;
    if (formatHolder.format) {
        result = CallGameVirtual<int32_t, 2>(formatHolder.format);
        formatHolder.format = nullptr;
    }
    uishader = nullptr;
    return result;
}

void* UiTextGlyphCache::QueryIface723DC0(uint32_t id)
{
    // TS3W.exe 0x00723DC0 (ret 4): this for the interface-id dwords 0xEE3F516E and
    // 0x00838293; for 0x6ED3E59B the original also returns this, through the pointless
    // the original instruction sequence mask (kept verbatim below); anything else
    // masks EAX to 0.
    void* self = this;
    if (id == 0xEE3F516E || id == 0x00838293)
        return self;
    // the original instruction sequence: 0xFFFFFFFF for 0x6ED3E59B (keeps
    // this), 0 otherwise.
    const uint32_t mask = (id == 0x6ED3E59B) ? 0xFFFFFFFFu : 0u;
    return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(self) & mask);
}

int32_t UiTextGlyphCache::Reset724810()
{
    // TS3W.exe 0x00724810: when +8 is non-zero, flush through 0x007243B0 (thiscall,
    // `this` still in ecx); zero +0x14/+0x18; call 0x0060FCE0 (thiscall, no args) on the
    // singleton at 0x011CECC8; store byte 0 at +4; return the singleton call's eax.
    if (field08)
        CallGameMethod<void, 0x007243B0>(this);
    field14 = 0;
    field18 = 0;
    const int32_t result = CallGameMethod<int32_t, 0x0060FCE0>(GameGlobal(0x011CECC8));
    flag04 = 0;
    return result;
}

}  // namespace ts3::ui
