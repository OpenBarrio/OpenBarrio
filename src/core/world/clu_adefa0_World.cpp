#include "ts3/world/clu_adefa0_World.h"

#include <cstring>

namespace ts3::world {
// 0x00ADEFA0, thiscall ret 8. The stored end pointer is the second native pointer.
char* WorldRecordVector::CopyRecords(char* destination, const char* source)
{
    char* oldEnd = this->end;
    const char* cursor = source;
    char* output = destination;
    const bool copiedAny = cursor != oldEnd;
    while (cursor != oldEnd) {
        std::memcpy(output, cursor, 28);
        cursor += 28;
        output += 28;
    }
    const auto bytes = source - (copiedAny ? oldEnd : destination);
    const auto records = bytes / 28; // game's signed divide-by-28 sequence truncates toward zero
    this->end = oldEnd + records * 28;
    return destination;
}

// 0x00AE0990, thiscall ret 8. Keep each multiply/add as a separate float operation.
float* __thiscall WorldTransformState::TransformVector(const float* input, float* output) const
{
    for (unsigned i = 0; i != 4; ++i) {
        float value = input[0] * basisX[i];
        value = value + translation[i];
        value = value + input[1] * basisY[i];
        value = value + input[2] * basisZ[i];
        output[i] = value;
    }
    return const_cast<float*>(input);
}
}
