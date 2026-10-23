// Unit clu_45ef30_Debug: the row-measuring helpers of the layout object and its owner.
// See the header for the function map and the ABI. Every field access keeps the
// original's byte offset and access width; console-free, portable C++ (no pointer-size
// assumptions - the offsets are the game's x86 layout, as in the rest of ts3core).
#include <cmath>
#include <cstdint>
#include <cstring>

#include "ts3/debug/clu_45ef30_Debug.h"
#include "ts3/layout.h"

namespace ts3::debug {
namespace {

// Raw game-object field accessors (offsets are the x86 layout, widths the original's).
template <class T> T Load(const void* base, std::size_t offset)
{
    T value;
    std::memcpy(&value, static_cast<const uint8_t*>(base) + offset, sizeof value);
    return value;
}
template <class T> void Store(void* base, std::size_t offset, T value)
{
    std::memcpy(static_cast<uint8_t*>(base) + offset, &value, sizeof value);
}
uint8_t* Bytes(void* p) { return static_cast<uint8_t*>(p); }
const uint8_t* Bytes(const void* p) { return static_cast<const uint8_t*>(p); }
// movss stores/loads move the raw bits; keep them bit-exact instead of a float cast.
uint32_t FloatBits(float f)
{
    uint32_t bits;
    std::memcpy(&bits, &f, sizeof bits);
    return bits;
}
float BitsToFloat(uint32_t bits)
{
    float f;
    std::memcpy(&f, &bits, sizeof f);
    return f;
}
// The originals copy/pass several floats through an x87 fld/fstp round trip, which leaves
// the bits unchanged except that a signaling NaN comes back quiet (bit 22 set). x87 keeps
// the payload and preserves finite values, infinities, zeros and quiet NaNs exactly.
uint32_t QuietNaNBits(uint32_t bits)
{
    const uint32_t mantissa = bits & 0x007FFFFFu;
    if ((bits & 0x7F800000u) == 0x7F800000u && mantissa != 0 && (mantissa & 0x00400000u) == 0)
        return bits | 0x00400000u;
    return bits;
}
float QuietNaN(float f) { return BitsToFloat(QuietNaNBits(FloatBits(f))); }

}  // namespace

void* LayoutBlock::CopyFrom(const void* src)
{
    // 0x0045EF30: eax = ecx (this); ecx = 0x9E; `rep movsd` copies 158 dwords.
    // A forward dword copy, deliberately not memmove: overlapping ranges with dst > src
    // read dwords this function already overwrote, exactly like the original.
    const uint8_t* s = static_cast<const uint8_t*>(src);
    for (std::size_t offset = 0; offset < kSize; offset += 4) {
        uint32_t dword;
        std::memcpy(&dword, s + offset, sizeof dword);       // 0x0045EF3F
        std::memcpy(bytes + offset, &dword, sizeof dword);   // 0x0045EF3F
    }
    return this;  // 0045EF36: eax = this throughout
}

void* SetLayoutSource(void* self, const void* src)
{
    // 0x00462CE0: null source leaves the object alone and returns 0 (single exit so the
    // planted `return {}` site is observable: the source path returns a live pointer).
    uint8_t* result = nullptr;  // 00462D10: EAX = 0
    if (src) {                  // 00462CE9
        uint8_t* block = Bytes(self) + 0xFC;  // 00462CEC
        static_cast<LayoutBlock*>(static_cast<void*>(block))->CopyFrom(src);  // 00462CF5
        // 00462CFA/00462D00: *(dword this+0x384) = this+0xFC, no null check.
        uint8_t* slot = Load<uint8_t*>(self, 0x384);
        Store<uint8_t*>(slot, 0, block);
        Store<uint8_t*>(self, 0x3B8, Load<uint8_t*>(slot, 0));  // 00462D0A: *[this+0x384]
        result = slot;                         // eax = [this+0x384]
    }
    return result;
}

void* AssignLayoutList(void* self, const void* other)
{
    // 0x00465330: self-assignment is a no-op that still returns this.
    if (self == other)
        return self;  // 0046533A

    // 0046533C: release the current tree through 0x0045A050 (unit clu_45a050_UI).
    ts3::CallGameMethod<void, 0x0045A050>(self, Load<uint8_t*>(self, 0xC));

    // 00465349..00465359: re-initialise the embedded list header at +0x04.
    Store<uint8_t*>(self, 0x4, Bytes(self) + 4);
    Store<uint8_t*>(self, 0x8, Bytes(self) + 4);
    Store<uint32_t>(self, 0xC, 0);
    Store<uint8_t>(self, 0x10, 0);
    Store<uint32_t>(self, 0x14, 0);

    const uint8_t* srcRoot = Load<const uint8_t*>(other, 0xC);  // 00465360
    if (srcRoot) {
        // 0046536B: recursive tree copy (0x00464890, unit clu_46xxx_x), out = this+0x04.
        uint8_t* node =
            ts3::CallGameMethod<uint8_t*, 0x00464890>(self, srcRoot, Bytes(self) + 4);
        Store<uint8_t*>(self, 0xC, node);  // 00465372
        // 00465375..00465388: follow the +0x00 chain to its end.
        uint8_t* last = node;
        for (uint8_t* next = Load<uint8_t*>(node, 0); next;
             next = Load<uint8_t*>(next, 0))
            last = next;
        Store<uint8_t*>(self, 0x4, last);
        // 0046538C..0046539C: follow the +0x04 chain to its end.
        uint8_t* lastSecond = node;
        for (uint8_t* next = Load<uint8_t*>(node, 4); next;
             next = Load<uint8_t*>(next, 4))
            lastSecond = next;
        Store<uint8_t*>(self, 0x8, lastSecond);
        Store<uint32_t>(self, 0x14, Load<uint32_t>(other, 0x14));  // 004653A2
    }
    return self;  // 004653A6: eax = esi
}

void* CopyLayoutObject(void* self, const void* src)
{
    // 0x004662F0: field order follows the assembly (0,4,8,0x10,0xC).
    Store<uint32_t>(self, 0x0, Load<uint32_t>(src, 0x0));    // 004662F8
    Store<uint32_t>(self, 0x4, Load<uint32_t>(src, 0x4));    // 004662FC
    Store<uint32_t>(self, 0x8, Load<uint32_t>(src, 0x8));    // 00466302
    Store<uint32_t>(self, 0x10, Load<uint32_t>(src, 0x10));  // 00466308
    Store<uint32_t>(self, 0xC, Load<uint32_t>(src, 0xC));    // 0046630E
    AssignLayoutList(Bytes(self) + 0x14, Bytes(src) + 0x14);  // 0046631B
    Store<uint8_t>(self, 0xF4, Load<uint8_t>(src, 0xF4));     // 00466334
    static_cast<LayoutBlock*>(static_cast<void*>(Bytes(self) + 0xF8))
        ->CopyFrom(Bytes(src) + 0xF8);                        // 0046633A
    Store<uint8_t>(self, 0x370, Load<uint8_t>(src, 0x370));   // 00466346
    Store<uint8_t>(self, 0x371, Load<uint8_t>(src, 0x371));   // 00466353
    Store<uint32_t>(self, 0x374, QuietNaNBits(Load<uint32_t>(src, 0x374)));  // 0x0046635F
    Store<uint8_t>(self, 0x378, Load<uint8_t>(src, 0x378));   // 0046636B
    Store<uint8_t>(self, 0x379, Load<uint8_t>(src, 0x379));   // 00466378
    Store<uint8_t>(self, 0x37A, Load<uint8_t>(src, 0x37A));   // 00466385
    return self;  // 0046638C
}

int ResetLayoutForRows(void* self, int a2, int a3, float a4, float a5, const void* a6)
{
    ts3::CallGameMethod<void, 0x00465DE0>(self, 1);  // 00466B25 (unit clu_44da00_UI)
    Store<uint32_t>(self, 0x3BC, 0x47C35000u);  // 00466B2A: 100000.0f, raw dword
    Store<uint32_t>(self, 0x3C0, FloatBits(a4));  // 0x00466B38
    Store<uint32_t>(self, 0x3C4, FloatBits(a5));  // 0x00466B46
    if (a6) {
        uint8_t* block = Bytes(self) + 0xFC;  // 00466B5F
        static_cast<LayoutBlock*>(static_cast<void*>(block))->CopyFrom(a6);  // 00466B68
        uint8_t* slot = Load<uint8_t*>(self, 0x384);   // 00466B6D, no null check
        Store<uint8_t*>(slot, 0, block);
        Store<uint8_t*>(self, 0x3B8, Load<uint8_t*>(slot, 0));  // 00466B7B: *[this+0x384]
    }
    Store<uint32_t>(self, 0x340, 0);                        // 00466B90
    ts3::CallGameMethod<void, 0x00464BA0>(self, a2, a3);    // 00466B9A
    ts3::CallGameMethod<void, 0x00466990>(self);            // 00466BA1 (unit clu_44da00_UI)
    // 00466BB4..00466BCA: 0x004626C0(this+0x418, 0.0f, 0.0f, 0, rowCount) with the two
    // zero floats pushed as raw dwords from the x87 stack (fldz/fst/fstp).
    const int32_t rows =
        (Load<int32_t>(self, 0x480) - Load<int32_t>(self, 0x47C)) >> 5;  // 00466BBA
    ts3::CallGameMethod<void, 0x004626C0>(Bytes(self) + 0x418, 0.0f, 0.0f, 0, rows);
    return (Load<int32_t>(self, 0x450) - Load<int32_t>(self, 0x44C)) >> 1;  // 00466BDC
}

void* MeasureRowsTweaked(void* self, const int32_t* cell, int32_t rows, int32_t* total,
                         int32_t* last)
{
    Store<int32_t>(self, 0x56C, Load<int32_t>(self, 0x568));  // 00541371: vector clear
    uint8_t* obj = ts3::CallGameMethod<uint8_t*, 0x005137D0>(self);  // 00541377
    ts3::CallGameMethod<void, 0x00465DE0>(obj, 1);                    // 00541382

    // 00541387..005413FB: the original source block, or a stack copy with the two
    // override fields at +0x244/+0x248 patched in.
    uint8_t block[LayoutBlock::kSize];
    uint8_t* src;
    if (Load<int32_t>(self, 0x284) == 4 && Load<int32_t>(self, 0x288) == 3) {
        src = Bytes(self) + 0x2E8;  // 00541399: no copy, no patching
    } else {
        static_cast<LayoutBlock*>(static_cast<void*>(block))
            ->CopyFrom(Bytes(self) + 0x2E8);  // 005413AC
        if (Load<int32_t>(self, 0x284) != 4)  // 005413B1
            Store<int32_t>(block, 0x244, Load<int32_t>(self, 0x284));  // 005413BC
        const int32_t span = static_cast<int32_t>(static_cast<uint32_t>(Load<int32_t>(self, 0x2DC)) -
                                                  static_cast<uint32_t>(Load<int32_t>(self, 0x2D8)));
        if (span / 24 == 1 && Load<int32_t>(self, 0x288) != 3)  // 005413C3..005413EE
            Store<int32_t>(block, 0x248, Load<int32_t>(self, 0x288));  // 005413F0
        src = block;  // 005413F7
    }
    ts3::CallGameMethod<void, 0x00462CE0>(obj, src);  // 005413FE

    // 00541403: 0x00464BA0(obj, this[0x7C] + 2*cell[0], cell[1]).
    const uint32_t x = static_cast<uint32_t>(Load<int32_t>(self, 0x7C)) +
                       2u * static_cast<uint32_t>(cell[0]);
    ts3::CallGameMethod<void, 0x00464BA0>(obj, x, static_cast<uint32_t>(cell[1]));

    // 0054141E..0054145B: 0x004621F0 with (dr - (this[0x658]-this[0x650]), this[0x28C], 0).
    // The third argument and the 0x28C argument come from an x87 fld/fstp (0.0f and the
    // field); the first is an SSE difference, so only those two quiet a signaling NaN.
    const float drB0 =
        (Load<float>(self, 0x294) - Load<float>(self, 0x28C)) -
        (Load<float>(self, 0x658) - Load<float>(self, 0x650));      // 00541430..00541447
    ts3::CallGameMethod<void, 0x004621F0>(obj, drB0, QuietNaN(Load<float>(self, 0x28C)), 0.0f);

    *total = 0;  // 0054146A
    uint8_t* item = ts3::CallGameMethod<uint8_t*, 0x00462240>(obj);  // 00541477
    if (ts3::CallGameMethod<int32_t, 0x00462230>(obj) != 2) {        // 0054147E
        for (;;) {
            ts3::CallGameMethod<void, 0x00466990>(obj);  // 00541492
            const int32_t count =
                (Load<int32_t>(item, 0x8) - Load<int32_t>(item, 0x4)) >> 1;  // 0054149A
            *last = count;  // 005414A8
            if (rows <= 0)  // 005414AA: signed, stops after writing *last
                break;
            *total += count;  // 005414AC
            ts3::CallGameMethod<void, 0x00463920>(obj, 0.0f);  // 005414B2
            --rows;                                            // 005414B9
            if (ts3::CallGameMethod<int32_t, 0x00462230>(obj) == 2)  // 005414C1
                break;
        }
    }
    return ts3::CallGameMethod<uint8_t*, 0x00462240>(obj);  // 005414C9
}

int32_t MeasureRows(void* self, void* cellArg)
{
    uint8_t* cell = Bytes(cellArg);
    uint8_t* vec = Bytes(self) + 0x568;                       // 0x00542CE6
    Store<int32_t>(self, 0x56C, Load<int32_t>(self, 0x568));  // 00542CEC vector clear
    Store<uint32_t>(cell, 0xC, 0);                            // 0x00542CEF
    Store<uint32_t>(cell, 0x10, 0);                           // 0x00542CF4

    uint8_t* obj = ts3::CallGameMethod<uint8_t*, 0x005137D0>(self);  // 00542CF9
    ts3::CallGameMethod<void, 0x00465DE0>(obj, 1);                    // 00542D04
    ts3::CallGameMethod<void, 0x00462CE0>(obj, Bytes(self) + 0x2E8); // 00542D12

    const uint32_t x = static_cast<uint32_t>(Load<int32_t>(self, 0x7C)) +
                       2u * static_cast<uint32_t>(Load<int32_t>(cell, 0x0));  // 00542D1A
    ts3::CallGameMethod<void, 0x00464BA0>(obj, x, static_cast<uint32_t>(Load<int32_t>(cell, 0x4)));

    // 00542D2B..00542D64: 0x004621F0 with (dr - (this[0x658]-this[0x650]), this[0x28C], 0);
    // the 0x28C argument and the 0.0f constant pass through x87 (see 0x00541360).
    const float delta =
        (Load<float>(self, 0x294) - Load<float>(self, 0x28C)) -
        (Load<float>(self, 0x658) - Load<float>(self, 0x650));      // 00542D3D..00542D54
    ts3::CallGameMethod<void, 0x004621F0>(obj, delta, QuietNaN(Load<float>(self, 0x28C)), 0.0f);

    int32_t runningTotal = 0;  // 00542D70 (local at [esp+0x10])
    if (ts3::CallGameMethod<int32_t, 0x00462230>(obj) != 2) {  // 00542D78
        do {
            ts3::CallGameMethod<void, 0x00466990>(obj);  // 00542D88
            uint8_t* item = ts3::CallGameMethod<uint8_t*, 0x00462240>(obj);  // 00542D8F
            const int32_t count =
                (Load<int32_t>(item, 0x8) - Load<int32_t>(item, 0x4)) >> 1;  // 00542D9B
            // 00542DA6: cell[4] += (float)this[0x560] before the bounds compare.
            Store<float>(cell, 0x10, Load<float>(cell, 0x10) +
                                        static_cast<float>(Load<int32_t>(self, 0x560)));
            // 00542DB0..00542DCB: comiss cell[3], item[0xB8]; `ja` keeps cell[3] on
            // unordered too, in which case the original still reloads and stores it.
            const float itemB8 = Load<float>(item, 0xB8);
            float merged = Load<float>(cell, 0xC);
            if (itemB8 > merged)
                merged = itemB8;
            Store<float>(cell, 0xC, QuietNaN(merged));  // 0x00542DC9

            // 00542DCD..00542DF3: push the running total into the vector, growing
            // through 0x008621C0 (cdecl-ish vector helper, unit clu_8621c0_x) when full.
            const uint32_t end = Load<uint32_t>(vec, 0x4);
            const uint32_t cap = Load<uint32_t>(vec, 0x8);
            int32_t value = runningTotal;
            if (end >= cap) {  // 0x00542DDB
                ts3::CallGameMethod<void, 0x008621C0>(vec, end, &value);
            } else {
                Store<uint32_t>(vec, 0x4, end + 4);  // 00542DE2
                if (end)                             // 00542DE5
                    Store<int32_t>(reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(end)), 0,
                                   runningTotal);
            }
            runningTotal += count;  // 00542DF8

            ts3::CallGameMethod<void, 0x00463920>(obj, 0.0f);  // 00542E00
        } while (ts3::CallGameMethod<int32_t, 0x00462230>(obj) != 2);  // 00542E0C
    }
    Store<int32_t>(self, 0x578, runningTotal);  // 00542E1D
    const int32_t result = static_cast<int32_t>(
        (Load<uint32_t>(vec, 0x4) - Load<uint32_t>(vec, 0x0)) >> 2);  // 00542E26
    Store<int32_t>(cell, 0x8, result);  // 00542E2F
    return result;                      // eax
}

float* RowBounds(float* out, int32_t a2, int32_t a3, void* a4, void* obj)
{
    if (!a2 || !a4) {  // 007E5BA4 / 007E5BB1: either null zeroes both floats
        out[0] = 0.0f;
        out[1] = 0.0f;
        return out;  // 007E5C3E: eax = out
    }
    ResetLayoutForRows(obj, a2, a3, 0.0f, 0.0f, a4);  // 007E5BD3
    uint8_t* item = ts3::CallGameMethod<uint8_t*, 0x00462240>(obj);  // 007E5BDA
    float height = Load<float>(a4, 0x274);  // 007E5BDF
    if (height == 0.0f) {                   // 0x007E5BE7
        // 007E5BF6..007E5C26: (float)(int)((float)ceil(item[0xBC]) - floor(item[0xC0])),
        // the ceil result rounded through a 32-bit float scratch slot first. The exe's
        // __ftol2 fallback truncates through a 64-bit x87 integer (fistp qword) and keeps
        // the low 32 bits, so an out-of-int32-range difference wraps instead of
        // saturating (4.5e9 -> 205032704, a plain (int) cast would give INT_MIN).
        const float ceilValue =
            static_cast<float>(std::ceil(static_cast<double>(Load<float>(item, 0xBC))));
        const double difference = static_cast<double>(ceilValue) -
                                  std::floor(static_cast<double>(Load<float>(item, 0xC0)));
        const auto wide = static_cast<int64_t>(difference);
        height = static_cast<float>(static_cast<int32_t>(static_cast<uint32_t>(
            static_cast<uint64_t>(wide))));
    }
    out[0] = QuietNaN(Load<float>(item, 0xB4));  // 0x007E5C2A
    out[1] = height;                   // 0x007E5C37
    return out;                        // 007E5C30: eax = out
}

}  // namespace ts3::debug