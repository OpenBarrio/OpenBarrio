// Rewrites for Scratch unit functions 0x00689D10, 0x0068BB40, 0x0068C660,
// and 0x006F7AB0.
#include "ts3/scratch/clu_689d10_Scratch.h"

#include <cstdint>
#include <xmmintrin.h>

namespace ts3::scratch {

int32_t __stdcall ReleaseScratchResources689D10(ScratchReleaseRecord689D10* begin,
                                                 ScratchReleaseRecord689D10* end)
{
    // 0x00689D10 releases +0x1C first, then +0x0C, for each 44-byte record.
    // The original leaves EAX indeterminate for an empty range; use zero there.
    int32_t result = 0;
    if (begin == end)
        return result;
    for (auto* record = begin; record < end; ++record) {
        result = static_cast<int32_t>(record->handle1C);
        if (result)
            result = CallGameFunction<int32_t, 0x004E3F70>(static_cast<uint32_t>(result));
        result = static_cast<int32_t>(record->handle0C);
        if (result)
            result = CallGameFunction<int32_t, 0x004E3F70>(static_cast<uint32_t>(result));
    }
    return result;
}

void ScratchBase689D10::Reset()
{
    // 0x0068BB40 uses MOVAPS at this and this+0x10, so the object is 16-byte aligned.
    const __m128 zero = _mm_setzero_ps();
    _mm_store_ps(reinterpret_cast<float*>(bytes), zero);
    _mm_store_ps(reinterpret_cast<float*>(bytes + 16), zero);
}

bool ScratchClipTest689D10::Contains(const Float4Pair689D10* value) const
{
    // 0x0068C660: preserve the source's six-product accumulation order for each output.
    const __m128 high = value->second;
    const __m128 low = value->first;
    const __m128 highX = _mm_shuffle_ps(high, high, 0x00);
    const __m128 highY = _mm_shuffle_ps(high, high, 0x55);
    const __m128 highZ = _mm_shuffle_ps(high, high, 0xAA);
    const __m128 lowX = _mm_shuffle_ps(low, low, 0x00);
    const __m128 lowY = _mm_shuffle_ps(low, low, 0x55);
    const __m128 lowZ = _mm_shuffle_ps(low, low, 0xAA);

    __m128 first = _mm_add_ps(_mm_mul_ps(vectors[32], highY), _mm_mul_ps(vectors[30], highX));
    first = _mm_add_ps(first, _mm_mul_ps(vectors[34], highZ));
    first = _mm_add_ps(first, _mm_mul_ps(vectors[36], lowX));
    first = _mm_add_ps(first, _mm_mul_ps(vectors[38], lowY));
    first = _mm_add_ps(first, _mm_mul_ps(vectors[40], lowZ));

    __m128 second = _mm_add_ps(_mm_mul_ps(vectors[33], highY), _mm_mul_ps(vectors[31], highX));
    second = _mm_add_ps(second, _mm_mul_ps(vectors[35], highZ));
    second = _mm_add_ps(second, _mm_mul_ps(vectors[37], lowX));
    second = _mm_add_ps(second, _mm_mul_ps(vectors[39], lowY));
    second = _mm_add_ps(second, _mm_mul_ps(vectors[41], lowZ));

    // The original complements CMPLE masks and returns true only when every lane in
    // both vectors is ordered and meets its lower bound. NaN therefore returns false.
    const __m128 zero = _mm_setzero_ps();
    const __m128 allBits = _mm_cmpeq_ps(zero, zero);
    const __m128 failFirst = _mm_xor_ps(_mm_cmple_ps(vectors[42], first), allBits);
    const __m128 failSecond = _mm_xor_ps(_mm_cmple_ps(vectors[43], second), allBits);
    return _mm_movemask_ps(_mm_or_ps(failFirst, failSecond)) == 0;
}

ScratchRecord689D10* ScratchRecordVector689D10::Resize(uint32_t count)
{
    // 0x006F7AB0 measures records in 64-byte units. Null/empty vectors have size zero.
    const auto oldCount = begin && end ? static_cast<uint32_t>(end - begin) : 0u;
    if (count <= oldCount) {
        ScratchRecord689D10* const position = begin ? begin + count : begin;
        // 0x006F5A80 is a bytewise field-copy loop; here its source range is empty.
        auto* const result = CallGameFunction<ScratchRecord689D10*, 0x006F5A80>(end, end, position);
        end = position;
        return result;
    }

    // 0x006F7AB0 initializes only the first 32 bytes through 0x0068BB40, then fields
    // +0x30..+0x37. Bytes +0x20..+0x2F and +0x38..+0x3F remain whatever was on its stack;
    // preserve that EA output quirk instead of normalizing the whole temporary.
    alignas(16) ScratchRecord689D10 defaultRecord;
    reinterpret_cast<ScratchBase689D10*>(&defaultRecord)->Reset();
    auto* fields = defaultRecord.bytes;
    *reinterpret_cast<uint32_t*>(fields + 48) = 0;
    *reinterpret_cast<uint16_t*>(fields + 52) = 100;
    *reinterpret_cast<uint16_t*>(fields + 54) = 0;

    return CallGameMethod<ScratchRecord689D10*, 0x006F76A0>(this, end, count - oldCount, &defaultRecord);
}

}  // namespace ts3::scratch
