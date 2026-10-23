// Unit cls_4553a0_x: TS3W.exe PolygonFont GlyphMetricsMap. Every function is transcribed
// from the original disassembly (the disassembly is the source of truth).
// The object is raw game memory: all fields are addressed by their x86 offsets, never
// through a C++ layout (which would grow the pointer fields in the x64 build).
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <cstdint>
#include <cstring>

#include "ts3/layout.h"
#include "ts3/misc/cls_4553a0_x.h"

namespace ts3::misc {
namespace {

inline uint8_t* Bytes(void* self) { return static_cast<uint8_t*>(self); }
inline const uint8_t* Bytes(const void* self) { return static_cast<const uint8_t*>(self); }

template <class T>
inline T& Field(void* self, uint32_t offset)
{
    return *reinterpret_cast<T*>(Bytes(self) + offset);
}
template <class T>
inline const T& Field(const void* self, uint32_t offset)
{
    return *reinterpret_cast<const T*>(Bytes(self) + offset);
}

// Game addresses only exist in the 32-bit image; the x64 build compiles them to nothing.
inline void* GamePtr(uint32_t address)
{
#if defined(_M_IX86)
    return reinterpret_cast<void*>(static_cast<uintptr_t>(address));
#else
    (void)address;
    return nullptr;
#endif
}
inline float GameFloat(uint32_t address)
{
#if defined(_M_IX86)
    return *reinterpret_cast<const float*>(static_cast<uintptr_t>(address));
#else
    (void)address;
    return 0.0f;
#endif
}

// 0x0045DC30/0x0045DFA0 release payload arrays through the allocator object's slot 3.
inline void AllocatorFree(void* allocator, void* pointer, uint32_t size)
{
#if defined(_M_IX86)
    using FreeFn = void(__thiscall*)(void*, void*, uint32_t);
    void** vtbl = *reinterpret_cast<void***>(allocator);
    reinterpret_cast<FreeFn>(vtbl[3])(allocator, pointer, size);
#else
    (void)allocator;
    (void)pointer;
    (void)size;
#endif
}

// The original's signed division by 6: imul 0x2AAAAAAB, sar edx,2, sign fixup.
inline int32_t DivideBy24(int32_t value)
{
    const int64_t product = static_cast<int64_t>(value) * static_cast<int64_t>(0x2AAAAAAB);
    int32_t high = static_cast<int32_t>(static_cast<uint64_t>(product) >> 32);
    high >>= 2;
    return high + static_cast<int32_t>(static_cast<uint32_t>(high) >> 31);
}

// Shared tree walk of 0x0045D120/0x0045D260: the map header sits at +0x158 (its parent
// pointer at +0x160 is the root), nodes are {left +0, right +4, key u16 +0x10,
// value u16 +0x12}; the walk keeps the smallest node whose key is >= ch, then only accepts
// it when its key equals ch.
inline uint32_t FindGlyphNode(void* self, uint16_t ch)
{
    const uintptr_t header = reinterpret_cast<uintptr_t>(Bytes(self) + 0x158);
    uintptr_t node = Field<uintptr_t>(self, 0x160);
    uintptr_t candidate = header;
    if (node) {
        do {
            if (Field<uint16_t>(reinterpret_cast<void*>(node), 0x10) < ch) {
                node = Field<uintptr_t>(reinterpret_cast<void*>(node), 0);
            } else {
                candidate = node;
                node = Field<uintptr_t>(reinterpret_cast<void*>(node), 4);
            }
        } while (node);
        if (candidate == header || ch < Field<uint16_t>(reinterpret_cast<void*>(candidate), 0x10))
            candidate = header;
    }
    return static_cast<uint32_t>(candidate);
}

// 0x0045D120/0x0045D260 special-cased control codes (the 0x2000 block only for UTF-16).
inline bool IsSpecialCode(uint32_t ch)
{
    if ((ch & 0xFF80u) == 0x2000u) {
        if (ch - 0x200Bu <= 4u || ch - 0x202Au <= 4u || ch - 0x2060u <= 3u || ch == 0x2028u)
            return true;
    }
    return ch == 0xADu || ch == 0x34Fu || ch == 0xFEFFu;
}

template <class Unit>
int MapGlyphsImpl(void* self, const Unit* src, int count, uint16_t* dst, uint32_t flagDefault,
                  int stride, uint32_t flagMissing)
{
    const Unit* p = src;
    const Unit* end = src + count;
    // The original stores the fallback destination into its own stack argument slot; that
    // slot is dead after `ret N`, so only the buffer case can be observed by a caller.
    uint16_t scratch = 0;
    uint16_t* out = dst ? dst : &scratch;
    int written = 0;
    if (reinterpret_cast<uintptr_t>(p) >= reinterpret_cast<uintptr_t>(end)) return 0;
    for (;;) {
        const uint32_t ch = *p;
        const uint32_t node = FindGlyphNode(self, static_cast<uint16_t>(ch));
        const uintptr_t header = reinterpret_cast<uintptr_t>(Bytes(self) + 0x158);
        bool wrote = true;
        if (node != header) {
            *out = Field<uint16_t>(reinterpret_cast<void*>(node), 0x12);
        } else if (IsSpecialCode(ch)) {
            *out = 0xFFFE;
        } else if (flagDefault & 0xFF) {
            *out = Field<uint16_t>(self, 0x10);
        } else if (flagMissing & 0xFF) {
            *out = 0xFFFF;
        } else {
            wrote = false;
        }
        if (wrote) {
            ++written;
            if (dst) out = reinterpret_cast<uint16_t*>(reinterpret_cast<uint8_t*>(out) + stride);
        }
        ++p;
        if (reinterpret_cast<uintptr_t>(p) >= reinterpret_cast<uintptr_t>(end)) break;
    }
    return written;
}

}  // namespace

// 0x004553A0
void GlyphMetricsMap::InitBaseVtable()
{
    Field<void*>(this, 0) = GamePtr(0x00FA3138u);
}

// 0x004553B0
GlyphMetricsMap* GlyphMetricsMap::DestroyBase(uint32_t flags)
{
    InitBaseVtable();
    if (flags & 1)
        CallGameFunction<void, 0x004E3F70>(this);
    return this;
}

// 0x00455420
GlyphMetricsMap* GlyphMetricsMap::ConstructBase(void* allocator)
{
    InitBaseVtable();
    if (!allocator)
        allocator = CallGameFunction<void*, 0x0044D800>();
    Field<void*>(this, 4) = allocator;
    Field<uint32_t>(this, 8) = 0;
    Field<uint32_t>(this, 0xC) = 0;
    Field<uint16_t>(this, 0x10) = 0;
    Field<uint32_t>(this, 0x14) = 0;
    Field<uint32_t>(this, 0x18) = 0;
    // Original is lock xchg [ecx+1C],0; the returned old value is discarded, so a plain
    // zero store has the same observable state.
    Field<uint32_t>(this, 0x1C) = 0;
    return this;
}

// 0x0045CF20. EAX on the id != 301 path is the caller's register (EA quirk): the rewrite
// returns nothing, but the only observable effect, +0x20, is byte-identical.
void GlyphMetricsMap::SetFlagOnId(uint32_t id, uint32_t value)
{
    if (id == 301)
        Field<uint8_t>(this, 0x20) = value != 0;
}

// 0x0045CF40
uint32_t GlyphMetricsMap::CopyMetricsBlock(void* out)
{
    std::memcpy(out, Bytes(this) + 0x24, 0x74);
    return 1;
}

// 0x0045CF60
uint32_t GlyphMetricsMap::CopyGlyphBlock(void* out)
{
    std::memcpy(out, Bytes(this) + 0x98, 0x44);
    return 1;
}

// 0x0045CF80. Only out is written; EAX leaves as the out pointer with AL cleared.
uintptr_t GlyphMetricsMap::ResetPair(void*, void*, void* out, void*, void*)
{
    Field<uint32_t>(out, 0) = 0;
    Field<uint32_t>(out, 4) = 0;
    return reinterpret_cast<uintptr_t>(out) & ~static_cast<uintptr_t>(0xFFu);
}

// 0x0045CFE0
uintptr_t GlyphMetricsMap::SetAllocator(void* allocator)
{
    const uintptr_t value = SetBaseAllocator(reinterpret_cast<uintptr_t>(allocator));
    Field<void*>(this, 0x120) = allocator;
    Field<void*>(this, 0x148) = allocator;
    Field<void*>(this, 0x16C) = allocator;
    return value;
}

// 0x0045D010
uint32_t GlyphMetricsMap::ParseStream(const void* src, uint32_t size)
{
    alignas(16) uint8_t stream[0x30] = {};
    CallGameMethod<void, 0x004DF120>(stream, src, size, 1u, 0u, Field<void*>(this, 0x148), 0u);
    const uint8_t result = CallGameVirtual<uint8_t, 25>(this, stream);
    Field<void*>(stream, 0) = GamePtr(0x00FB5BCCu);
    void* owned = Field<void*>(stream, 4);
    if (owned)
        CallGameMethod<void, 0x004DEA80>(owned);
    return result;
}

// 0x0045D120
int GlyphMetricsMap::MapGlyphs16(const uint16_t* src, int count, uint16_t* dst,
                                 uint32_t flagDefault, int stride, uint32_t flagMissing)
{
    return MapGlyphsImpl<uint16_t>(this, src, count, dst, flagDefault, stride, flagMissing);
}

// 0x0045D260
int GlyphMetricsMap::MapGlyphs8(const uint8_t* src, int count, uint16_t* dst,
                                uint32_t flagDefault, int stride, uint32_t flagMissing)
{
    return MapGlyphsImpl<uint8_t>(this, src, count, dst, flagDefault, stride, flagMissing);
}

// 0x0045D3A0
uint32_t GlyphMetricsMap::HasGlyph(uint32_t code, int mode)
{
    if (static_cast<uint16_t>(code) < 0x80)
        return 1;
    // 0x0045D430 holds one dispatch byte per mode+1 in [0, 0x1F]: 0 -> sentinel check +
    // tree, 1 -> tree, 2 -> bit set. Only modes -1, 0, 21 and 30 use the tree (the original
    // dispatch table has a 1 at indices 0, 1, 0x16 and 0x1F); every other mode goes to the
    // bit set, keyed by the mode itself rather than by the code.
    const uint32_t idx = static_cast<uint32_t>(mode) + 1u;
    if (idx == 0u || idx == 1u || idx == 0x16u || idx == 0x1Fu) {
        if (idx == 0u && static_cast<uint16_t>(code) == 0xFFFF)
            return 1;
        uint32_t node = 0;
        uint32_t key = code;
        CallGameMethod<uint32_t*, 0x005C9D00>(Bytes(this) + 0x154, &node, &key);
        return node != static_cast<uint32_t>(reinterpret_cast<uintptr_t>(Bytes(this) + 0x158));
    }
    const uint32_t m = static_cast<uint32_t>(mode);
    if (m >= 0x40u) return 0;
    return (Field<uint32_t>(this, 0x14 + 4u * (m >> 5)) & (1u << (m & 31u))) != 0;
}

// 0x0045D5F0
uint32_t GlyphMetricsMap::LookupRecord(uint32_t key, void* out)
{
    uint32_t iterator[2] = {0, 0};
    uint32_t k = key;
    uint32_t* found = CallGameMethod<uint32_t*, 0x0045C0D0>(Bytes(this) + 0x104, iterator, &k);
    const uint32_t node = *found;
    void* buckets = Field<void*>(this, 0x108);
    const uint32_t count = Field<uint32_t>(this, 0x10C);
    if (node == Field<uint32_t>(buckets, 4u * count)) return 0;
    std::memcpy(out, Bytes(reinterpret_cast<void*>(static_cast<uintptr_t>(node))) + 4,
                5 * sizeof(uint32_t));
    return 1;
}

// 0x0045D650. The result is LeaveCriticalSection's leftover EAX, which no caller reads.
void GlyphMetricsMap::SetRecordValue(uint32_t key, uint32_t value)
{
    auto* lock = reinterpret_cast<CRITICAL_SECTION*>(Bytes(this) + 0x178);
    EnterCriticalSection(lock);
    uint32_t iterator[2] = {0, 0};
    uint32_t k = key;
    CallGameMethod<void, 0x0045D4A0>(Bytes(this) + 0x12C, iterator, &k);
    void* buckets = Field<void*>(this, 0x130);
    const uint32_t count = Field<uint32_t>(this, 0x134);
    if (iterator[0] != Field<uint32_t>(buckets, 4u * count))
        Field<uint32_t>(reinterpret_cast<void*>(static_cast<uintptr_t>(iterator[0])), 4) = value;
    LeaveCriticalSection(lock);
}

// 0x0045D6A0
uint32_t GlyphMetricsMap::GetGlyphMetrics(uint32_t* out, uint32_t key, uint32_t, uint32_t,
                                          uint32_t)
{
    auto* lock = reinterpret_cast<CRITICAL_SECTION*>(Bytes(this) + 0x178);
    EnterCriticalSection(lock);
    uint32_t iterator[2] = {0, 0};
    uint32_t k = key;
    CallGameMethod<void, 0x0045D4A0>(Bytes(this) + 0x12C, iterator, &k);
    void* buckets = Field<void*>(this, 0x130);
    const uint32_t count = Field<uint32_t>(this, 0x134);
    uint32_t result = 0;
    if (iterator[0] != Field<uint32_t>(buckets, 4u * count)) {
        CallGameVirtual<void, 13>(this, key, Bytes(this) + 0xE4);
        // x87 fistp with the control word forced to truncation, then the low dword is kept.
        const int64_t first = static_cast<int64_t>(Field<float>(this, 0xE4));
        Field<uint32_t>(this, 0xFC) = 0;
        Field<uint32_t>(this, 0xF8) = static_cast<uint32_t>(iterator[0] + 4);
        Field<uint32_t>(this, 0xDC) = static_cast<uint32_t>(first);
        const int64_t second = static_cast<int64_t>(Field<float>(this, 0xE8));
        Field<uint32_t>(this, 0xE0) = static_cast<uint32_t>(second);
        *out = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(Bytes(this) + 0xDC));
        result = 1;
    }
    LeaveCriticalSection(lock);
    return result;
}

// 0x0045DAB0
GlyphMetricsMap* GlyphMetricsMap::Construct(void* allocator)
{
    ConstructBase(allocator);
    Field<void*>(this, 0) = GamePtr(0x00FA9CC0u);
    Field<uint8_t>(this, 0x20) = 0;
    Field<float>(this, 0x64) = 0.0f;
    Field<uint32_t>(this, 0x68) = 0;
    Field<float>(this, 0x6C) = GameFloat(0x00FA1BACu);
    Field<float>(this, 0x70) = GameFloat(0x0107A538u);
    Field<uint32_t>(this, 0x74) = 0;
    Field<uint32_t>(this, 0x78) = 0;
    Field<uint32_t>(this, 0x7C) = 0;
    Field<uint32_t>(this, 0x80) = 0;
    Field<float>(this, 0x84) = GameFloat(0x0107A538u);
    Field<float>(this, 0x88) = GameFloat(0x0107A538u);
    Field<uint32_t>(this, 0x8C) = 0xFFFFFFFFu;
    Field<uint32_t>(this, 0x90) = 0xFF000000u;
    Field<uint32_t>(this, 0x94) = 0xFFFFFFFFu;
    Field<uint16_t>(this, 0x24) = 0;
    std::memset(Bytes(this) + 0x98, 0, 0x44);
    void* alloc = Field<void*>(this, 4);
    Field<float>(this, 0x114) = GameFloat(0x0107A538u);
    Field<float>(this, 0x118) = GameFloat(0x00F98A58u);
    Field<void*>(this, 0x120) = alloc;
    Field<uint32_t>(this, 0x124) = 0;
    Field<void*>(this, 0x128) = GamePtr(0x00FA9C98u);
    Field<uint32_t>(this, 0x110) = 0;
    Field<uint32_t>(this, 0x11C) = 0;
    Field<uint32_t>(this, 0x10C) = 1;
    Field<void*>(this, 0x108) = GamePtr(0x0114C900u);
    Field<float>(this, 0x13C) = GameFloat(0x0107A538u);
    Field<float>(this, 0x140) = GameFloat(0x00F98A58u);
    Field<void*>(this, 0x148) = alloc;
    Field<uint32_t>(this, 0x14C) = 0;
    Field<void*>(this, 0x150) = GamePtr(0x00FA9C98u);
    Field<void*>(this, 0x130) = GamePtr(0x0114C900u);
    Field<uint32_t>(this, 0x134) = 1;
    Field<uint32_t>(this, 0x138) = 0;
    Field<uint32_t>(this, 0x144) = 0;
    // The +0x158 std::map-like header: left = right = itself, parent (root) = null.
    Field<void*>(this, 0x158) = Bytes(this) + 0x158;
    Field<void*>(this, 0x15C) = Bytes(this) + 0x158;
    Field<uint32_t>(this, 0x160) = 0;
    // The original zeroes the whole dword at +0x164 (mov [eax+0xC] with eax = this+0x158)
    // and then redundantly rewrites its low byte.
    Field<uint32_t>(this, 0x164) = 0;
    Field<uint32_t>(this, 0x168) = 0;
    Field<void*>(this, 0x16C) = alloc;
    Field<uint32_t>(this, 0x170) = 0;
    Field<void*>(this, 0x174) = GamePtr(0x00FA9C98u);
    InitializeCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(Bytes(this) + 0x178));
    return this;
}

// 0x0045DC30
uint32_t GlyphMetricsMap::DestroyContent()
{
    auto* lock = reinterpret_cast<CRITICAL_SECTION*>(Bytes(this) + 0x178);
    EnterCriticalSection(lock);
    if (Field<uint8_t>(this, 0x20) == 0) {
        auto* buckets = reinterpret_cast<uint8_t**>(Field<void*>(this, 0x130));
        const uint32_t count = Field<uint32_t>(this, 0x134);
        const uintptr_t end = reinterpret_cast<uintptr_t>(buckets[count]);
        uint8_t** cursor = buckets;
        uint8_t* node = *cursor;
        if (!node) {
            do {
                ++cursor;
            } while (*cursor == nullptr);
            node = *cursor;
        }
        while (reinterpret_cast<uintptr_t>(node) != end) {
            auto* payload = reinterpret_cast<uint8_t*>(Field<void*>(node, 4));
            void* first = Field<void*>(payload, 0x1C);
            if (first) {
                const int32_t span = static_cast<int32_t>(Field<uint32_t>(payload, 0x24) -
                                                           Field<uint32_t>(payload, 0x1C));
                const uint32_t size = static_cast<uint32_t>((span >> 1) << 1);
                AllocatorFree(Field<void*>(payload, 0x28), first, size);
            }
            if (Field<void*>(payload, 4)) {
                const int32_t diff = static_cast<int32_t>(Field<uint32_t>(payload, 0xC) -
                                                          Field<uint32_t>(payload, 4));
                const uint32_t size = static_cast<uint32_t>(DivideBy24(diff)) * 24u;
                AllocatorFree(Field<void*>(payload, 0x10), Field<void*>(payload, 4), size);
            }
            AllocatorFree(Field<void*>(this, 0x148), payload, 4);
            node = reinterpret_cast<uint8_t*>(Field<void*>(node, 8));
            if (!node) {
                do {
                    ++cursor;
                    node = *cursor;
                } while (node == nullptr);
            }
        }
    }
    LeaveCriticalSection(lock);
    return 1;
}

// 0x0045DFA0
void GlyphMetricsMap::Destroy()
{
    Field<void*>(this, 0) = GamePtr(0x00FA9CC0u);
    DestroyContent();
    DeleteCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(Bytes(this) + 0x178));
    CallGameMethod<void, 0x0044DCE0>(Bytes(this) + 0x154, Field<void*>(this, 0x160));
    CallGameMethod<void, 0x0044C6F0>(Bytes(this) + 0x12C, Field<void*>(this, 0x130),
                                     Field<uint32_t>(this, 0x134));
    uint32_t count = Field<uint32_t>(this, 0x134);
    Field<uint32_t>(this, 0x138) = 0;
    if (count > 1)
        AllocatorFree(Field<void*>(this, 0x148), Field<void*>(this, 0x130), count * 4u + 4u);
    CallGameMethod<void, 0x0045B9C0>(Bytes(this) + 0x104, Field<void*>(this, 0x108),
                                     Field<uint32_t>(this, 0x10C));
    count = Field<uint32_t>(this, 0x10C);
    Field<uint32_t>(this, 0x110) = 0;
    if (count > 1)
        AllocatorFree(Field<void*>(this, 0x120), Field<void*>(this, 0x108), count * 4u + 4u);
    InitBaseVtable();
}

// 0x0045E100
GlyphMetricsMap* GlyphMetricsMap::DeleteAndFree(uint32_t flags)
{
    Destroy();
    if (flags & 1)
        CallGameFunction<void, 0x004E3F70>(this);
    return this;
}

// 0x00534E60
uintptr_t GlyphMetricsMap::SetBaseAllocator(uintptr_t value)
{
    Field<uintptr_t>(this, 4) = value;
    return value;
}

// 0x00565F20
float GlyphMetricsMap::GetScale() const
{
    return Field<float>(this, 0x64);
}

}  // namespace ts3::misc