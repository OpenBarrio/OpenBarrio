// Rewrites for small Scripting sorted-list and coordinate helpers.
#include "ts3/scripting/clu_805460_Scripting.h"

#include <cstdint>

namespace ts3::scripting {

bool ScriptState805460::HasActiveOrKeyMatch() const
{
    // 0x00805460 checks the 24-byte records' +0x0D flag or +0x08 key.
    uint8_t* cursor = reinterpret_cast<uint8_t*>(begin);
    const uint8_t* const finish = reinterpret_cast<const uint8_t*>(end);
    if (cursor == finish)
        return false;
    if (cursor == finish) // Preserved redundant compare at 0x0080546E.
        return false;
    do {
        if (cursor[0x0d] != 0 || *reinterpret_cast<const uint32_t*>(cursor + 8) == key)
            return true;
        cursor += 0x18;
    } while (cursor != finish);
    return false;
}

uint32_t* LowerBoundKey0082AB70(uint32_t* begin, uint32_t* end, const uint32_t* key)
{
    // 0x0082AB70's signed arithmetic-shift binary search (lower bound).
    if (begin == end)
        return begin;
    auto* result = begin;
    std::ptrdiff_t count = end - begin;
    while (count > 0) {
        const std::ptrdiff_t half = count >> 1;
        if (result[half] >= *key) {
            count >>= 1;
        } else {
            result += half + 1;
            count += -1 - half;
        }
    }
    return result;
}

uint32_t* SortedWordVector82DCD0::InsertAt(uint32_t* position, const uint32_t* value)
{
    // 0x00AD3080 delegates middle inserts and capacity growth to the game's
    // vector helper; the simple append path writes one key directly.
    const std::ptrdiff_t index = begin ? position - begin : 0;
    if (end == capacity || position != end) {
        CallGameMethod<void, 0x008621C0>(this, position, value);
    } else {
        uint32_t* const oldEnd = end;
        end = oldEnd + 1;
        if (oldEnd) {
            *oldEnd = *value;
            return begin + index;
        }
    }
    return begin + index;
}

SearchResult82DCD0* SortedWordVector82DCD0::FindOrInsert(SearchResult82DCD0* result,
                                                         const uint32_t* key)
{
    // 0x0082DCD0 passes option10 as an unused fourth stack argument to the
    // lower-bound helper; the original helper reads only its first three.
    uint32_t* const position = LowerBoundKey0082AB70(begin, end, key);
    if (position == end || *key < *position) {
        result->position = InsertAt(position, key);
        result->inserted = 1;
    } else {
        result->position = position;
        result->inserted = 0;
    }
    return result;
}

void* CoordinateMapAF03C0::GetOrAddCoordinate(int32_t x, int32_t y, float z)
{
    // 0x00AF03C0 accepts both bounds inclusively and returns bounds on failure.
    CoordinateBoundsAF03C0* const limit = bounds;
    if (x < 0 || x > limit->maxX || y < 0 || y > limit->maxY)
        return limit;

    // AECB20 is a game method not rewritten in this unit.
    CallGameMethod<void, 0x00AECB20>(this, x, y, z);
    const uint32_t packed = static_cast<uint32_t>(x) | (static_cast<uint32_t>(y) << 16);
    SearchResult82DCD0 local{};
    const uintptr_t localAddress = reinterpret_cast<uintptr_t>(&local);
    points.FindOrInsert(&local, &packed);
    // EA returns the address of this stack local from 0x00AF03C0.
    return reinterpret_cast<void*>(localAddress);
}

}  // namespace ts3::scripting
