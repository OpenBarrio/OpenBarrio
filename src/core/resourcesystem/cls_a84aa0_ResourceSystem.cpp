#include "ts3/resourcesystem/cls_a84aa0_ResourceSystem.h"
#include <bit>
#include <cstring>

namespace ts3::resourcesystem {
namespace {
int32_t Signed(uint32_t bits) { return std::bit_cast<int32_t>(bits); }
int32_t Multiply(int32_t a, int32_t b) { return Signed(uint32_t(a) * uint32_t(b)); }
int32_t Add(int32_t a, int32_t b) { return Signed(uint32_t(a) + uint32_t(b)); }
}

// 0x00A84AA0: keep the original signed comparisons and wrapping 32-bit arithmetic.
void Grid16::ShiftDepth(int32_t delta, bool atEnd, bool dimensionsOnly)
{
    if (dimensionsOnly) {
        depth = Add(depth, delta);
        return;
    }
    const int32_t oldDepth = depth;
    if (!atEnd && delta < 0 && oldDepth > 0) {
        const int32_t removed = Signed(0u - uint32_t(delta));
        if (removed < oldDepth) {
            const int32_t plane = Multiply(width, height);
            const int32_t first = Multiply(plane, removed);
            const int32_t limit = Multiply(plane, oldDepth);
            ptrdiff_t destination = 0;
            for (int32_t i = first; i < limit; i = Add(i, 1))
                samples.begin[destination++] = samples.begin[i];
        }
    }
    depth = Add(depth, delta);
    // 0x9CBAD0 belongs to the unreimplemented dependency; never substitute std::vector.
    CallGameMethod<void, 0x009CBAD0>(&samples, uint32_t(Multiply(Multiply(height, width), depth)));
    if (!atEnd && delta > 0 && oldDepth > 0) {
        int32_t source = Add(Multiply(Multiply(height, width), oldDepth), -1);
        ptrdiff_t destination = (samples.begin ? samples.end - samples.begin : 0) - 1;
        for (; source >= 0; source = Add(source, -1))
            samples.begin[destination--] = samples.begin[source];
        // EA leaves the vacated leading cells unchanged (rather than clearing them).
    }
}

// 0x00A84B80: adjust this by +0x10 and tail-call the same original vector resize.
void Grid16::ResizeSamples(uint32_t count)
{
    CallGameMethod<void, 0x009CBAD0>(&samples, count);
}

// 0x00AC0810: use the source allocator, allocate exactly its vector length, then memcpy.
Grid16* Grid16::ConstructCopy(const Grid16* source)
{
    vtable = reinterpret_cast<void* const*>(uintptr_t(0x01062198));
    width = source->width;
    height = source->height;
    depth = source->depth;
    vtable = reinterpret_cast<void* const*>(uintptr_t(0x010621B0));
    const size_t bytes = source->samples.begin ?
        size_t(reinterpret_cast<uintptr_t>(source->samples.end) - reinterpret_cast<uintptr_t>(source->samples.begin)) : 0;
    const int32_t count = Signed(uint32_t(bytes)) >> 1;
    CallGameMethod<void*, 0x005CC4D0>(&samples, count, &source->samples.allocator);
    // Read the source range again after construction, as EA does. For source == this,
    // the dependency has already replaced it with an empty vector (EA loses old data).
    const size_t copyBytes = source->samples.begin ?
        size_t(reinterpret_cast<uintptr_t>(source->samples.end) - reinterpret_cast<uintptr_t>(source->samples.begin)) : 0;
    if (copyBytes) std::memcpy(samples.begin, source->samples.begin, copyBytes);
    const int32_t copied = Signed(uint32_t(copyBytes)) >> 1;
    samples.end = copied ? samples.begin + copied : samples.begin;
    return this;
}
}
