#include "ts3/cas/clu_4a6710_CAS.h"
#include <cstring>
#include "ts3/world/clu_4a0a00_World.h"
#include "ts3/resourceload/clu_4a6860_ResourceLoad.h"

namespace ts3::cas {
namespace {
struct KeyWriter {
    void* object;
    void* ownedStream;
    void* stream;
    uintptr_t opaque;
    int32_t flags;
    uint8_t success;
    uint8_t reserved[3];
};
TS3_X86_OFFSET(KeyWriter, stream, 8);
TS3_X86_OFFSET(KeyWriter, flags, 16);
TS3_X86_OFFSET(KeyWriter, success, 20);
TS3_X86_SIZE(KeyWriter, 24);
}
// 0x005BB770, thiscall, ret 4; only AL is defined.
uint8_t CompositorKeyList::Save(CachePackedFile* file)
{
    if (!file) return 0;
    CallGameFunction<void*, 0x0072F1C0>();
    int32_t previousLimit = CallGameMethod<int32_t, 0x004A69F0>(file);
    file->SetLimit(0);
    void* record = nullptr;
    uint8_t success = 0;
    if (CallGameVirtual<bool, 13>(file, &key, &record, 2, 2, 1, 0)) {
        auto stream = CallGameVirtual<void*, 6>(record);
        KeyWriter writer;
        CallGameMethod<void*, 0x007D91B0>(&writer, stream, 1);
        uint32_t count = 0;
        for (auto node = anchor.next; node != &anchor; node = node->next) ++count;
        auto status = CallGameFunction<uint8_t, 0x004DDAD0>(writer.stream, 1, writer.flags);
        CallGameMethod<uint8_t, 0x007D77C0>(&writer, status);
        status = CallGameFunction<uint8_t, 0x004DDAD0>(writer.stream, count, writer.flags);
        CallGameMethod<uint8_t, 0x007D77C0>(&writer, status);
        for (auto node = anchor.next; node != &anchor; node = node->next) {
            status = CallGameFunction<uint8_t, 0x007D74E0>(writer.stream, &reinterpret_cast<CompositorNode*>(node)->key, writer.flags);
            CallGameMethod<uint8_t, 0x007D77C0>(&writer, status);
        }
        success = writer.success;
        if (!CallGameVirtual<bool, 8>(record)) success = 0;
        CallGameMethod<void, 0x007D7760>(&writer);
    }
    file->SetLimit(previousLimit);
    if (record) CallGameVirtual<void, 2>(record);
    return success;
}
// 0x005BB8B0, thiscall, ret 4; returns the full flush result, not the byte count.
int32_t CompositorCache::Evict(uint32_t bytes)
{
    if (!file) return 0;
    uint32_t removed = 0;
    while (removed < bytes && keys.anchor.next != &keys.anchor) {
        auto node = reinterpret_cast<CompositorNode*>(keys.anchor.previous);
        CompositorKey key = node->key;
        node->link.previous->next = node->link.next;
        node->link.next->previous = node->link.previous;
        CallGameFunction<void, 0x004E3F70>(node);
        removed += CallGameMethod<uint32_t, 0x005BB6A0>(this, &key);
    }
    if (!file) return 0; // Eviction callbacks can clear the file.
    auto stream = CallGameVirtual<void*, 20>(file);
    return stream ? CallGameVirtual<int32_t, 7>(stream) : 0;
}
// 0x005BBC10, thiscall, ret 0. A release callback may install a second reference.
void CompositorCache::Close()
{
    if (!file) return;
    if (auto manager = CallGameFunction<void*, 0x0072F1C0>())
        CallGameVirtual<void, 25>(manager, file);
    keys.Save(file);
    CallGameVirtual<void, 2>(file);
    auto previous = file;
    file = nullptr;
    if (previous)
        CallGameVirtual<void, 1>(&reinterpret_cast<CacheReference::Object*>(previous)->secondary);
    previous = file;
    if (previous) {
        file = nullptr;
        CallGameVirtual<void, 1>(&reinterpret_cast<CacheReference::Object*>(previous)->secondary);
    }
}
// 0x005BC6D0, thiscall, ret 4; only AL is the return value.
uint8_t CompositorCache::Open(const uint16_t* suffix)
{
    if (file) return 1;
    auto manager = reinterpret_cast<void*>(world::WorldResourceService());
    uint16_t path[260];
    CopyUtf16(path, CallGameVirtual<const uint16_t*, 10>(manager), 260);
    CallGameFunction<int, 0x004FF470>(path, suffix, 260);
    auto memory = CallGameFunction<void*, 0x004E41F0>(sizeof(CachePackedFile), "CAS/CompositorCache/Cache", 0, 0, nullptr, 0);
    auto created = memory ? CallGameMethod<CachePackedFile*, 0x004A8F70>(memory, path, 0) : nullptr;
    auto previous = file;
    if (created != previous) {
        if (created) CallGameVirtual<void, 0>(&reinterpret_cast<CacheReference::Object*>(created)->secondary);
        file = created;
        if (previous) CallGameVirtual<void, 1>(&reinterpret_cast<CacheReference::Object*>(previous)->secondary);
    }
    if (!file || !CallGameVirtual<uint8_t, 1>(file)) return 0;
    const bool bounded = !CallGameFunction<uint8_t, 0x0058B100>("UnboundedTCC", 0);
    if (bounded) file->SetLimit(static_cast<int32_t>(limit));
    CallGameMethod<void, 0x005169D0>(file, bounded ? 1 : 3);
    reinterpret_cast<world::WorldDbOptions*>(file)->SetOptions(18, 0.25f);
    file->SetObserver(&opaque[2]);
    const int mode = CallGameFunction<uint8_t, 0x0058B100>("ClearTCC", 0) ? 2 : 4;
    if (!CallGameVirtual<uint8_t, 6>(file, 3, mode, 0)) {
        CallGameVirtual<void, 2>(file);
        previous = file;
        file = nullptr;
        if (previous) CallGameVirtual<void, 1>(&reinterpret_cast<CacheReference::Object*>(previous)->secondary);
        return 0;
    }
    auto registry = CallGameFunction<void*, 0x004AFD20>();
    if (!CallGameVirtual<uint8_t, 13>(registry, 1, file, 0)) {
        CallGameVirtual<void, 2>(file);
        // Clear-before-release, including reentrant replacement, matches 0x005BB380.
        reinterpret_cast<CacheReference*>(&file)->Clear();
        return 0;
    }
    CallGameMethod<void, 0x005BBDE0>(&keys, file);
    if (!bounded) return 1;
    CallGameMethod<void, 0x005BC620>(this);
    const uint32_t threshold = limit + 0x100000u;
    // EA deliberately reads the total twice and compares it unsigned here.
    if (static_cast<uint32_t>(file->SpaceTotal()) < threshold ||
        static_cast<uint32_t>(file->SpaceTotal()) <= threshold) return 1;
    for (;;) {
        struct KeyArray { CompositorKey* begin; CompositorKey* end; CompositorKey* capacity; const char* label; };
        TS3_X86_SIZE(KeyArray, 16);
        KeyArray array{nullptr, nullptr, nullptr, "CAS/CompositorCache/TempKeyList"};
        CallGameVirtual<void, 12>(file, &array, 0);
        CompositorKey selected{};
        int32_t highest = -1;
        uint32_t size = 0;
        for (auto key = array.begin; key != array.end; ++key) {
            struct RecordLocation { int32_t offset; uint32_t size; uint32_t rest; } location;
            if (CallGameVirtual<uint8_t, 13>(file, key, nullptr, 1, 3, 0, &location) &&
                location.size && location.offset > highest) {
                selected = *key;
                highest = location.offset;
                size = location.size;
            }
        }
        bool stop = highest < static_cast<int32_t>(limit); // signed, unlike the total check
        if (!stop) {
            int32_t offset;
            if (file->Reserve(&offset, size)) {
                if (offset > highest || !resourceload::MoveRecord(file,
                    reinterpret_cast<const resource::ResourceKey*>(&selected), offset, size)) {
                    file->ReturnSpace(offset, size);
                    stop = true;
                }
            } else if (!Evict(limit >> 2)) {
                stop = true;
            } else {
                keys.Save(file);
                CallGameVirtual<void, 9>(file);
            }
        }
        if (array.begin) CallGameFunction<void, 0x004E3F70>(array.begin);
        if (stop) break;
    }
    CallGameVirtual<void, 9>(file);
    keys.Save(file);
    CallGameVirtual<void, 9>(file);
    return 1;
}
// 0x004A6710
void* CachePackedFile::SetObserver(void* observer)
{
    indexObserver = observer;
    return observer;
}
// 0x004A9C30
int32_t CacheLimit::SetLimit(int32_t value)
{
    const int32_t normalized = value ? value : 0x7fffffff;
    limit = static_cast<uint32_t>(normalized);
    return normalized;
}
// 0x004A69E0
int32_t CachePackedFile::SetLimit(int32_t value)
{
    CacheLimit state{freeSpace.total, freeSpace.limit};
    const int32_t result = state.SetLimit(value);
    freeSpace.limit = state.limit;
    return result;
}
namespace {
void Lock(thread::Mutex& mutex)
{
    // Lock/Unlock are not yet verified: keep the game ABI, including the time reference.
    const uint32_t timeout = 0xffffffffu; // 0x00FB24F8
    CallGameMethod<int, 0x004E16F0>(&mutex, &timeout);
}
void Unlock(thread::Mutex& mutex)
{
    CallGameMethod<int, 0x004E17B0>(&mutex);
}
}
// 0x004A6EB0
int32_t CachePackedFile::SpaceTotal()
{
    Lock(recordMutex);
    const int32_t result = freeSpace.total;
    Unlock(recordMutex);
    return result;
}
// 0x004A6F80
bool CachePackedFile::Reserve(int32_t* offset, uint32_t size)
{
    Lock(recordMutex);
    bool result = false;
    if (openFlags & 2) {
        // Already verified implementation of 0x004AC4E0.
        *offset = freeSpace.Allocate(size, false);
        result = *offset >= 0;
    }
    Unlock(recordMutex);
    return result;
}
// 0x004A6FD0
bool CachePackedFile::ReturnSpace(int32_t offset, int32_t size)
{
    Lock(recordMutex);
    if (offset && size) {
        if (writeFlags & 0x10)
            freeSpace2.AddFree(offset, size, true);
        else
            freeSpace.AddFree(offset, size, false);
        freeSpaceChanged = true;
    }
    Unlock(recordMutex);
    return true;
}
// 0x004AC730
void* CacheFreeSpaceMap::Drain(CacheFreeSpaceMap* source)
{
    for (const auto& block : source->blocks)
        AddFree(block.first, block.second, false);
    source->blocks.clear();
    // EASTL keeps the end iterator as a single pointer to the anchor.
    const auto end = source->blocks.end();
    void* anchor;
    static_assert(sizeof(end) == sizeof(anchor), "EASTL iterator representation");
    std::memcpy(&anchor, &end, sizeof(anchor));
    return anchor;
}
// 0x005BB380
intptr_t CacheReference::Clear()
{
    Object* previous = object;
    object = nullptr;
    if (previous)
        return CallGameVirtual<intptr_t, 1>(&previous->secondary);
    return 0;
}
// 0x004FF380
uint32_t CopyUtf16(uint16_t* destination, const uint16_t* source, uint32_t capacity)
{
    uint32_t length = 0;
    while (source[length]) ++length;
    if (length < capacity) {
        std::memcpy(destination, source, (static_cast<size_t>(length) + 1) * 2);
    } else if (capacity) {
        std::memcpy(destination, source, (static_cast<size_t>(capacity) - 1) * 2);
        destination[capacity - 1] = 0;
    }
    return length;
}
} // namespace ts3::cas
