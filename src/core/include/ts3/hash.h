// FNV-1 hashes used for resource keys and lookup tables, bug-for-bug compatible with
// TS3W.exe 1.67.2. Portable: builds unchanged for x86 and x64.
#pragma once

#include <cstddef>
#include <cstdint>

namespace ts3::hash {

inline constexpr uint32_t kFnv32Basis = 0x811C9DC5u;
inline constexpr uint32_t kFnv32Prime = 0x01000193u;
inline constexpr uint64_t kFnv64Basis = 0xCBF29CE484222325ull;
inline constexpr uint64_t kFnv64Prime = 0x00000100000001B3ull;

// TS3W.exe 0x007CFC50 (__cdecl). FNV-1 over `len` chars lowercased with the CRT's
// tolower in the "C" locale. Chars are sign-extended before tolower and the XOR, so a
// byte >= 0x80 contributes 0xFFFFFFxx (that is what the original computes).
uint32_t Fnv32Lower(const char* s, uint32_t len);

// TS3W.exe 0x004AEB90 (__cdecl). FNV-1 64-bit over `len` UTF-16 units lowercased with
// towlower in the "C" locale (only A-Z change).
uint64_t Fnv64LowerW(const char16_t* s, uint32_t len);

}  // namespace ts3::hash
