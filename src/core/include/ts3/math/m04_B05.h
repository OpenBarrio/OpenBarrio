#pragma once
#include <cstdint>
namespace ts3::math::b05 {
// TS3W.exe 0x00583EB0: Multiply selected lanes by signs; preserves original path-dependent pointer return.
uintptr_t MirrorSigns(const void* source, void* output, char alternate);
// TS3W.exe 0x00583F50: Multiply selected lanes by signs; preserves original path-dependent pointer return.
uintptr_t MirrorSignsShuffled(const void* source, void* output, char alternate);
// TS3W.exe 0x005842A0: Decode signed packed lanes; preserves untouched lanes, signed zeros and path-dependent EAX.
uintptr_t Decode10(const void* source, void* output, uint16_t* flags);
// TS3W.exe 0x00584480: Decode signed packed lanes; preserves untouched lanes, signed zeros and path-dependent EAX.
uintptr_t DecodeTable1(const void* source, void* output, uint16_t* flags, const float* const* table);
// TS3W.exe 0x00584550: Decode signed packed lanes; preserves untouched lanes, signed zeros and path-dependent EAX.
uintptr_t DecodeTable2(const void* source, void* output, uint16_t* flags, const float* const* table);
// TS3W.exe 0x005846A0: Decode signed packed lanes; preserves untouched lanes, signed zeros and path-dependent EAX.
uintptr_t DecodeTable3(const void* source, void* output, uint16_t* flags, const float* const* table);
// TS3W.exe 0x00584800: Decode signed packed lanes; preserves untouched lanes, signed zeros and path-dependent EAX.
uintptr_t DecodeTable4(const void* source, void* output, uint16_t* flags, const float* const* table);
// TS3W.exe 0x00584910: Decode signed packed lanes; preserves untouched lanes, signed zeros and path-dependent EAX.
uintptr_t DecodeByte1(const void* source, void* output, uint16_t* flags);
// TS3W.exe 0x005849F0: Decode signed packed lanes; preserves untouched lanes, signed zeros and path-dependent EAX.
uintptr_t DecodeByte2(const void* source, void* output, uint16_t* flags);
// TS3W.exe 0x00584B30: Decode signed packed lanes; preserves untouched lanes, signed zeros and path-dependent EAX.
uintptr_t DecodeByte3(const void* source, void* output, uint16_t* flags);
// TS3W.exe 0x00584C30: Decode signed packed lanes; preserves untouched lanes, signed zeros and path-dependent EAX.
uintptr_t DecodeByte4(const void* source, void* output, uint16_t* flags);
// TS3W.exe 0x00584D50: Decode signed packed lanes; preserves untouched lanes, signed zeros and path-dependent EAX.
uintptr_t DecodeWord4(const void* source, void* output, uint16_t* flags);
// TS3W.exe 0x007CF860: Pack XYZ into BGR bytes using SSE truncation and low-byte wrapping, with alpha zero.
uintptr_t PackNormal(const void* source, void* output);
}
