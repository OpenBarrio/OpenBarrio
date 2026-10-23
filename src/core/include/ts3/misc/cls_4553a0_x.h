// Unit cls_4553a0_x: TS3W.exe PolygonFont GlyphMetricsMap (0x004553A0 ... 0x00565F20).
// The class is a game object, never constructed by this code: only the x86 byte layout
// below is meaningful (TS3_X86_OFFSET-style offsets in the .cpp). Pointer fields are
// void*/uintptr_t so the x64 ts3core build keeps them whole, but the methods only run in
// the x86 harness (every game address/call is guarded by _M_IX86).
#pragma once

#include <cstdint>

namespace ts3::misc {

class GlyphMetricsMap {
public:
    GlyphMetricsMap() = delete;
    GlyphMetricsMap(const GlyphMetricsMap&) = delete;
    GlyphMetricsMap& operator=(const GlyphMetricsMap&) = delete;

    // TS3W.exe 0x004553A0: store the base vtable 0x00FA3138 at +0. No result.
    void InitBaseVtable();
    // TS3W.exe 0x004553B0: base scalar-deleting destructor: base vtable, free this through
    // 0x004E3F70 when flags&1, return this.
    GlyphMetricsMap* DestroyBase(uint32_t flags);
    // TS3W.exe 0x00455420: base constructor; a null allocator uses the global 0x0044D800.
    // Clears +4/+8/+C/10/14/18 and atomically zeroes +1C.
    GlyphMetricsMap* ConstructBase(void* allocator);

    // TS3W.exe 0x0045CF20: vtable slot. Only when id == 301 stores (value != 0) at +0x20
    // and leaves that bool in AL; on every other id EAX is untouched (EA quirk - callers
    // ignore the result, so the rewrite takes no return value).
    void SetFlagOnId(uint32_t id, uint32_t value);
    // TS3W.exe 0x0045CF40: copy the 0x74 bytes at +0x24 into out; returns 1 in AL.
    uint32_t CopyMetricsBlock(void* out);
    // TS3W.exe 0x0045CF60: copy the 0x44 bytes at +0x98 into out; returns 1 in AL.
    uint32_t CopyGlyphBlock(void* out);
    // TS3W.exe 0x0045CF80: out[0] = out[1] = 0; the result is the out pointer with AL
    // cleared (EA quirk: the upper EAX bits are the pointer).
    uintptr_t ResetPair(void* a0, void* a1, void* out, void* a3, void* a4);
    // TS3W.exe 0x0045CFE0: base id setter (0x00534E60) plus allocator copies at +0x120,
    // +0x148, +0x16C; returns the argument.
    uintptr_t SetAllocator(void* allocator);

    // TS3W.exe 0x0045D010: build an EaMemoryStream (0x004DF120) over (src, size) for
    // this+0x148 and hand it to vtable slot 25; releases stream+4 through 0x004DEA80 and
    // restores the EaMemoryStream vtable 0x00FB5BCC. Returns the slot's AL.
    uint32_t ParseStream(const void* src, uint32_t size);
    // TS3W.exe 0x0045D120: glyph-map a UTF-16 buffer through the tree at +0x158/+0x160.
    // Writes a 16-bit value per source unit; returns how many were written. dst == null
    // writes into the caller's stack slot (EA quirk kept, unobservable to callers).
    // flagDefault (low byte) selects the +0x10 fallback, flagMissing (low byte) 0xFFFF.
    int MapGlyphs16(const uint16_t* src, int count, uint16_t* dst, uint32_t flagDefault,
                    int stride, uint32_t flagMissing);
    // TS3W.exe 0x0045D260: the same for a byte buffer (only 0xAD is special).
    int MapGlyphs8(const uint8_t* src, int count, uint16_t* dst, uint32_t flagDefault,
                   int stride, uint32_t flagMissing);
    // TS3W.exe 0x0045D3A0: code < 0x80 is always present; mode -1 first maps 0xFFFF to
    // present, then modes -1/0/21/30 look the code up in the map at +0x154 (0x005C9D00);
    // every other mode in 0..0x3F tests the bit set at +0x14 indexed by the mode itself
    // (EA quirk), and a mode >= 0x40 is absent. Result is a bool in AL.
    uint32_t HasGlyph(uint32_t code, int mode);
    // TS3W.exe 0x0045D5F0: fetch the 5 dwords after the node found by 0x0045C0D0 in the map
    // at +0x104; returns 1 on a hit, 0 on the end sentinel.
    uint32_t LookupRecord(uint32_t key, void* out);
    // TS3W.exe 0x0045D650: set the dword at node+4 for key in the map at +0x12C under the
    // lock at +0x178. Result is LeaveCriticalSection's EAX (undefined - declared void).
    void SetRecordValue(uint32_t key, uint32_t value);
    // TS3W.exe 0x0045D6A0: lookup key in the map at +0x12C under the lock, ask vtable slot
    // 13 for metrics into +0xE4, truncate +0xE4/+0xE8 into +0xDC/+0xE0, publish those at
    // *out and link the node at +0xF8. Returns 1 on a hit, 0 otherwise.
    uint32_t GetGlyphMetrics(uint32_t* out, uint32_t key, uint32_t a2, uint32_t a3, uint32_t a4);

    // TS3W.exe 0x0045DAB0: full constructor: base constructor, derived vtable 0x00FA9CC0,
    // font defaults, three containers (+0x104, +0x12C, the +0x158 tree) and the lock.
    GlyphMetricsMap* Construct(void* allocator);
    // TS3W.exe 0x0045DC30: under the lock release the record payload arrays through the
    // allocator vtables of the map at +0x12C; skips everything when +0x20 is set.
    // Returns 1 in AL.
    uint32_t DestroyContent();
    // TS3W.exe 0x0045DFA0: destroy body: vtable, DestroyContent, DeleteCriticalSection,
    // free the +0x158 tree (0x0044DCE0), destroy both hash maps (0x0044C6F0, 0x0045B9C0)
    // and finally restore the base vtable. No result.
    void Destroy();
    // TS3W.exe 0x0045E100: deleting destructor: Destroy, free this through 0x004E3F70 when
    // flags&1, return this.
    GlyphMetricsMap* DeleteAndFree(uint32_t flags);

    // TS3W.exe 0x00534E60: shared id setter used by many classes: +4 = value, return it.
    uintptr_t SetBaseAllocator(uintptr_t value);
    // TS3W.exe 0x00565F20: the float at +0x64.
    float GetScale() const;

private:
    uint8_t storage_[0x1A0];
};

}  // namespace ts3::misc