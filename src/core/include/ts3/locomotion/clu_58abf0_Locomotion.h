// Cluster unit clu_58abf0_Locomotion (subsystem Locomotion): one 58-byte thiscall helper
// that reads a double out of the game's config registry - two key strings in, a fallback
// float. The registry object comes from the 0x0058B880 getter (global 0x011CDD00) and the
// two-level lookup is 0x0058C380 (thiscall, 3 stack args, ret 0xC); both are not rewritten
// yet, so they run through the ts3 layout.h bridge. On a hit the value string is parsed
// with the game's own wcstod (the IAT slot 0x00F954DC, MSVCR80.dll!wcstod) - NOT the host
// CRT, because MSVCR80 parses "inf"/"nan"/"0x10" differently (it returns 0 for them). On
// any miss the fallback float is returned, widened to double exactly as the original's
// the original instruction sequence does.
//
//   0x0058ABF0  ConfigDouble  thiscall: (this unused, key1, key2, fallback) -> double
//
// The original never dereferences `this` (push ecx / pop ecx only), so the rewrite is a
// member of a stateless holder class - MSVC only accepts __thiscall on member functions,
// and a member function has the identical ABI (this in ecx, three stack args, ret 0).
//
// EA quirks kept: `this` is saved and restored but unused; the output slot is never
// initialized before the lookup (it holds `this` until the lookup overwrites it) and is
// only read when the lookup succeeds; the lookup result is tested with the original instruction sequence
// (low byte only - upper EAX bits are undefined and must be ignored).
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::locomotion {

// Stateless holder for the 0x0058ABF0 rewrite; `this` is never read (the original
// pushes and pops ecx without using it).
class ConfigDoubleLookup {
public:
    // TS3W.exe 0x0058ABF0: config-registry double lookup. Returns wcstod of the
    // looked-up value string on a hit, else the fallback float widened to double.
    double ConfigDouble(const wchar_t* key1, const wchar_t* key2, float fallback);
};

}  // namespace ts3::locomotion
