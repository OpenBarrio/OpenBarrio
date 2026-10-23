#pragma once
#include "ts3/layout.h"
namespace ts3::audio::b32_detail {
struct Composite { void* next; void* vtable; uint32_t refs; void* first; void* second; };
struct Collection { uint32_t unknown; void* left; void* right; void* root; uint8_t flag; uint32_t count; };
struct BitReader { const uint8_t* data; uint32_t position; };
// TS3W.exe 0x008A07E0: Forwards to the first successful delegate, testing AL.
uint32_t Forward14(Composite*, uint32_t, uint32_t);
// TS3W.exe 0x008A0830: Forwards slot 15, testing AL.
uint32_t Forward15(Composite*, uint32_t, uint32_t, uint32_t);
// TS3W.exe 0x008A0890: Returns the first nonzero slot 16 result.
uint32_t Forward16(Composite*, uint32_t);
// TS3W.exe 0x008A08D0: Returns the first nonzero slot 17 result.
uint32_t Forward17(Composite*, uint32_t);
// TS3W.exe 0x008A0910: Forwards slot 18, testing AL.
uint32_t Forward18(Composite*, uint32_t, uint32_t, uint32_t);
// TS3W.exe 0x008A0970: Forwards slot 19, testing AL.
uint32_t Forward19(Composite*, uint32_t, uint32_t, uint32_t);
// TS3W.exe 0x008A09D0: Forwards slot 20, testing AL.
uint32_t Forward20(Composite*, uint32_t, uint32_t, uint32_t);
// TS3W.exe 0x008A0A30: Releases second then first and optionally returns storage to the pool.
Composite* Destroy(Composite*, uint32_t);
// TS3W.exe 0x008A0A80: Optionally clears the collection then visits both delegates.
void Collect(Composite*, Collection*, uint8_t);
// TS3W.exe 0x008A0AE0: Reads MSB-first bits with unsigned 32-bit accumulation.
uint32_t ReadBits(BitReader*, uint32_t);
// TS3W.exe 0x008A0B60: Sets or clears the requested emitter flags.
uint32_t SetFlags(void*, uint32_t, uint8_t);
// TS3W.exe 0x008A0B90: Sets the active bit before command dispatch.
uint8_t Start(void*);
// TS3W.exe 0x008A0BD0: Calls slot 25 and updates flags only when bit 16 is set.
uint8_t Resume(void*);
// TS3W.exe 0x008A0C00: Returns the byte at emitter offset 0x3C8 (verified getter twin).
uint8_t GetFlag(const void*);
}

// ABI view used by generated thiscall adapters. No native object is constructed:
// each member forwards its original this pointer to the typed implementation.
namespace ts3::audio {
struct b32 {
    uint32_t Forward14(uint32_t a0, uint32_t a1) { return b32_detail::Forward14(reinterpret_cast<b32_detail::Composite*>(this), a0, a1); }
    uint32_t Forward15(uint32_t a0, uint32_t a1, uint32_t a2) { return b32_detail::Forward15(reinterpret_cast<b32_detail::Composite*>(this), a0, a1, a2); }
    uint32_t Forward16(uint32_t a0) { return b32_detail::Forward16(reinterpret_cast<b32_detail::Composite*>(this), a0); }
    uint32_t Forward17(uint32_t a0) { return b32_detail::Forward17(reinterpret_cast<b32_detail::Composite*>(this), a0); }
    uint32_t Forward18(uint32_t a0, uint32_t a1, uint32_t a2) { return b32_detail::Forward18(reinterpret_cast<b32_detail::Composite*>(this), a0, a1, a2); }
    uint32_t Forward19(uint32_t a0, uint32_t a1, uint32_t a2) { return b32_detail::Forward19(reinterpret_cast<b32_detail::Composite*>(this), a0, a1, a2); }
    uint32_t Forward20(uint32_t a0, uint32_t a1, uint32_t a2) { return b32_detail::Forward20(reinterpret_cast<b32_detail::Composite*>(this), a0, a1, a2); }
    b32_detail::Composite* Destroy(uint32_t a0) { return b32_detail::Destroy(reinterpret_cast<b32_detail::Composite*>(this), a0); }
    void Collect(void* a0, uint8_t a1) { return b32_detail::Collect(reinterpret_cast<b32_detail::Composite*>(this), static_cast<b32_detail::Collection*>(a0), a1); }
    uint32_t ReadBits(uint32_t a0) { return b32_detail::ReadBits(reinterpret_cast<b32_detail::BitReader*>(this), a0); }
    uint32_t SetFlags(uint32_t a0, uint8_t a1) { return b32_detail::SetFlags(reinterpret_cast<void*>(this), a0, a1); }
    uint8_t Start() { return b32_detail::Start(reinterpret_cast<void*>(this)); }
    uint8_t Resume() { return b32_detail::Resume(reinterpret_cast<void*>(this)); }
    uint8_t GetFlag() { return b32_detail::GetFlag(reinterpret_cast<const void*>(this)); }
};
}
