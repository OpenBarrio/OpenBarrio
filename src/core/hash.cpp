#include "ts3/hash.h"

namespace ts3::hash {

namespace {

// tolower/towlower in the CRT's "C" locale: only ASCII A-Z change. TODO(harness): the
// game imports setlocale; confirm in-game that the locale stays "C" for these calls.
constexpr int32_t CLocaleLower(int32_t c)
{
    return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
}

}  // namespace

uint32_t Fnv32Lower(const char* s, uint32_t len)
{
    uint32_t h = kFnv32Basis;
    for (uint32_t i = 0; i < len; ++i) {
        const int32_t c = CLocaleLower(static_cast<signed char>(s[i]));
        h = h * kFnv32Prime ^ static_cast<uint32_t>(c);
    }
    return h;
}

uint64_t Fnv64LowerW(const char16_t* s, uint32_t len)
{
    uint64_t h = kFnv64Basis;
    for (uint32_t i = 0; i < len; ++i) {
        const auto c = static_cast<uint16_t>(CLocaleLower(s[i]));
        h = h * kFnv64Prime ^ c;
    }
    return h;
}

}  // namespace ts3::hash
