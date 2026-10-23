// Unit cls_6ef780_Renderer - TS3W.exe 0x006EF780. See the header for the contract.
//
// 0x006EF780 is the 5679-byte scene-manager/renderer constructor. It installs the two
// base vtables, clears the object, builds the 22-entry culling-flag adapter table and
// the 16-entry visualisation adapter table, registers the renderer object with the
// global config services (0x5A00A0) and finishes with the mission settings reads
// (0x58AAD0). The body below is a 1:1 transcription of the disassembly listing
// (the original disassembly); every generated line carries the
// original address, and the register variables mirror the original ones. There are no
// loops in the original; all branches are forward, reproduced with labels + goto.
//
// Every callee belongs to an unrewritten unit, so all of them go through the layout.h
// bridge (CallGameMethod / CallGameFunction / CallGameVirtual). The game vtables,
// descriptor pointers and wide-string addresses are 32-bit x86 image addresses and
// stay inside the `#if defined(_M_IX86)` body; the x64 build is a stub.
//
// EA behaviour kept on purpose:
//  * the base-class vtable word at +0x0 is first set to 0xFEFEC0, then the
//    `xchg [this+4], eax` (0x6EF797) zeroes the word at +4 and discards the old
//    value, and only then is the derived vtable 0xFF91B8 installed.
//  * 0x005A00A0 is called once per registration (72 times) and its result is only
//    used when non-null; the rewrite calls it the same number of times in order.
//  * the setting selects in the tail test AL of 0x58AAD0's result (the original instruction sequence),
//    so only the low byte matters; the float compare at 0x6F0D57 is COMISS + JBE,
//    so a NaN epsilon picks the +0x540 arm (written as `!(xmm0 > value)`).
//  * the constructor returns `this` in EAX on all three exit paths.

#include <cstddef>
#include <cstdint>

#include "ts3/layout.h"
#include "ts3/renderer/cls_6ef780_Renderer.h"

#if defined(_M_IX86)
#include <intrin.h>
#endif

namespace ts3::renderer {

#if defined(_M_IX86)
namespace {

inline void Store32(void* base, std::size_t off, std::uint32_t v) {
  *reinterpret_cast<std::uint32_t*>(static_cast<std::uint8_t*>(base) + off) = v;
}
inline void Store8(void* base, std::size_t off, std::uint8_t v) {
  *reinterpret_cast<std::uint8_t*>(static_cast<std::uint8_t*>(base) + off) = v;
}
inline void StoreFloat(void* base, std::size_t off, float v) {
  *reinterpret_cast<float*>(static_cast<std::uint8_t*>(base) + off) = v;
}
inline std::uint32_t Load32(const void* base, std::size_t off) {
  return *reinterpret_cast<const std::uint32_t*>(static_cast<const std::uint8_t*>(base) + off);
}
inline void* LoadPtr(const void* base, std::size_t off) {
  return *reinterpret_cast<void* const*>(static_cast<const std::uint8_t*>(base) + off);
}
inline float LoadFloat(const void* base, std::size_t off) {
  return *reinterpret_cast<const float*>(static_cast<const std::uint8_t*>(base) + off);
}
inline std::uint32_t Ptr32(const void* p) {
  return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p));
}
inline std::uint32_t LoadGame32(std::uintptr_t addr) {
  return *reinterpret_cast<const std::uint32_t*>(addr);
}
inline void* LoadGamePtr(std::uintptr_t addr) {
  return *reinterpret_cast<void* const*>(addr);
}
inline void StoreGame32(std::uintptr_t addr, std::uint32_t v) {
  *reinterpret_cast<std::uint32_t*>(addr) = v;
}
inline std::uint32_t Exchange32(void* p, std::uint32_t v) {
  return static_cast<std::uint32_t>(
      _InterlockedExchange(reinterpret_cast<volatile long*>(p), static_cast<long>(v)));
}

}  // namespace
#endif

void* __fastcall Construct6EF780(void* self_, void* /*edx*/) {
#if defined(_M_IX86)
  std::uint8_t* const self = static_cast<std::uint8_t*>(self_);
  void* eax = nullptr;
  void* ebx = nullptr;
  void* ecx = nullptr;
  void* edx = nullptr;
  void* edi = nullptr;
  void* const esi = self;
  float xmm0 = 0.0f;
  float xmm1 = 0.0f;
  void* a0 = nullptr;
  void* a1 = nullptr;
  void* a2 = nullptr;
  void* a3 = nullptr;
  void* a4 = nullptr;
  void* a5 = nullptr;
  void* a6 = nullptr;
  void* a7 = nullptr;
  void* a8 = nullptr;
  void* a9 = nullptr;
  void* a10 = nullptr;
  void* a11 = nullptr;
  void* a12 = nullptr;
  void* a13 = nullptr;
  void* a14 = nullptr;
  void* a15 = nullptr;
  void* a16 = nullptr;
  void* a17 = nullptr;
  void* a18 = nullptr;
  void* a19 = nullptr;
  void* a20 = nullptr;
  void* a21 = nullptr;
  void* a22 = nullptr;
  void* a23 = nullptr;
  void* a24 = nullptr;
  void* a25 = nullptr;
  void* a26 = nullptr;
  void* a27 = nullptr;
  void* a28 = nullptr;
  void* a29 = nullptr;
  void* a30 = nullptr;
  void* a31 = nullptr;
  void* a32 = nullptr;
  void* a33 = nullptr;
  void* a34 = nullptr;
  void* a35 = nullptr;
  void* a36 = nullptr;
  void* a37 = nullptr;
  void* a38 = nullptr;
  void* a39 = nullptr;
  void* a40 = nullptr;
  void* a41 = nullptr;
  void* a42 = nullptr;
  void* a43 = nullptr;
  void* a44 = nullptr;
  void* a45 = nullptr;
  void* a46 = nullptr;
  void* a47 = nullptr;
  void* a48 = nullptr;
  void* a49 = nullptr;
  void* a50 = nullptr;
  void* a51 = nullptr;
  void* a52 = nullptr;
  void* a53 = nullptr;
  void* a54 = nullptr;
  void* a55 = nullptr;
  void* a56 = nullptr;
  void* a57 = nullptr;
  void* a58 = nullptr;
  void* a59 = nullptr;
  void* a60 = nullptr;
  void* a61 = nullptr;
  void* a62 = nullptr;
  void* a63 = nullptr;
  void* a64 = nullptr;
  void* a65 = nullptr;
  float f0 = 0.0f;
  float f1 = 0.0f;
  float f2 = 0.0f;
  float f3 = 0.0f;
  float f4 = 0.0f;
  float f5 = 0.0f;
  float f6 = 0.0f;
  float f7 = 0.0f;
  float f8 = 0.0f;
  float f9 = 0.0f;
  float f10 = 0.0f;
  float f11 = 0.0f;
  float f12 = 0.0f;
  float f13 = 0.0f;
  float f14 = 0.0f;
  void* l0 = nullptr;
  void* l1 = nullptr;
  void* l2 = nullptr;
  void* l3 = nullptr;
  void* l4 = nullptr;
  void* l5 = nullptr;
  void* l6 = nullptr;
  void* l7 = nullptr;
  void* l8 = nullptr;
  void* l9 = nullptr;
  void* l10 = nullptr;
  void* l11 = nullptr;
  void* l12 = nullptr;
  void* l13 = nullptr;
  void* l14 = nullptr;
  void* l15 = nullptr;
  void* l16 = nullptr;
  void* l17 = nullptr;
  void* l18 = nullptr;
  void* l19 = nullptr;
  void* l20 = nullptr;
  void* l21 = nullptr;
  // 0x006EF78C
  Store32(esi, 0x0, 0xFEFEC0u);
  // 0x006EF792
  eax = nullptr;
  // 0x006EF794
  ecx = static_cast<std::uint8_t*>(self) + 0x4;
  // 006EF797  xchg dword ptr [ecx], eax
  (void)Exchange32(ecx, Ptr32(eax));
  // 0x006EF799
  ebx = nullptr;
  // 0x006EF79B
  Store32(esi, 0x0, 0xFF91B8u);
  // 0x006EF7A1
  Store32(esi, 0x8, Ptr32(ebx));
  // 0x006EF7A4
  Store32(esi, 0xC, Ptr32(ebx));
  // 0x006EF7A7
  Store32(esi, 0x10, Ptr32(ebx));
  // 0x006EF7AA
  Store32(esi, 0x14, Ptr32(ebx));
  // 0x006EF7AD
  Store32(esi, 0x18, Ptr32(ebx));
  // 0x006EF7B0
  Store32(esi, 0x1C, Ptr32(ebx));
  // 0x006EF7B3
  Store32(esi, 0x20, Ptr32(ebx));
  // 0x006EF7B6
  Store32(esi, 0x24, Ptr32(ebx));
  // 0x006EF7B9
  Store32(esi, 0x28, Ptr32(ebx));
  // 0x006EF7BC
  Store32(esi, 0x2C, Ptr32(ebx));
  // 0x006EF7BF
  Store32(esi, 0x30, Ptr32(ebx));
  // 0x006EF7C2
  Store32(esi, 0x34, Ptr32(ebx));
  // 0x006EF7C5
  Store32(esi, 0x38, Ptr32(ebx));
  // 006EF7C8  xorps xmm0, xmm0
  xmm0 = 0.0f;
  // 0x006EF7CB
  Store32(self, 0xC0, 0u);
  Store32(self, 0xC4, 0u);
  Store32(self, 0xC8, 0u);
  Store32(self, 0xCC, 0u);
  // 0x006EF7D2
  Store32(self, 0x160, 0u);
  Store32(self, 0x164, 0u);
  Store32(self, 0x168, 0u);
  Store32(self, 0x16C, 0u);
  // 0x006EF7D9
  ecx = static_cast<std::uint8_t*>(self) + 0x1A0;
  // 0x006EF7DF
  Store8(esi, 0x194, 0u);
  // 0x006EF7E5
  CallGameMethod<void, 0x00723560>(reinterpret_cast<void*>(ecx));
  // 0x006EF7EA
  xmm0 = 1.0f;
  // 0x006EF7F2
  edi = reinterpret_cast<void*>(static_cast<std::uintptr_t>(0xFFFFFFFFu));
  // 0x006EF7F6
  StoreFloat(self, 0x1B8, xmm0);
  // 0x006EF7FE
  xmm0 = 2.0f;
  // 0x006EF808
  ecx = static_cast<std::uint8_t*>(self) + 0x238;
  // 0x006EF80E
  StoreFloat(self, 0x1BC, xmm0);
  // 0x006EF816
  Store32(esi, 0x1C0, Ptr32(ebx));
  // 0x006EF81C
  Store32(esi, 0x1CC, Ptr32(ebx));
  // 0x006EF822
  Store32(esi, 0x1EC, Ptr32(edi));
  // 0x006EF828
  Store32(esi, 0x1F0, Ptr32(ebx));
  // 0x006EF82E
  Store8(esi, 0x1F4, 0u);
  // 0x006EF834
  Store8(esi, 0x1F5, 0u);
  // 0x006EF83A
  Store32(esi, 0x1F8, Ptr32(ebx));
  // 0x006EF840
  Store32(esi, 0x234, Ptr32(edi));
  // 0x006EF846
  CallGameMethod<void, 0x004F35B0>(reinterpret_cast<void*>(ecx), 0x5u, 0u);
  // 006EF84B  xorps xmm0, xmm0
  xmm0 = 0.0f;
  // 0x006EF84E
  Store32(esi, 0x250, Ptr32(ebx));
  // 0x006EF854
  StoreFloat(self, 0x254, xmm0);
  // 0x006EF85C
  StoreFloat(self, 0x258, xmm0);
  // 0x006EF864
  Store8(esi, 0x25C, 0u);
  // 0x006EF86A
  Store8(esi, 0x25D, 0u);
  // 0x006EF870
  Store8(esi, 0x25E, 0u);
  // 0x006EF876
  Store8(esi, 0x25F, 0u);
  // 0x006EF87C
  Store8(esi, 0x260, 1u);
  // 0x006EF883
  Store8(esi, 0x261, 1u);
  // 0x006EF88A
  Store8(esi, 0x262, 1u);
  // 0x006EF891
  Store8(esi, 0x263, 1u);
  // 0x006EF898
  Store8(esi, 0x264, 0u);
  // 0x006EF89E
  Store8(esi, 0x265, 1u);
  // 0x006EF8A5
  Store8(esi, 0x266, 0u);
  // 0x006EF8AB
  Store8(esi, 0x267, 1u);
  // 0x006EF8B2
  Store8(esi, 0x268, 1u);
  // 0x006EF8B9
  Store8(esi, 0x269, 0u);
  // 0x006EF8BF
  Store8(esi, 0x26A, 0u);
  // 0x006EF8C5
  Store8(esi, 0x26B, 1u);
  // 0x006EF8CC
  Store8(esi, 0x26C, 0u);
  // 0x006EF8D2
  Store8(esi, 0x26D, 0u);
  // 0x006EF8D8
  Store8(esi, 0x26E, 0u);
  // 0x006EF8DE
  Store8(esi, 0x26F, 0u);
  // 0x006EF8E4
  Store8(esi, 0x270, 0u);
  // 0x006EF8EA
  Store8(esi, 0x271, 0u);
  // 0x006EF8F0
  Store8(esi, 0x272, 1u);
  // 0x006EF8F7
  Store8(esi, 0x273, 0u);
  // 0x006EF8FD
  Store8(esi, 0x274, 0u);
  // 0x006EF903
  Store8(esi, 0x275, 0u);
  // 0x006EF909
  Store8(esi, 0x276, 0u);
  // 0x006EF90F
  Store8(esi, 0x277, 0u);
  // 0x006EF915
  Store8(esi, 0x278, 0u);
  // 0x006EF91B
  Store8(esi, 0x279, 0u);
  // 0x006EF921
  Store8(esi, 0x27A, 1u);
  // 0x006EF928
  Store8(esi, 0x27B, 0u);
  // 0x006EF92E
  Store8(esi, 0x27C, 0u);
  // 0x006EF934
  Store32(esi, 0x280, Ptr32(ebx));
  // 0x006EF93A
  Store32(esi, 0x284, 0x400u);
  // 0x006EF944
  Store32(esi, 0x288, Ptr32(edi));
  // 0x006EF94A
  Store8(esi, 0x28C, 0u);
  // 0x006EF950
  Store8(esi, 0x28D, 0u);
  // 0x006EF956
  Store8(esi, 0x28E, 0u);
  // 0x006EF95C
  Store8(esi, 0x28F, 0u);
  // 0x006EF962
  Store8(esi, 0x290, 0u);
  // 0x006EF968
  Store8(esi, 0x291, 0u);
  // 0x006EF96E
  Store32(esi, 0x294, Ptr32(ebx));
  // 0x006EF974
  Store32(esi, 0x298, 0x3E8u);
  // 0x006EF97E
  Store8(esi, 0x29C, 0u);
  // 0x006EF984
  Store8(esi, 0x29D, 1u);
  // 0x006EF98B
  Store8(esi, 0x29E, 1u);
  // 0x006EF992
  Store8(esi, 0x29F, 1u);
  // 0x006EF999
  Store8(esi, 0x2A0, 0u);
  // 0x006EF99F
  Store8(esi, 0x2A1, 0u);
  // 0x006EF9A5
  Store8(esi, 0x2A2, 0u);
  // 0x006EF9AB
  Store8(esi, 0x2A3, 0u);
  // 0x006EF9B1
  edx = nullptr;
  // 0x006EF9B3
  eax = static_cast<std::uint8_t*>(self) + 0x2A4;
  // 006EF9B9  xchg dword ptr [eax], edx
  (void)Exchange32(eax, Ptr32(edx));
  // 0x006EF9BB
  xmm1 = 60.0f;
  // 0x006EF9C3
  StoreFloat(self, 0x2A8, xmm1);
  // 0x006EF9CB
  xmm1 = -1.0f;
  // 0x006EF9D3
  StoreFloat(self, 0x2B0, xmm0);
  // 0x006EF9DB
  xmm0 = 1.0f;
  // 0x006EF9E3
  StoreFloat(self, 0x2AC, xmm1);
  // 0x006EF9EB
  StoreFloat(self, 0x2B4, xmm0);
  // 0x006EF9F3
  StoreFloat(self, 0x2B8, xmm0);
  // 0x006EF9FB
  StoreFloat(self, 0x2BC, xmm0);
  // 0x006EFA03
  Store32(esi, 0x2C0, 0x1u);
  // 0x006EFA0D
  Store32(esi, 0x2C4, 0x7FFFF000u);
  // 0x006EFA17
  Store32(esi, 0x2C8, Ptr32(edi));
  // 0x006EFA1D
  Store32(esi, 0x2CC, Ptr32(edi));
  // 0x006EFA23
  Store32(esi, 0x2D0, 0x1446A000u);
  // 0x006EFA2D
  Store32(esi, 0x2D4, Ptr32(ebx));
  // 0x006EFA33
  Store32(esi, 0x2D8, Ptr32(ebx));
  // 0x006EFA39
  Store32(esi, 0x2DC, Ptr32(ebx));
  // 0x006EFA3F
  Store32(esi, 0x2E0, Ptr32(ebx));
  // 0x006EFA45
  Store32(esi, 0x2E4, Ptr32(ebx));
  // 0x006EFA4B
  Store32(esi, 0x2E8, Ptr32(ebx));
  // 0x006EFA51
  Store32(esi, 0x2EC, Ptr32(ebx));
  // 0x006EFA57
  Store32(esi, 0x2F0, Ptr32(ebx));
  // 0x006EFA5D
  Store32(esi, 0x2F4, Ptr32(ebx));
  // 0x006EFA63
  Store32(esi, 0x2F8, Ptr32(ebx));
  // 0x006EFA69
  Store32(esi, 0x2FC, Ptr32(ebx));
  // 0x006EFA6F
  Store32(esi, 0x300, Ptr32(ebx));
  // 0x006EFA75
  Store32(esi, 0x304, Ptr32(ebx));
  // 0x006EFA7B
  Store32(esi, 0x308, Ptr32(ebx));
  // 0x006EFA81
  Store32(esi, 0x30C, Ptr32(ebx));
  // 0x006EFA87
  Store32(esi, 0x310, Ptr32(ebx));
  // 0x006EFA8D
  Store32(esi, 0x314, Ptr32(ebx));
  // 0x006EFA93
  Store32(esi, 0x318, Ptr32(ebx));
  // 0x006EFA99
  Store32(esi, 0x31C, Ptr32(ebx));
  // 0x006EFA9F
  Store32(esi, 0x320, Ptr32(ebx));
  // 0x006EFAA5
  ecx = static_cast<std::uint8_t*>(self) + 0x328;
  // 0x006EFAAB
  Store32(esi, 0x324, Ptr32(ebx));
  // 0x006EFAB1
  CallGameMethod<void, 0x006C8320>(reinterpret_cast<void*>(ecx));
  // 0x006EFAB6
  ecx = static_cast<std::uint8_t*>(self) + 0x344;
  // 0x006EFABC
  CallGameMethod<void, 0x006C8320>(reinterpret_cast<void*>(ecx));
  // 006EFAC1  xorps xmm0, xmm0
  xmm0 = 0.0f;
  // 0x006EFAC4
  Store32(esi, 0x360, Ptr32(ebx));
  // 0x006EFACA
  Store32(esi, 0x364, Ptr32(ebx));
  // 0x006EFAD0
  Store32(esi, 0x36C, Ptr32(ebx));
  // 0x006EFAD6
  StoreFloat(self, 0x370, xmm0);
  // 0x006EFADE
  Store32(esi, 0x374, Ptr32(ebx));
  // 0x006EFAE4
  Store8(esi, 0x378, 0u);
  // 0x006EFAEA
  Store32(esi, 0x3C0, Ptr32(ebx));
  // 0x006EFAF0
  Store32(esi, 0x3C4, Ptr32(ebx));
  // 0x006EFAF6
  ecx = static_cast<std::uint8_t*>(self) + 0x4D4;
  // 0x006EFAFC
  Store32(esi, 0x3C8, Ptr32(ebx));
  // 0x006EFB02
  CallGameMethod<void, 0x006C8320>(reinterpret_cast<void*>(ecx));
  // 0x006EFB07
  Store32(esi, 0x4F0, Ptr32(ebx));
  // 0x006EFB0D
  eax = static_cast<std::uint8_t*>(self) + 0x4F8;
  // 0x006EFB13
  Store32(esi, 0x4F4, Ptr32(ebx));
  // 0x006EFB19
  Store32(eax, 0x8, 0xFF9198u);
  // 0x006EFB20
  Store32(eax, 0x0, Ptr32(eax));
  // 0x006EFB22
  Store32(eax, 0x4, Ptr32(eax));
  // 0x006EFB2B
  xmm0 = -10.0f;
  // 0x006EFB33
  Store32(esi, 0x504, Ptr32(ebx));
  // 0x006EFB39
  Store32(esi, 0x508, Ptr32(ebx));
  // 0x006EFB3F
  Store32(esi, 0x50C, Ptr32(ebx));
  // 0x006EFB45
  Store32(esi, 0x510, 0xFF9180u);
  // 0x006EFB4F
  Store8(esi, 0x514, 0u);
  // 0x006EFB55
  Store8(esi, 0x515, 0u);
  // 0x006EFB5B
  Store8(esi, 0x520, 0u);
  // 0x006EFB61
  Store32(esi, 0x530, Ptr32(ebx));
  // 0x006EFB67
  Store32(esi, 0x534, Ptr32(ebx));
  // 0x006EFB6D
  Store32(esi, 0x538, Ptr32(ebx));
  // 0x006EFB73
  Store32(esi, 0x53C, Ptr32(ebx));
  // 0x006EFB79
  Store32(esi, 0x540, Ptr32(ebx));
  // 0x006EFB7F
  Store32(esi, 0x544, Ptr32(ebx));
  // 0x006EFB85
  Store32(esi, 0x548, Ptr32(ebx));
  // 0x006EFB8B
  Store32(esi, 0x54C, Ptr32(ebx));
  // 0x006EFB91
  Store32(esi, 0x550, Ptr32(ebx));
  // 0x006EFB97
  StoreFloat(self, 0x554, xmm0);
  // 0x006EFB9F
  Store32(esi, 0x1C4, Ptr32(ebx));
  // 0x006EFBA5
  Store32(esi, 0x1C8, Ptr32(ebx));
  // 0x006EFBAB
  if (LoadGame32(0x11D1868) != 0u) goto L_006F0C6B;
  // 0x006EFBB1
  StoreGame32(0x11D1868, Ptr32(esi));
  // 0x006EFBB7
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006EFBBE
  if ((eax) == nullptr) goto L_006EFBD3;
  // 0x006EFBC7
  ecx = eax;
  // 0x006EFBD1
  CallGameVirtual<void, 6>(reinterpret_cast<void*>(ecx), 0xFF5FDCu, 0xFA0u);
L_006EFBD3:
  // 0x006EFBD3
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006EFBDA
  if ((eax) == nullptr) goto L_006EFBF1;
  // 0x006EFBE3
  ecx = static_cast<std::uint8_t*>(self) + 0x25C;
  // 0x006EFBE9
  a0 = ecx;
  // 0x006EFBEA
  ecx = eax;
  // 0x006EFBEF
  CallGameVirtual<void, 10>(reinterpret_cast<void*>(ecx), a0, 0xFF9160u);
L_006EFBF1:
  // 0x006EFBF1
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006EFBF8
  if ((eax) == nullptr) goto L_006EFC0F;
  // 0x006EFC01
  ecx = static_cast<std::uint8_t*>(self) + 0x263;
  // 0x006EFC07
  a1 = ecx;
  // 0x006EFC08
  ecx = eax;
  // 0x006EFC0D
  CallGameVirtual<void, 10>(reinterpret_cast<void*>(ecx), a1, 0xFF9140u);
L_006EFC0F:
  // 0x006EFC0F
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006EFC16
  if ((eax) == nullptr) goto L_006EFC2D;
  // 0x006EFC1F
  ecx = static_cast<std::uint8_t*>(self) + 0x262;
  // 0x006EFC25
  a2 = ecx;
  // 0x006EFC26
  ecx = eax;
  // 0x006EFC2B
  CallGameVirtual<void, 10>(reinterpret_cast<void*>(ecx), a2, 0xFF9128u);
L_006EFC2D:
  // 0x006EFC2D
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006EFC34
  if ((eax) == nullptr) goto L_006EFC4B;
  // 0x006EFC3D
  ecx = static_cast<std::uint8_t*>(self) + 0x260;
  // 0x006EFC43
  a3 = ecx;
  // 0x006EFC44
  ecx = eax;
  // 0x006EFC49
  CallGameVirtual<void, 10>(reinterpret_cast<void*>(ecx), a3, 0xFF910Cu);
L_006EFC4B:
  // 0x006EFC4B
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006EFC52
  if ((eax) == nullptr) goto L_006EFC69;
  // 0x006EFC5B
  ecx = static_cast<std::uint8_t*>(self) + 0x261;
  // 0x006EFC61
  a4 = ecx;
  // 0x006EFC62
  ecx = eax;
  // 0x006EFC67
  CallGameVirtual<void, 10>(reinterpret_cast<void*>(ecx), a4, 0xFF90D4u);
L_006EFC69:
  // 0x006EFC69
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006EFC70
  if ((eax) == nullptr) goto L_006EFC87;
  // 0x006EFC79
  ecx = static_cast<std::uint8_t*>(self) + 0x265;
  // 0x006EFC7F
  a5 = ecx;
  // 0x006EFC80
  ecx = eax;
  // 0x006EFC85
  CallGameVirtual<void, 10>(reinterpret_cast<void*>(ecx), a5, 0xFF90BCu);
L_006EFC87:
  // 0x006EFC87
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006EFC8E
  if ((eax) == nullptr) goto L_006EFCA5;
  // 0x006EFC97
  ecx = static_cast<std::uint8_t*>(self) + 0x25D;
  // 0x006EFC9D
  a6 = ecx;
  // 0x006EFC9E
  ecx = eax;
  // 0x006EFCA3
  CallGameVirtual<void, 10>(reinterpret_cast<void*>(ecx), a6, 0xFF90A4u);
L_006EFCA5:
  // 0x006EFCA5
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006EFCAC
  if ((eax) == nullptr) goto L_006EFCC3;
  // 0x006EFCB5
  ecx = static_cast<std::uint8_t*>(self) + 0x25E;
  // 0x006EFCBB
  a7 = ecx;
  // 0x006EFCBC
  ecx = eax;
  // 0x006EFCC1
  CallGameVirtual<void, 10>(reinterpret_cast<void*>(ecx), a7, 0xFF907Cu);
L_006EFCC3:
  // 0x006EFCC3
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006EFCCA
  if ((eax) == nullptr) goto L_006EFCE1;
  // 0x006EFCD3
  ecx = static_cast<std::uint8_t*>(self) + 0x25F;
  // 0x006EFCD9
  a8 = ecx;
  // 0x006EFCDA
  ecx = eax;
  // 0x006EFCDF
  CallGameVirtual<void, 10>(reinterpret_cast<void*>(ecx), a8, 0xFF9064u);
L_006EFCE1:
  // 0x006EFCE1
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006EFCE8
  if ((eax) == nullptr) goto L_006EFCFF;
  // 0x006EFCF1
  ecx = static_cast<std::uint8_t*>(self) + 0x267;
  // 0x006EFCF7
  a9 = ecx;
  // 0x006EFCF8
  ecx = eax;
  // 0x006EFCFD
  CallGameVirtual<void, 10>(reinterpret_cast<void*>(ecx), a9, 0xFF9048u);
L_006EFCFF:
  // 0x006EFCFF
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006EFD06
  if ((eax) == nullptr) goto L_006EFD1D;
  // 0x006EFD0F
  ecx = static_cast<std::uint8_t*>(self) + 0x272;
  // 0x006EFD15
  a10 = ecx;
  // 0x006EFD16
  ecx = eax;
  // 0x006EFD1B
  CallGameVirtual<void, 10>(reinterpret_cast<void*>(ecx), a10, 0xFF9018u);
L_006EFD1D:
  // 0x006EFD1D
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006EFD24
  if ((eax) == nullptr) goto L_006EFD3B;
  // 0x006EFD2D
  ecx = static_cast<std::uint8_t*>(self) + 0x26F;
  // 0x006EFD33
  a11 = ecx;
  // 0x006EFD34
  ecx = eax;
  // 0x006EFD39
  CallGameVirtual<void, 10>(reinterpret_cast<void*>(ecx), a11, 0xFF8FFCu);
L_006EFD3B:
  // 0x006EFD3B
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006EFD42
  if ((eax) == nullptr) goto L_006EFD59;
  // 0x006EFD4B
  ecx = static_cast<std::uint8_t*>(self) + 0x271;
  // 0x006EFD51
  a12 = ecx;
  // 0x006EFD52
  ecx = eax;
  // 0x006EFD57
  CallGameVirtual<void, 10>(reinterpret_cast<void*>(ecx), a12, 0xFF8FD8u);
L_006EFD59:
  // 0x006EFD59
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006EFD60
  if ((eax) == nullptr) goto L_006EFD77;
  // 0x006EFD69
  ecx = static_cast<std::uint8_t*>(self) + 0x273;
  // 0x006EFD6F
  a13 = ecx;
  // 0x006EFD70
  ecx = eax;
  // 0x006EFD75
  CallGameVirtual<void, 10>(reinterpret_cast<void*>(ecx), a13, 0xFF8FBCu);
L_006EFD77:
  // 0x006EFD77
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006EFD7E
  if ((eax) == nullptr) goto L_006EFD95;
  // 0x006EFD87
  ecx = static_cast<std::uint8_t*>(self) + 0x274;
  // 0x006EFD8D
  a14 = ecx;
  // 0x006EFD8E
  ecx = eax;
  // 0x006EFD93
  CallGameVirtual<void, 10>(reinterpret_cast<void*>(ecx), a14, 0xFF8F84u);
L_006EFD95:
  // 0x006EFD95
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006EFD9C
  if ((eax) == nullptr) goto L_006EFDB3;
  // 0x006EFDA5
  ecx = static_cast<std::uint8_t*>(self) + 0x26C;
  // 0x006EFDAB
  a15 = ecx;
  // 0x006EFDAC
  ecx = eax;
  // 0x006EFDB1
  CallGameVirtual<void, 10>(reinterpret_cast<void*>(ecx), a15, 0xFF8F5Cu);
L_006EFDB3:
  // 0x006EFDB3
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006EFDBA
  if ((eax) == nullptr) goto L_006EFDD1;
  // 0x006EFDC3
  ecx = static_cast<std::uint8_t*>(self) + 0x276;
  // 0x006EFDC9
  a16 = ecx;
  // 0x006EFDCA
  ecx = eax;
  // 0x006EFDCF
  CallGameVirtual<void, 10>(reinterpret_cast<void*>(ecx), a16, 0xFF8F38u);
L_006EFDD1:
  // 0x006EFDD1
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006EFDD8
  if ((eax) == nullptr) goto L_006EFDEF;
  // 0x006EFDE1
  ecx = static_cast<std::uint8_t*>(self) + 0x277;
  // 0x006EFDE7
  a17 = ecx;
  // 0x006EFDE8
  ecx = eax;
  // 0x006EFDED
  CallGameVirtual<void, 10>(reinterpret_cast<void*>(ecx), a17, 0xFF8F04u);
L_006EFDEF:
  // 0x006EFDEF
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006EFDF6
  if ((eax) == nullptr) goto L_006EFE0D;
  // 0x006EFDFF
  ecx = static_cast<std::uint8_t*>(self) + 0x278;
  // 0x006EFE05
  a18 = ecx;
  // 0x006EFE06
  ecx = eax;
  // 0x006EFE0B
  CallGameVirtual<void, 10>(reinterpret_cast<void*>(ecx), a18, 0xFF8ECCu);
L_006EFE0D:
  // 0x006EFE0D
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006EFE14
  if ((eax) == nullptr) goto L_006EFE4B;
  // 0x006EFE21
  f0 = 0.25f;
  // 0x006EFE25
  ecx = static_cast<std::uint8_t*>(self) + 0x2A8;
  // 0x006EFE31
  f1 = 100.0f;
  // 0x006EFE3B
  f2 = -100.0f;
  // 0x006EFE43
  a19 = ecx;
  // 0x006EFE44
  ecx = eax;
  // 0x006EFE49
  CallGameVirtual<void, 12>(reinterpret_cast<void*>(ecx), a19, 0xFF8E9Cu, f2, f1, f0);
L_006EFE4B:
  // 0x006EFE4B
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006EFE52
  if ((eax) == nullptr) goto L_006EFE89;
  // 0x006EFE5F
  f3 = 10.0f;
  // 0x006EFE63
  ecx = static_cast<std::uint8_t*>(self) + 0x2AC;
  // 0x006EFE6F
  f4 = 2000.0f;
  // 0x006EFE79
  f5 = -1.0f;
  // 0x006EFE81
  a20 = ecx;
  // 0x006EFE82
  ecx = eax;
  // 0x006EFE87
  CallGameVirtual<void, 12>(reinterpret_cast<void*>(ecx), a20, 0xFF8E70u, f5, f4, f3);
L_006EFE89:
  // 0x006EFE89
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006EFE90
  if ((eax) == nullptr) goto L_006EFEC3;
  // 0x006EFE9D
  f6 = 0.001f;
  // 0x006EFEA1
  ecx = static_cast<std::uint8_t*>(self) + 0x2B4;
  // 0x006EFEAD
  f7 = 5.0f;
  // 0x006EFEB3
  f8 = 0.0f;
  // 0x006EFEBB
  a21 = ecx;
  // 0x006EFEBC
  ecx = eax;
  // 0x006EFEC1
  CallGameVirtual<void, 12>(reinterpret_cast<void*>(ecx), a21, 0xFF8E28u, f8, f7, f6);
L_006EFEC3:
  // 0x006EFEC3
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006EFECA
  if ((eax) == nullptr) goto L_006EFEFD;
  // 0x006EFED7
  f9 = 0.001f;
  // 0x006EFEDB
  ecx = static_cast<std::uint8_t*>(self) + 0x2B8;
  // 0x006EFEE7
  f10 = 1.5f;
  // 0x006EFEED
  f11 = 0.0f;
  // 0x006EFEF5
  a22 = ecx;
  // 0x006EFEF6
  ecx = eax;
  // 0x006EFEFB
  CallGameVirtual<void, 12>(reinterpret_cast<void*>(ecx), a22, 0xFF8DECu, f11, f10, f9);
L_006EFEFD:
  // 0x006EFEFD
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006EFF04
  if ((eax) == nullptr) goto L_006EFF37;
  // 0x006EFF11
  f12 = 0.001f;
  // 0x006EFF15
  ecx = static_cast<std::uint8_t*>(self) + 0x2BC;
  // 0x006EFF21
  f13 = 4.0f;
  // 0x006EFF27
  f14 = 0.0f;
  // 0x006EFF2F
  a23 = ecx;
  // 0x006EFF30
  ecx = eax;
  // 0x006EFF35
  CallGameVirtual<void, 12>(reinterpret_cast<void*>(ecx), a23, 0xFF8DB0u, f14, f13, f12);
L_006EFF37:
  // 0x006EFF37
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006EFF3E
  if ((eax) == nullptr) goto L_006EFF4F;
  // 0x006EFF47
  ecx = eax;
  // 0x006EFF4D
  CallGameVirtual<void, 6>(reinterpret_cast<void*>(ecx), 0u, 0x7FFFFFFFu);
L_006EFF4F:
  // 0x006EFF5D
  eax = CallGameFunction<void*, 0x004E3F90>(0x164u, 0xFF8D90u, 0u, 0u, 0u, 0u);
  // 0x006EFF67
  if ((eax) == nullptr) goto L_006EFF74;
  // 0x006EFF69
  Store32(eax, 0x0, 0x16u);
  // 0x006EFF6F
  eax = static_cast<std::uint8_t*>(eax) + 0x4;
  // 0x006EFF72
  goto L_006EFF76;
L_006EFF74:
  // 0x006EFF74
  eax = nullptr;
L_006EFF76:
  // 0x006EFF76
  Store32(esi, 0x2DC, Ptr32(eax));
  // 0x006EFF7C
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006EFF83
  if ((eax) == nullptr) goto L_006EFF98;
  // 0x006EFF8C
  ecx = eax;
  // 0x006EFF96
  CallGameVirtual<void, 6>(reinterpret_cast<void*>(ecx), 0xFF8D5Cu, 0xFA0u);
L_006EFF98:
  // 0x006EFF98
  eax = LoadPtr(esi, 0x2DC);
  // 0x006EFF9E
  edi = static_cast<std::uint8_t*>(self) + 0x2C4;
  // 0x006EFFA4
  l0 = eax;
  // 0x006EFFA8
  Store32(eax, 0x0, 0xFF8D50u);
  // 0x006EFFAE
  Store32(eax, 0x4, 0x8000u);
  // 0x006EFFB5
  Store32(eax, 0x8, Ptr32(edi));
  // 0x006EFFB8
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006EFFBF
  if ((eax) == nullptr) goto L_006EFFEC;
  // 0x006EFFC1
  ecx = l0;
  // 0x006EFFE5
  a24 = ecx;
  // 0x006EFFE8
  ecx = eax;
  // 0x006EFFEA
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a24, 0xFF8D50u, 0x6E95A0u, 0x6E95C0u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006EFFEC:
  // 0x006EFFEC
  eax = LoadPtr(esi, 0x2DC);
  // 0x006EFFF2
  eax = static_cast<std::uint8_t*>(eax) + 0x10;
  // 0x006EFFF5
  l1 = eax;
  // 0x006EFFF9
  Store32(eax, 0x0, 0xFF8D34u);
  // 0x006EFFFF
  Store32(eax, 0x4, 0x10000u);
  // 0x006F0006
  Store32(eax, 0x8, Ptr32(edi));
  // 0x006F0009
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F0010
  if ((eax) == nullptr) goto L_006F003D;
  // 0x006F0012
  ecx = l1;
  // 0x006F0036
  a25 = ecx;
  // 0x006F0039
  ecx = eax;
  // 0x006F003B
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a25, 0xFF8D34u, 0x6E95A0u, 0x6E95C0u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F003D:
  // 0x006F003D
  eax = LoadPtr(esi, 0x2DC);
  // 0x006F0043
  eax = static_cast<std::uint8_t*>(eax) + 0x20;
  // 0x006F0046
  l2 = eax;
  // 0x006F004A
  Store32(eax, 0x0, 0xFF8D1Cu);
  // 0x006F0050
  Store32(eax, 0x4, 0x20000u);
  // 0x006F0057
  Store32(eax, 0x8, Ptr32(edi));
  // 0x006F005A
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F0061
  if ((eax) == nullptr) goto L_006F008E;
  // 0x006F0063
  ecx = l2;
  // 0x006F0087
  a26 = ecx;
  // 0x006F008A
  ecx = eax;
  // 0x006F008C
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a26, 0xFF8D1Cu, 0x6E95A0u, 0x6E95C0u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F008E:
  // 0x006F008E
  eax = LoadPtr(esi, 0x2DC);
  // 0x006F0094
  eax = static_cast<std::uint8_t*>(eax) + 0x30;
  // 0x006F0097
  l3 = eax;
  // 0x006F009B
  Store32(eax, 0x0, 0xFF8D10u);
  // 0x006F00A1
  Store32(eax, 0x4, 0x40000u);
  // 0x006F00A8
  Store32(eax, 0x8, Ptr32(edi));
  // 0x006F00AB
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F00B2
  if ((eax) == nullptr) goto L_006F00DF;
  // 0x006F00B4
  ecx = l3;
  // 0x006F00D8
  a27 = ecx;
  // 0x006F00DB
  ecx = eax;
  // 0x006F00DD
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a27, 0xFF8D10u, 0x6E95A0u, 0x6E95C0u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F00DF:
  // 0x006F00DF
  eax = LoadPtr(esi, 0x2DC);
  // 0x006F00E5
  eax = static_cast<std::uint8_t*>(eax) + 0x40;
  // 0x006F00E8
  l4 = eax;
  // 0x006F00EC
  Store32(eax, 0x0, 0xFF8CF8u);
  // 0x006F00F2
  Store32(eax, 0x4, 0x80000u);
  // 0x006F00F9
  Store32(eax, 0x8, Ptr32(edi));
  // 0x006F00FC
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F0103
  if ((eax) == nullptr) goto L_006F0130;
  // 0x006F0105
  ecx = l4;
  // 0x006F0129
  a28 = ecx;
  // 0x006F012C
  ecx = eax;
  // 0x006F012E
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a28, 0xFF8CF8u, 0x6E95A0u, 0x6E95C0u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F0130:
  // 0x006F0130
  eax = LoadPtr(esi, 0x2DC);
  // 0x006F0136
  eax = static_cast<std::uint8_t*>(eax) + 0x50;
  // 0x006F0139
  l5 = eax;
  // 0x006F013D
  Store32(eax, 0x0, 0xFF8CE0u);
  // 0x006F0143
  Store32(eax, 0x4, 0x100000u);
  // 0x006F014A
  Store32(eax, 0x8, Ptr32(edi));
  // 0x006F014D
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F0154
  if ((eax) == nullptr) goto L_006F0181;
  // 0x006F0156
  ecx = l5;
  // 0x006F017A
  a29 = ecx;
  // 0x006F017D
  ecx = eax;
  // 0x006F017F
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a29, 0xFF8CE0u, 0x6E95A0u, 0x6E95C0u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F0181:
  // 0x006F0181
  eax = LoadPtr(esi, 0x2DC);
  // 0x006F0187
  eax = static_cast<std::uint8_t*>(eax) + 0x60;
  // 0x006F018A
  l6 = eax;
  // 0x006F018E
  Store32(eax, 0x0, 0xFF8CD8u);
  // 0x006F0194
  Store32(eax, 0x4, 0x200000u);
  // 0x006F019B
  Store32(eax, 0x8, Ptr32(edi));
  // 0x006F019E
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F01A5
  if ((eax) == nullptr) goto L_006F01D2;
  // 0x006F01A7
  ecx = l6;
  // 0x006F01CB
  a30 = ecx;
  // 0x006F01CE
  ecx = eax;
  // 0x006F01D0
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a30, 0xFF8CD8u, 0x6E95A0u, 0x6E95C0u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F01D2:
  // 0x006F01D2
  eax = LoadPtr(esi, 0x2DC);
  // 0x006F01D8
  eax = static_cast<std::uint8_t*>(eax) + 0x70;
  // 0x006F01DB
  l7 = eax;
  // 0x006F01DF
  Store32(eax, 0x0, 0xFF8CCCu);
  // 0x006F01E5
  Store32(eax, 0x4, 0x400000u);
  // 0x006F01EC
  Store32(eax, 0x8, Ptr32(edi));
  // 0x006F01EF
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F01F6
  if ((eax) == nullptr) goto L_006F0223;
  // 0x006F01F8
  ecx = l7;
  // 0x006F021C
  a31 = ecx;
  // 0x006F021F
  ecx = eax;
  // 0x006F0221
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a31, 0xFF8CCCu, 0x6E95A0u, 0x6E95C0u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F0223:
  // 0x006F0223
  eax = LoadPtr(esi, 0x2DC);
  // 0x006F0229
  eax = static_cast<std::uint8_t*>(eax) + 0x80;
  // 0x006F022E
  l8 = eax;
  // 0x006F0232
  Store32(eax, 0x0, 0xFF8CC0u);
  // 0x006F0238
  Store32(eax, 0x4, 0x800000u);
  // 0x006F023F
  Store32(eax, 0x8, Ptr32(edi));
  // 0x006F0242
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F0249
  if ((eax) == nullptr) goto L_006F0276;
  // 0x006F024B
  ecx = l8;
  // 0x006F026F
  a32 = ecx;
  // 0x006F0272
  ecx = eax;
  // 0x006F0274
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a32, 0xFF8CC0u, 0x6E95A0u, 0x6E95C0u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F0276:
  // 0x006F0276
  eax = LoadPtr(esi, 0x2DC);
  // 0x006F027C
  eax = static_cast<std::uint8_t*>(eax) + 0x90;
  // 0x006F0281
  l9 = eax;
  // 0x006F0285
  Store32(eax, 0x0, 0xFF8CB0u);
  // 0x006F028B
  Store32(eax, 0x4, 0x1000000u);
  // 0x006F0292
  Store32(eax, 0x8, Ptr32(edi));
  // 0x006F0295
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F029C
  if ((eax) == nullptr) goto L_006F02C9;
  // 0x006F029E
  ecx = l9;
  // 0x006F02C2
  a33 = ecx;
  // 0x006F02C5
  ecx = eax;
  // 0x006F02C7
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a33, 0xFF8CB0u, 0x6E95A0u, 0x6E95C0u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F02C9:
  // 0x006F02C9
  eax = LoadPtr(esi, 0x2DC);
  // 0x006F02CF
  eax = static_cast<std::uint8_t*>(eax) + 0xA0;
  // 0x006F02D4
  l10 = eax;
  // 0x006F02D8
  Store32(eax, 0x0, 0xFF8C94u);
  // 0x006F02DE
  Store32(eax, 0x4, 0x2000000u);
  // 0x006F02E5
  Store32(eax, 0x8, Ptr32(edi));
  // 0x006F02E8
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F02EF
  if ((eax) == nullptr) goto L_006F031C;
  // 0x006F02F1
  ecx = l10;
  // 0x006F0315
  a34 = ecx;
  // 0x006F0318
  ecx = eax;
  // 0x006F031A
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a34, 0xFF8C94u, 0x6E95A0u, 0x6E95C0u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F031C:
  // 0x006F031C
  eax = LoadPtr(esi, 0x2DC);
  // 0x006F0322
  eax = static_cast<std::uint8_t*>(eax) + 0xB0;
  // 0x006F0327
  l11 = eax;
  // 0x006F032B
  Store32(eax, 0x0, 0xFF8C88u);
  // 0x006F0331
  Store32(eax, 0x4, 0x4000000u);
  // 0x006F0338
  Store32(eax, 0x8, Ptr32(edi));
  // 0x006F033B
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F0342
  if ((eax) == nullptr) goto L_006F036F;
  // 0x006F0344
  ecx = l11;
  // 0x006F0368
  a35 = ecx;
  // 0x006F036B
  ecx = eax;
  // 0x006F036D
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a35, 0xFF8C88u, 0x6E95A0u, 0x6E95C0u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F036F:
  // 0x006F036F
  eax = LoadPtr(esi, 0x2DC);
  // 0x006F0375
  eax = static_cast<std::uint8_t*>(eax) + 0xC0;
  // 0x006F037A
  l12 = eax;
  // 0x006F037E
  Store32(eax, 0x0, 0xFF8C74u);
  // 0x006F0384
  Store32(eax, 0x4, 0x8000000u);
  // 0x006F038B
  Store32(eax, 0x8, Ptr32(edi));
  // 0x006F038E
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F0395
  if ((eax) == nullptr) goto L_006F03C2;
  // 0x006F0397
  ecx = l12;
  // 0x006F03BB
  a36 = ecx;
  // 0x006F03BE
  ecx = eax;
  // 0x006F03C0
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a36, 0xFF8C74u, 0x6E95A0u, 0x6E95C0u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F03C2:
  // 0x006F03C2
  eax = LoadPtr(esi, 0x2DC);
  // 0x006F03C8
  eax = static_cast<std::uint8_t*>(eax) + 0xD0;
  // 0x006F03CD
  l13 = eax;
  // 0x006F03D1
  Store32(eax, 0x0, 0xFF8C68u);
  // 0x006F03D7
  Store32(eax, 0x4, 0x10000000u);
  // 0x006F03DE
  Store32(eax, 0x8, Ptr32(edi));
  // 0x006F03E1
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F03E8
  if ((eax) == nullptr) goto L_006F0415;
  // 0x006F03EA
  ecx = l13;
  // 0x006F040E
  a37 = ecx;
  // 0x006F0411
  ecx = eax;
  // 0x006F0413
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a37, 0xFF8C68u, 0x6E95A0u, 0x6E95C0u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F0415:
  // 0x006F0415
  eax = LoadPtr(esi, 0x2DC);
  // 0x006F041B
  eax = static_cast<std::uint8_t*>(eax) + 0xE0;
  // 0x006F0420
  l14 = eax;
  // 0x006F0424
  Store32(eax, 0x0, 0xFF8C58u);
  // 0x006F042A
  Store32(eax, 0x4, 0x20000000u);
  // 0x006F0431
  Store32(eax, 0x8, Ptr32(edi));
  // 0x006F0434
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F043B
  if ((eax) == nullptr) goto L_006F0468;
  // 0x006F043D
  ecx = l14;
  // 0x006F0461
  a38 = ecx;
  // 0x006F0464
  ecx = eax;
  // 0x006F0466
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a38, 0xFF8C58u, 0x6E95A0u, 0x6E95C0u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F0468:
  // 0x006F0468
  eax = LoadPtr(esi, 0x2DC);
  // 0x006F046E
  eax = static_cast<std::uint8_t*>(eax) + 0xF0;
  // 0x006F0473
  l15 = eax;
  // 0x006F0477
  Store32(eax, 0x0, 0xFF8C4Cu);
  // 0x006F047D
  Store32(eax, 0x4, 0x40000000u);
  // 0x006F0484
  Store32(eax, 0x8, Ptr32(edi));
  // 0x006F0487
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F048E
  if ((eax) == nullptr) goto L_006F04BB;
  // 0x006F0490
  ecx = l15;
  // 0x006F04B4
  a39 = ecx;
  // 0x006F04B7
  ecx = eax;
  // 0x006F04B9
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a39, 0xFF8C4Cu, 0x6E95A0u, 0x6E95C0u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F04BB:
  // 0x006F04BB
  eax = LoadPtr(esi, 0x2DC);
  // 0x006F04C1
  eax = static_cast<std::uint8_t*>(eax) + 0x100;
  // 0x006F04C6
  l16 = eax;
  // 0x006F04CA
  Store32(eax, 0x0, 0xFF8C34u);
  // 0x006F04D0
  Store32(eax, 0x4, 0x4000u);
  // 0x006F04D7
  Store32(eax, 0x8, Ptr32(edi));
  // 0x006F04DA
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F04E1
  if ((eax) == nullptr) goto L_006F050E;
  // 0x006F04E3
  ecx = l16;
  // 0x006F0507
  a40 = ecx;
  // 0x006F050A
  ecx = eax;
  // 0x006F050C
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a40, 0xFF8C34u, 0x6E95A0u, 0x6E95C0u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F050E:
  // 0x006F050E
  eax = LoadPtr(esi, 0x2DC);
  // 0x006F0514
  eax = static_cast<std::uint8_t*>(eax) + 0x110;
  // 0x006F0519
  l17 = eax;
  // 0x006F051D
  Store32(eax, 0x0, 0xFF8C1Cu);
  // 0x006F0523
  Store32(eax, 0x4, 0x2000u);
  // 0x006F052A
  Store32(eax, 0x8, Ptr32(edi));
  // 0x006F052D
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F0534
  if ((eax) == nullptr) goto L_006F0561;
  // 0x006F0536
  ecx = l17;
  // 0x006F055A
  a41 = ecx;
  // 0x006F055D
  ecx = eax;
  // 0x006F055F
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a41, 0xFF8C1Cu, 0x6E95A0u, 0x6E95C0u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F0561:
  // 0x006F0561
  eax = LoadPtr(esi, 0x2DC);
  // 0x006F0567
  eax = static_cast<std::uint8_t*>(eax) + 0x120;
  // 0x006F056C
  l18 = eax;
  // 0x006F0570
  Store32(eax, 0x0, 0xFF8C14u);
  // 0x006F0576
  Store32(eax, 0x4, 0x7FFFF000u);
  // 0x006F057D
  Store32(eax, 0x8, Ptr32(edi));
  // 0x006F0580
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F0587
  if ((eax) == nullptr) goto L_006F05B4;
  // 0x006F0589
  ecx = l18;
  // 0x006F05AD
  a42 = ecx;
  // 0x006F05B0
  ecx = eax;
  // 0x006F05B2
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a42, 0xFF8C14u, 0x6E95A0u, 0x6E95C0u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F05B4:
  // 0x006F05B4
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F05BB
  if ((eax) == nullptr) goto L_006F05CC;
  // 0x006F05C4
  ecx = eax;
  // 0x006F05CA
  CallGameVirtual<void, 6>(reinterpret_cast<void*>(ecx), 0u, 0x7FFFFFFFu);
L_006F05CC:
  // 0x006F05CC
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F05D3
  if ((eax) == nullptr) goto L_006F05E8;
  // 0x006F05DC
  ecx = eax;
  // 0x006F05E6
  CallGameVirtual<void, 6>(reinterpret_cast<void*>(ecx), 0xFF8BD4u, 0xFA0u);
L_006F05E8:
  // 0x006F05E8
  eax = LoadPtr(esi, 0x2DC);
  // 0x006F05EE
  eax = static_cast<std::uint8_t*>(eax) + 0x130;
  // 0x006F05F3
  l19 = eax;
  // 0x006F05F7
  Store32(eax, 0x0, 0xFF8BC4u);
  // 0x006F05FD
  Store32(eax, 0x4, 0x1u);
  // 0x006F0604
  Store32(eax, 0x8, Ptr32(edi));
  // 0x006F0607
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F060E
  if ((eax) == nullptr) goto L_006F063B;
  // 0x006F0610
  ecx = l19;
  // 0x006F0634
  a43 = ecx;
  // 0x006F0637
  ecx = eax;
  // 0x006F0639
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a43, 0xFF8BC4u, 0x6E95A0u, 0x6E95C0u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F063B:
  // 0x006F063B
  eax = LoadPtr(esi, 0x2DC);
  // 0x006F0641
  eax = static_cast<std::uint8_t*>(eax) + 0x140;
  // 0x006F0646
  l20 = eax;
  // 0x006F064A
  Store32(eax, 0x0, 0xFF8BA8u);
  // 0x006F0650
  Store32(eax, 0x4, 0x10u);
  // 0x006F0657
  Store32(eax, 0x8, Ptr32(edi));
  // 0x006F065A
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F0661
  if ((eax) == nullptr) goto L_006F068E;
  // 0x006F0663
  ecx = l20;
  // 0x006F0687
  a44 = ecx;
  // 0x006F068A
  ecx = eax;
  // 0x006F068C
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a44, 0xFF8BA8u, 0x6E95A0u, 0x6E95C0u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F068E:
  // 0x006F068E
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F0695
  if ((eax) == nullptr) goto L_006F06A6;
  // 0x006F069E
  ecx = eax;
  // 0x006F06A4
  CallGameVirtual<void, 6>(reinterpret_cast<void*>(ecx), 0u, 0x7FFFFFFFu);
L_006F06A6:
  // 0x006F06A6
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F06AD
  if ((eax) == nullptr) goto L_006F06C2;
  // 0x006F06B6
  ecx = eax;
  // 0x006F06C0
  CallGameVirtual<void, 6>(reinterpret_cast<void*>(ecx), 0xFF8B88u, 0x36B0u);
L_006F06C2:
  // 0x006F06C2
  eax = LoadPtr(esi, 0x2DC);
  // 0x006F06C8
  eax = static_cast<std::uint8_t*>(eax) + 0x150;
  // 0x006F06CD
  l21 = eax;
  // 0x006F06D1
  Store32(eax, 0x0, 0xFF8B58u);
  // 0x006F06D7
  Store32(eax, 0x4, 0x7DFFF000u);
  // 0x006F06DE
  Store32(eax, 0x8, Ptr32(edi));
  // 0x006F06E1
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F06E8
  if ((eax) == nullptr) goto L_006F0715;
  // 0x006F06EA
  ecx = l21;
  // 0x006F070E
  a45 = ecx;
  // 0x006F0711
  ecx = eax;
  // 0x006F0713
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a45, 0xFF8B58u, 0x6E95A0u, 0x6E95C0u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F0715:
  // 0x006F0715
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F071C
  if ((eax) == nullptr) goto L_006F072D;
  // 0x006F0725
  ecx = eax;
  // 0x006F072B
  CallGameVirtual<void, 6>(reinterpret_cast<void*>(ecx), 0u, 0x7FFFFFFFu);
L_006F072D:
  // 0x006F073B
  eax = CallGameFunction<void*, 0x004E3F90>(0x104u, 0xFF8B34u, 0u, 0u, 0u, 0u);
  // 0x006F0745
  if ((eax) == nullptr) goto L_006F0752;
  // 0x006F0747
  Store32(eax, 0x0, 0x10u);
  // 0x006F074D
  eax = static_cast<std::uint8_t*>(eax) + 0x4;
  // 0x006F0750
  goto L_006F0754;
L_006F0752:
  // 0x006F0752
  eax = nullptr;
L_006F0754:
  // 0x006F0754
  Store32(esi, 0x2E0, Ptr32(eax));
  // 0x006F075A
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F0761
  if ((eax) == nullptr) goto L_006F0776;
  // 0x006F076A
  ecx = eax;
  // 0x006F0774
  CallGameVirtual<void, 6>(reinterpret_cast<void*>(ecx), 0xFF8B00u, 0xFA0u);
L_006F0776:
  // 0x006F0776
  edi = LoadPtr(esi, 0x2E0);
  // 0x006F077C
  Store32(edi, 0x0, 0xFF8AF8u);
  // 0x006F0782
  Store32(edi, 0x4, Ptr32(ebx));
  // 0x006F0785
  Store32(edi, 0x8, Ptr32(esi));
  // 0x006F0788
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F078F
  if ((eax) == nullptr) goto L_006F07B8;
  // 0x006F07AE
  a46 = edi;
  // 0x006F07AF
  ecx = eax;
  // 0x006F07B6
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a46, 0xFF8AF8u, 0x6E95E0u, 0x6EB420u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F07B8:
  // 0x006F07B8
  edi = LoadPtr(esi, 0x2E0);
  // 0x006F07BE
  edi = static_cast<std::uint8_t*>(edi) + 0x10;
  // 0x006F07C1
  Store32(edi, 0x0, 0xFF8AD0u);
  // 0x006F07C7
  Store32(edi, 0x4, 0x1u);
  // 0x006F07CE
  Store32(edi, 0x8, Ptr32(esi));
  // 0x006F07D1
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F07D8
  if ((eax) == nullptr) goto L_006F0801;
  // 0x006F07F7
  a47 = edi;
  // 0x006F07F8
  ecx = eax;
  // 0x006F07FF
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a47, 0xFF8AD0u, 0x6E95E0u, 0x6EB420u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F0801:
  // 0x006F0801
  edi = LoadPtr(esi, 0x2E0);
  // 0x006F0807
  edi = static_cast<std::uint8_t*>(edi) + 0x20;
  // 0x006F080A
  Store32(edi, 0x0, 0xFF8AACu);
  // 0x006F0810
  Store32(edi, 0x4, 0x2u);
  // 0x006F0817
  Store32(edi, 0x8, Ptr32(esi));
  // 0x006F081A
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F0821
  if ((eax) == nullptr) goto L_006F084A;
  // 0x006F0840
  a48 = edi;
  // 0x006F0841
  ecx = eax;
  // 0x006F0848
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a48, 0xFF8AACu, 0x6E95E0u, 0x6EB420u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F084A:
  // 0x006F084A
  edi = LoadPtr(esi, 0x2E0);
  // 0x006F0850
  edi = static_cast<std::uint8_t*>(edi) + 0x30;
  // 0x006F0853
  Store32(edi, 0x0, 0xFF8A84u);
  // 0x006F0859
  Store32(edi, 0x4, 0x3u);
  // 0x006F0860
  Store32(edi, 0x8, Ptr32(esi));
  // 0x006F0863
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F086A
  if ((eax) == nullptr) goto L_006F0893;
  // 0x006F0889
  a49 = edi;
  // 0x006F088A
  ecx = eax;
  // 0x006F0891
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a49, 0xFF8A84u, 0x6E95E0u, 0x6EB420u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F0893:
  // 0x006F0893
  edi = LoadPtr(esi, 0x2E0);
  // 0x006F0899
  edi = static_cast<std::uint8_t*>(edi) + 0x40;
  // 0x006F089C
  Store32(edi, 0x0, 0xFF8A68u);
  // 0x006F08A2
  Store32(edi, 0x4, 0x4u);
  // 0x006F08A9
  Store32(edi, 0x8, Ptr32(esi));
  // 0x006F08AC
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F08B3
  if ((eax) == nullptr) goto L_006F08DC;
  // 0x006F08D2
  a50 = edi;
  // 0x006F08D3
  ecx = eax;
  // 0x006F08DA
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a50, 0xFF8A68u, 0x6E95E0u, 0x6EB420u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F08DC:
  // 0x006F08DC
  edi = LoadPtr(esi, 0x2E0);
  // 0x006F08E2
  edi = static_cast<std::uint8_t*>(edi) + 0x50;
  // 0x006F08E5
  Store32(edi, 0x0, 0xFF8A44u);
  // 0x006F08EB
  Store32(edi, 0x4, 0x5u);
  // 0x006F08F2
  Store32(edi, 0x8, Ptr32(esi));
  // 0x006F08F5
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F08FC
  if ((eax) == nullptr) goto L_006F0925;
  // 0x006F091B
  a51 = edi;
  // 0x006F091C
  ecx = eax;
  // 0x006F0923
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a51, 0xFF8A44u, 0x6E95E0u, 0x6EB420u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F0925:
  // 0x006F0925
  edi = LoadPtr(esi, 0x2E0);
  // 0x006F092B
  edi = static_cast<std::uint8_t*>(edi) + 0x60;
  // 0x006F092E
  Store32(edi, 0x0, 0xFF8A2Cu);
  // 0x006F0934
  Store32(edi, 0x4, 0x6u);
  // 0x006F093B
  Store32(edi, 0x8, Ptr32(esi));
  // 0x006F093E
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F0945
  if ((eax) == nullptr) goto L_006F096E;
  // 0x006F0964
  a52 = edi;
  // 0x006F0965
  ecx = eax;
  // 0x006F096C
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a52, 0xFF8A2Cu, 0x6E95E0u, 0x6EB420u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F096E:
  // 0x006F096E
  edi = LoadPtr(esi, 0x2E0);
  // 0x006F0974
  edi = static_cast<std::uint8_t*>(edi) + 0x70;
  // 0x006F0977
  Store32(edi, 0x0, 0xFF8A10u);
  // 0x006F097D
  Store32(edi, 0x4, 0x7u);
  // 0x006F0984
  Store32(edi, 0x8, Ptr32(esi));
  // 0x006F0987
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F098E
  if ((eax) == nullptr) goto L_006F09B7;
  // 0x006F09AD
  a53 = edi;
  // 0x006F09AE
  ecx = eax;
  // 0x006F09B5
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a53, 0xFF8A10u, 0x6E95E0u, 0x6EB420u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F09B7:
  // 0x006F09B7
  edi = LoadPtr(esi, 0x2E0);
  // 0x006F09BD
  edi = static_cast<std::uint8_t*>(edi) + 0x80;
  // 0x006F09C3
  Store32(edi, 0x0, 0xFF89E8u);
  // 0x006F09C9
  Store32(edi, 0x4, 0x8u);
  // 0x006F09D0
  Store32(edi, 0x8, Ptr32(esi));
  // 0x006F09D3
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F09DA
  if ((eax) == nullptr) goto L_006F0A03;
  // 0x006F09F9
  a54 = edi;
  // 0x006F09FA
  ecx = eax;
  // 0x006F0A01
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a54, 0xFF89E8u, 0x6E95E0u, 0x6EB420u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F0A03:
  // 0x006F0A03
  edi = LoadPtr(esi, 0x2E0);
  // 0x006F0A09
  edi = static_cast<std::uint8_t*>(edi) + 0x90;
  // 0x006F0A0F
  Store32(edi, 0x0, 0xFF89BCu);
  // 0x006F0A15
  Store32(edi, 0x4, 0x9u);
  // 0x006F0A1C
  Store32(edi, 0x8, Ptr32(esi));
  // 0x006F0A1F
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F0A26
  if ((eax) == nullptr) goto L_006F0A4F;
  // 0x006F0A45
  a55 = edi;
  // 0x006F0A46
  ecx = eax;
  // 0x006F0A4D
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a55, 0xFF89BCu, 0x6E95E0u, 0x6EB420u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F0A4F:
  // 0x006F0A4F
  edi = LoadPtr(esi, 0x2E0);
  // 0x006F0A55
  edi = static_cast<std::uint8_t*>(edi) + 0xA0;
  // 0x006F0A5B
  Store32(edi, 0x0, 0xFF89A4u);
  // 0x006F0A61
  Store32(edi, 0x4, 0xAu);
  // 0x006F0A68
  Store32(edi, 0x8, Ptr32(esi));
  // 0x006F0A6B
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F0A72
  if ((eax) == nullptr) goto L_006F0A9B;
  // 0x006F0A91
  a56 = edi;
  // 0x006F0A92
  ecx = eax;
  // 0x006F0A99
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a56, 0xFF89A4u, 0x6E95E0u, 0x6EB420u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F0A9B:
  // 0x006F0A9B
  edi = LoadPtr(esi, 0x2E0);
  // 0x006F0AA1
  edi = static_cast<std::uint8_t*>(edi) + 0xB0;
  // 0x006F0AA7
  Store32(edi, 0x0, 0xFF898Cu);
  // 0x006F0AAD
  Store32(edi, 0x4, 0xBu);
  // 0x006F0AB4
  Store32(edi, 0x8, Ptr32(esi));
  // 0x006F0AB7
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F0ABE
  if ((eax) == nullptr) goto L_006F0AE7;
  // 0x006F0ADD
  a57 = edi;
  // 0x006F0ADE
  ecx = eax;
  // 0x006F0AE5
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a57, 0xFF898Cu, 0x6E95E0u, 0x6EB420u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F0AE7:
  // 0x006F0AE7
  edi = LoadPtr(esi, 0x2E0);
  // 0x006F0AED
  edi = static_cast<std::uint8_t*>(edi) + 0xC0;
  // 0x006F0AF3
  Store32(edi, 0x0, 0xFF8974u);
  // 0x006F0AF9
  Store32(edi, 0x4, 0xCu);
  // 0x006F0B00
  Store32(edi, 0x8, Ptr32(esi));
  // 0x006F0B03
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F0B0A
  if ((eax) == nullptr) goto L_006F0B33;
  // 0x006F0B29
  a58 = edi;
  // 0x006F0B2A
  ecx = eax;
  // 0x006F0B31
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a58, 0xFF8974u, 0x6E95E0u, 0x6EB420u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F0B33:
  // 0x006F0B33
  edi = LoadPtr(esi, 0x2E0);
  // 0x006F0B39
  edi = static_cast<std::uint8_t*>(edi) + 0xD0;
  // 0x006F0B3F
  Store32(edi, 0x0, 0xFF8954u);
  // 0x006F0B45
  Store32(edi, 0x4, 0xDu);
  // 0x006F0B4C
  Store32(edi, 0x8, Ptr32(esi));
  // 0x006F0B4F
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F0B56
  if ((eax) == nullptr) goto L_006F0B7F;
  // 0x006F0B75
  a59 = edi;
  // 0x006F0B76
  ecx = eax;
  // 0x006F0B7D
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a59, 0xFF8954u, 0x6E95E0u, 0x6EB420u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F0B7F:
  // 0x006F0B7F
  edi = LoadPtr(esi, 0x2E0);
  // 0x006F0B85
  edi = static_cast<std::uint8_t*>(edi) + 0xE0;
  // 0x006F0B8B
  Store32(edi, 0x0, 0xFF8934u);
  // 0x006F0B91
  Store32(edi, 0x4, 0xEu);
  // 0x006F0B98
  Store32(edi, 0x8, Ptr32(esi));
  // 0x006F0B9B
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F0BA2
  if ((eax) == nullptr) goto L_006F0BCB;
  // 0x006F0BC1
  a60 = edi;
  // 0x006F0BC2
  ecx = eax;
  // 0x006F0BC9
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a60, 0xFF8934u, 0x6E95E0u, 0x6EB420u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F0BCB:
  // 0x006F0BCB
  edi = LoadPtr(esi, 0x2E0);
  // 0x006F0BD1
  edi = static_cast<std::uint8_t*>(edi) + 0xF0;
  // 0x006F0BD7
  Store32(edi, 0x0, 0xFF8910u);
  // 0x006F0BDD
  Store32(edi, 0x4, 0xFu);
  // 0x006F0BE4
  Store32(edi, 0x8, Ptr32(esi));
  // 0x006F0BE7
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F0BEE
  if ((eax) == nullptr) goto L_006F0C17;
  // 0x006F0C0D
  a61 = edi;
  // 0x006F0C0E
  ecx = eax;
  // 0x006F0C15
  CallGameVirtual<void, 16>(reinterpret_cast<void*>(ecx), 0x4u, a61, 0xFF8910u, 0x6E95E0u, 0x6EB420u, 0xFFFE7960u, 0x186A0u, 0x1u);
L_006F0C17:
  // 0x006F0C17
  eax = CallGameFunction<void*, 0x005A00A0>();
  // 0x006F0C1E
  if ((eax) == nullptr) goto L_006F0C2F;
  // 0x006F0C27
  ecx = eax;
  // 0x006F0C2D
  CallGameVirtual<void, 6>(reinterpret_cast<void*>(ecx), 0u, 0x7FFFFFFFu);
L_006F0C2F:
  // 0x006F0C3A
  eax = CallGameFunction<void*, 0x004E3F90>(0x1u, 0xFF88F8u, 0u, 0u, 0u, 0u);
  // 0x006F0C44
  if ((eax) == nullptr) goto L_006F0C4F;
  // 0x006F0C46
  ecx = eax;
  // 0x006F0C48
  eax = CallGameMethod<void*, 0x006946A0>(reinterpret_cast<void*>(ecx));
  // 0x006F0C4D
  goto L_006F0C51;
L_006F0C4F:
  // 0x006F0C4F
  eax = nullptr;
L_006F0C51:
  // 0x006F0C51
  Store32(esi, 0x4F0, Ptr32(eax));
  // 0x006F0C57
  eax = static_cast<std::uint8_t*>(self) + 0x1A0;
  // 0x006F0C5D
  a62 = eax;
  // 0x006F0C5E
  CallGameFunction<void, 0x007235B0>(a62);
  // 0x006F0C66
  CallGameFunction<void, 0x006E89F0>();
L_006F0C6B:
  // 0x006F0C6B
  ecx = static_cast<std::uint8_t*>(self) + 0x3CC;
  // 0x006F0C71
  CallGameMethod<void, 0x00706610>(reinterpret_cast<void*>(ecx));
  // 0x006F0C81
  eax = CallGameFunction<void*, 0x004E3F90>(0x40u, 0xFF88D4u, 0u, 0u, 0u, 0u);
  // 0x006F0C8B
  if ((eax) == nullptr) goto L_006F0C96;
  // 0x006F0C8D
  ecx = eax;
  // 0x006F0C8F
  eax = CallGameMethod<void*, 0x00725C10>(reinterpret_cast<void*>(ecx));
  // 0x006F0C94
  goto L_006F0C98;
L_006F0C96:
  // 0x006F0C96
  eax = nullptr;
L_006F0C98:
  // 0x006F0C99
  Store32(esi, 0x4F4, Ptr32(eax));
  // 0x006F0C9F
  ecx = LoadGamePtr(0x114FAB0);
  // 0x006F0CAA
  a63 = ecx;
  // 0x006F0CAB
  eax = reinterpret_cast<void*>(static_cast<std::uintptr_t>(CallGameFunction<std::uint32_t, 0x0058AAD0>(a63, 0xFF88B0u, 0u)));
  // 0x006F0CB5
  if ((Ptr32(eax) & 0xFFu) == 0u) goto L_006F0CC1;
  // 0x006F0CB7
  xmm0 = 2.0f;
  // 0x006F0CBF
  goto L_006F0CC9;
L_006F0CC1:
  // 0x006F0CC1
  xmm0 = 1.0f;
L_006F0CC9:
  // 0x006F0CCA
  StoreFloat(self, 0x1B8, xmm0);
  // 0x006F0CD2
  edx = LoadGamePtr(0x114FAB0);
  // 0x006F0CDD
  a64 = edx;
  // 0x006F0CDE
  eax = reinterpret_cast<void*>(static_cast<std::uintptr_t>(CallGameFunction<std::uint32_t, 0x0058AAD0>(a64, 0xFF8894u, 0u)));
  // 0x006F0CE8
  if ((Ptr32(eax) & 0xFFu) == 0u) goto L_006F0CF4;
  // 0x006F0CEA
  xmm0 = 3.0f;
  // 0x006F0CF2
  goto L_006F0CFC;
L_006F0CF4:
  // 0x006F0CF4
  xmm0 = 2.0f;
L_006F0CFC:
  // 0x006F0CFD
  StoreFloat(self, 0x1BC, xmm0);
  // 0x006F0D05
  eax = LoadGamePtr(0x114FAB0);
  // 0x006F0D0F
  a65 = eax;
  // 0x006F0D10
  eax = reinterpret_cast<void*>(static_cast<std::uintptr_t>(CallGameFunction<std::uint32_t, 0x0058AAD0>(a65, 0xFF885Cu, 0u)));
  // 0x006F0D1D
  ecx = static_cast<std::uint8_t*>(self) + 0x328;
  // 0x006F0D23
  Store8(esi, 0x27B, static_cast<std::uint8_t>(Ptr32(eax)));
  // 0x006F0D29
  CallGameMethod<void, 0x009DFF30>(reinterpret_cast<void*>(ecx), 0xF4240u);
  // 0x006F0D33
  ecx = static_cast<std::uint8_t*>(self) + 0x4D4;
  // 0x006F0D39
  CallGameMethod<void, 0x009DFF30>(reinterpret_cast<void*>(ecx), 0xF4240u);
  // 0x006F0D44
  if (Load32(self, 0x300) == 0u) goto L_006F0D9C;
  // 0x006F0D4C
  if (Load32(self, 0x534) == 0u) goto L_006F0D9C;
  // 006F0D4E  xorps xmm0, xmm0
  xmm0 = 0.0f;
  // 0x006F0D51
  Store32(esi, 0x534, Ptr32(ebx));
  // 0x006F0D5E
  eax = esi;
  // 0x006F0D60
  if (!(!(xmm0 > LoadFloat(self, 0x2AC)))) goto L_006F0D7F;
  // 0x006F0D62
  ecx = LoadPtr(esi, 0x53C);
  // 0x006F0D68
  Store32(esi, 0x538, Ptr32(ecx));
  // 0x006F0D6E
  Store32(esi, 0x2CC, 0x5192000u);
  // 0x006F0D7E
  return self;
L_006F0D7F:
  // 0x006F0D7F
  edx = LoadPtr(esi, 0x540);
  // 0x006F0D85
  Store32(esi, 0x538, Ptr32(edx));
  // 0x006F0D8B
  Store32(esi, 0x2CC, 0x5192000u);
  // 0x006F0D9B
  return self;
L_006F0D9C:
  // 0x006F0D9D
  eax = esi;
  // 0x006F0D9F
  Store32(esi, 0x2CC, 0x5192000u);
  // 0x006F0DAE
  return self;
#else
  (void)self_;
  return self_;
#endif
}

}  // namespace ts3::renderer
