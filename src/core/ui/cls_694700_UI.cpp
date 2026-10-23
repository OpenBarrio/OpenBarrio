// Unit cls_694700_UI: TS3W.exe 0x00694700 (constructor) and 0x00694960 (parameter-record
// builder) - see the header for the full map. Everything the two do is either field writes
// on `this` / the record, or operations on the shared statics cell 0x11CF6B0..0x11CF6DC;
// on x86 those are the live game cells (identical state for original and rewrite), on x64
// a private copy whose flags start pre-set so the 0x007CFE70 hash call is never reached
// (no game code exists there).
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <cstdint>
#include <cstring>

#include "ts3/layout.h"
#include "ts3/ui/cls_694700_UI.h"

namespace ts3::ui {
namespace {

// Reads one of the original's live float cells (they are read-only constant pools of the
// mapped image; the rewrite reads them too so a patched cell stays observable), with a
// portable fallback where there is no image.
float CellFloat(uint32_t address, float fallback)
{
#if defined(_M_IX86)
    return *reinterpret_cast<const float*>(static_cast<uintptr_t>(address));
#else
    (void)address;
    return fallback;
#endif
}

// The shared statics block: the game image's own memory on x86, a private copy on x64.
GhostStatics& Statics()
{
#if defined(_M_IX86)
    return *reinterpret_cast<GhostStatics*>(kGhostStaticsAddress);
#else
    static GhostStatics shared = [] {
        GhostStatics s{};
        s.flags = 0xFFu;  // every id "already hashed", vec4 "already zeroed": fail closed
        return s;
    }();
    return shared;
#endif
}

// 0x007CFE70 (clu_5c87a0_Renderer, not rewritten): string -> id hash, cdecl.
uint32_t HashId(const char* name)
{
    return CallGameFunction<uint32_t, 0x007CFE70>(name);
}

}  // namespace

GhostParams* GhostParams::Construct694700()
{
    // Original: mov eax,ecx; [eax] = 0x00FEFF6C; xchg [eax+4],0 (old value dropped);
    // [eax] = 0x00FF28B0; byte [eax+8] = 1; dword [eax+0xC] = 0; ret. The first vtable
    // store is invisible in the final state (overwritten before any branch), only the
    // final one is kept; the interlocked xchg degenerates to a plain clear here.
#if defined(_M_IX86)
    vtable_ = reinterpret_cast<void*>(static_cast<uintptr_t>(0x00FF28B0));
#else
    vtable_ = nullptr;
#endif
    field04 = 0;
    field08 = 1;
    field0C = 0;
    return this;  // EAX = this on every path
}

GhostParamRecord* GhostParams::Build694960(GhostParamRecord* out, float* params)
{
    GhostStatics& st = Statics();

    // One-shot id init behind the flag byte at 0x11CF6DC: EA tests the flag as a byte
    // and sets it with a plain (non-atomic) dword or - kept as-is.
    if ((st.flags & 0x01u) == 0) { st.flags |= 0x01u; st.idColor1 = HashId("GhostColor1"); }
    if ((st.flags & 0x02u) == 0) { st.flags |= 0x02u; st.idColor2 = HashId("GhostColor2"); }
    if ((st.flags & 0x04u) == 0) { st.flags |= 0x04u; st.idFilterParams = HashId("GhostFilterParams"); }
    if ((st.flags & 0x08u) == 0) { st.flags |= 0x08u; st.idNoiseParams = HashId("GhostNoiseParams"); }
    if ((st.flags & 0x10u) == 0) { st.flags |= 0x10u; st.idNoiseTexture = HashId("GhostNoiseTexture"); }
    if ((st.flags & 0x20u) == 0) { st.flags |= 0x20u; st.idHeightParams = HashId("GhostHeightParams"); }
    if ((st.flags & 0x40u) == 0) { st.flags |= 0x40u; st.idFilterColor = HashId("GhostFilterColor"); }

    // First descriptor block group (original's store order kept for readability; `out`
    // and `params` never alias in the game).
    out->count00 = 1;
    out->idColor1 = st.idColor1;
    out->params0 = params;
    out->n24 = 3;
    out->flag28 = 1;
    out->idColor2 = st.idColor2;
    out->params4 = params + 4;
    out->n4C = 3;
    out->flag50 = 1;
    out->idFilterParams = st.idFilterParams;
    out->params8 = params + 8;
    out->n74 = 4;
    out->flag78 = 1;

    // EA quirk: not a float compare - bit-select on the sign of (params[8] - 0.6f), so
    // +NaN keeps its bits and -NaN (or any negative delta) becomes 0.6f.
    if (clampParams != 0) {
        const float kPointSix = CellFloat(0x00F9D500, 0.6f);
        float a, d;
        std::memcpy(&a, &params[8], sizeof a);
        d = a - kPointSix;
        uint32_t deltaBits, aBits, kBits;
        std::memcpy(&deltaBits, &d, sizeof deltaBits);
        std::memcpy(&aBits, &a, sizeof aBits);
        std::memcpy(&kBits, &kPointSix, sizeof kBits);
        const uint32_t sign = (deltaBits & 0x80000000u) ? 0xFFFFFFFFu : 0u;  // sar 31
        const uint32_t result = (kBits & sign) | (aBits & ~sign);
        std::memcpy(&params[8], &result, sizeof result);
    }

    // Second block group (noise) + the raw copy of params[20..23].
    out->idNoiseParams = st.idNoiseParams;
    out->params12 = params + 12;
    out->n9C = 4;
    out->nA0 = 4;
    out->idNoiseTexture = st.idNoiseTexture;
    std::memcpy(out->noiseTex, params + 20, sizeof out->noiseTex);

    // The bit-0x80 one-shot: zero the vec4 exactly once. EA evaluates the flag before
    // the block writes above (no flag-setting instruction between) - same result here.
    if ((st.flags & 0x80u) == 0) {
        st.flags |= 0x80u;
        st.vec4[0] = 0.0f;
        st.vec4[1] = 0.0f;
        st.vec4[2] = 0.0f;
        st.vec4[3] = 0.0f;
    }

    // vec4[0] = 1.0f iff (mode & 0x7F) == 1, else +0.0f (xorps zero, not -0.0).
    const uint32_t mode = this->mode;
    st.vec4[0] = ((mode & 0x7Fu) == 1u) ? CellFloat(0x0107A538, 1.0f) : 0.0f;

    // vec4[1] = gain: default 1.0f; for (mode & 0xCF00) in {0x300,0x400,0x500,0x700}
    // -> 10.0f when (mode & 0x7F) < 0x20, else 6.0f for 0x500 and 3.0f otherwise.
    // vec4[2]/vec4[3] are never written by this path (they persist).
    float gain = CellFloat(0x0107A538, 1.0f);
    const uint32_t mid = mode & 0xCF00u;
    if (mid == 0x300u || mid == 0x400u || mid == 0x500u || mid == 0x700u) {
        if ((mode & 0x7Fu) >= 0x20u)
            gain = (mid == 0x500u) ? CellFloat(0x00FB2F68, 6.0f)
                                   : CellFloat(0x01074010, 3.0f);
        else
            gain = CellFloat(0x01045CCC, 10.0f);
    }
    st.vec4[1] = gain;

    // Tail block group: the record points at the shared vec4 itself (0x11CF6B0 - the
    // original stores that absolute address, and the rewrite stores the address of the
    // very same memory).
    out->flagC8 = 1;
    out->idHeightParams = st.idHeightParams;
    out->sharedVec4 = st.vec4;
    out->nEC = 4;
    out->flagF0 = 1;
    out->idFilterColor = st.idFilterColor;
    out->params16 = params + 16;
    out->n114 = 3;

    return out;  // EAX = out on every path
}

}  // namespace ts3::ui
