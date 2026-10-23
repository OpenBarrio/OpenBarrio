// DatabasePackedStream's raw read path, rewritten from TS3W.exe 0x0072CF90, 0x0072D300,
// 0x0072D360
// 0x00401040) and the throttled handle opener (0x0072CE10) stay original game code, called
// by address.
#include "ts3/resource/packed_stream.h"

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::resource {

// 0x0072CF90. The request lives on the stack, as in the game (0x48 bytes).
int PackedStreamAsyncRead(DatabasePackedStream* self, uint32_t offset, void* dst, uint32_t size, uint8_t flag)
{
    void* file;
    if (flag) {
        file = self->throttledHandle;
        if (!file)
            file = CallGameMethod<void*, 0x0072CE10>(self);
    } else {
        file = self->primaryHandle;
    }
    alignas(16) uint8_t request[0x48] = {};
    CallGameMethod<void, 0x00401000>(request);
    // The high half of the offset is its sign bit: offsets of 2 GB or more name a wrapped
    // position, as in the original (kept).
    CallGameMethod<void, 0x00401400>(request, file, dst, offset, static_cast<uint32_t>(static_cast<int32_t>(offset) >> 31),
                                     size, 0u, static_cast<void*>(nullptr), 0u, 500u);
    const uint32_t forever = 0xFFFFFFFFu;  // the game's wait-forever constant at 0x00FFD74C
    CallGameMethod<int32_t, 0x00401290>(request, &forever);
    const uint32_t noWait = 0;  // 0x00FFD748 is always 0, so the second wait only reads the state
    const int32_t state = CallGameMethod<int32_t, 0x00401290>(request, &noWait);
    CallGameMethod<void, 0x00401040>(request);
    return state == 1 ? 1 : 0;
}

// 0x0072D300.
bool DatabasePackedStream::ReadRaw(void* dst, uint32_t offset, uint32_t size, uint8_t flag)
{
    mutex.Lock();
    bool ok = false;
    if (openFlags)
        // The comparison kept from the original: PackedStreamAsyncRead answers 0 or 1, so
        // this is true even when the request failed and the buffer holds nothing (an EA
        // bug that affects the result — reported in the project notes).
        ok = PackedStreamAsyncRead(this, offset, dst, size, flag) != -1;
    mutex.Unlock();
    return ok;
}

// 0x0072D360.
bool DatabasePackedStream::ReadAndDecompress(uint32_t compression, uint32_t offset, uint32_t size, uint32_t memSize,
                                             void* dst, uint8_t flag)
{
    if (!offset)
        return false;  // EA: a read at offset 0 always fails (kept)
    void* raw = allocator->Alloc(size, "ResourceLoad/PackedStream/Raw", 0);
    if (!raw)
        return false;
    bool ok = false;
    if (ReadRaw(raw, offset, size, flag))
        ok = DecompressRecord(compression, raw, size, memSize, dst);
    allocator->Free(raw, 0);
    return ok;
}

// 0x0072D3E0.
bool DatabasePackedStream::ReadRecordRaw(const ResourceKey* key, void* dst, uint32_t maxSize)
{
    const RecordInfo* entry = CallGameVirtual<const RecordInfo*, 10>(index, key);
    if (!entry)
        return false;
    uint32_t size = entry->size;
    if (maxSize < size)
        size = maxSize;
    if (!dst || !size)
        return true;
    if (!openFlags)
        return false;
    uint8_t flag = 1;
    if (perKeyObject)
        flag = CallGameVirtual<uint8_t, 1>(perKeyObject, key);
    return ReadRaw(dst, entry->offset, size, flag);
}

}  // namespace ts3::resource
