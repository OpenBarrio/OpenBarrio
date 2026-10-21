#include "ts3/resource/package_index.h"

#include <cstring>

namespace ts3::resource {

const RecordInfo* IndexCompressed::GetRecord(const ResourceKey* key)
{
    const IndexRecord* record = Find(*key);
    if (!record)
        return nullptr;
    lastRecord.offset = record->offset;
    lastRecord.valid = 1;
    if (record->size16 == 0xFFFF) {
        lastRecord.size = overflow[record->memSize16].size;
        lastRecord.memSize = overflow[record->memSize16].memSize;
    } else {
        lastRecord.size = record->size16;
        lastRecord.memSize = record->memSize16;
    }
    lastRecord.compression = lastRecord.memSize != lastRecord.size ? 0xFFFF : 0;
    return &lastRecord;
}

bool IndexCompressed::Load(const void* index, uint32_t bytes, uint32_t)
{
    CompressedIndex built;
    if (!BuildCompressedIndex(static_cast<const uint8_t*>(index), bytes, built))
        built = CompressedIndex{};  // truncated input; the original would read past it

    // The same vector operations as the original, in its order, so storage is reused and
    // grown identically: overflow and buckets are emptied (not freed), overflow entries are
    // appended one by one, buckets are resized with {0, 0, 0, -1}, records are resized from
    // their old size.
    overflow.clear();
    buckets.clear();
    for (const IndexSizeOverflow& entry : built.overflow)
        overflow.push_back(entry);
    groupCount = built.groupCount;
    buckets.resize(groupCount + 1, IndexBucket{0, 0, 0, 0xFFFFFFFFu});
    records.resize(built.records.size());
    if (!built.records.empty())
        std::memcpy(records.data(), built.records.data(), built.records.size() * sizeof(IndexRecord));
    if (!built.buckets.empty())
        std::memcpy(buckets.data(), built.buckets.data(), buckets.size() * sizeof(IndexBucket));
    return true;
}

}  // namespace ts3::resource
