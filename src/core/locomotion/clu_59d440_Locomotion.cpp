#include <cstring>

#include "ts3/locomotion/clu_59d440_Locomotion.h"

namespace ts3::locomotion {
namespace {
struct Entry { int32_t key; int32_t mask; };
constexpr Entry kEntries[] = {
    {1, 1},
    {2, 2},
    {4, 3},
    {8, 4},
    {16, 5},
    {32, 6},
    {64, 7},
    {128, 8},
    {256, 9},
    {512, 10},
    {1024, 11},
    {2048, 12},
    {4096, 13},
    {8192, 14},
    {16384, 15},
    {32768, 16},
    {65536, 17},
    {131072, 18},
    {262144, 19},
    {524288, 20},
    {1048576, 21},
};
}

// 0x0059D440: linear search; the miss result is the literal integer 1.
int32_t LookupLocomotionMask(int32_t key)
{
    for (const Entry& entry : kEntries)
        if (entry.key == key) return entry.mask;
    return 1;
}

// 0x007D7630: preserve the game's 0x0fffffff mask and 5-bit shifted result.
int32_t* SetLocomotionMask(int32_t value, int32_t* word)
{
    if (value <= 1) return reinterpret_cast<int32_t*>(static_cast<uintptr_t>(static_cast<uint32_t>(value)));
    const uint32_t code = static_cast<uint32_t>(LookupLocomotionMask(value) - 1);
    uint32_t bits;
    // The original dereferences `word` only on this branch.
    std::memcpy(&bits, word, sizeof bits);
    bits = (bits & 0x0fffffffu) | (code << 27);
    std::memcpy(word, &bits, sizeof bits);
    return word;
}
}
