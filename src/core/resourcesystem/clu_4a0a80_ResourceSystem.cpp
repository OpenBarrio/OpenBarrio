#include "ts3/resourcesystem/clu_4a0a80_ResourceSystem.h"
#include <cstring>
namespace ts3::resourcesystem {
// 0x004E8650. Growth arithmetic is deliberately uint32_t, including overflow.
bool ResourceArena::Grow(uint32_t requested)
{
    const uint32_t active = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(usedEnd) - reinterpret_cast<uintptr_t>(cursor));
    uint32_t size = requested + active + (active >> 2) + 4104;
    if (size < 8192) size = 8192;
    auto* block = static_cast<ArenaBlock*>(allocate(size, &size, context));
    if (!block) return false;
    block->previous = blocks;
    block->end = reinterpret_cast<uint8_t*>(block) + size;
    blocks = block;
    limit = block->end;
    auto* start = reinterpret_cast<uint8_t*>((reinterpret_cast<uintptr_t>(block + 1) + 7) & ~uintptr_t(7));
    if (active && start && cursor) std::memcpy(start, cursor, active);
    cursor = start;
    usedEnd = start + active;
    return true;
}
// 0x004A0A80
void* AllocateResource(uint32_t bytes, ResourceArena* arena)
{
    const uint32_t rounded = (bytes + 7) & ~uint32_t(7);
    const uint32_t remaining = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(arena->limit) - reinterpret_cast<uintptr_t>(arena->cursor) - rounded);
    if (static_cast<int32_t>(remaining) < 0 && !arena->Grow(rounded)) return nullptr;
    auto* result = arena->cursor;
    arena->cursor = reinterpret_cast<uint8_t*>(reinterpret_cast<uintptr_t>(result) + rounded);
    arena->usedEnd = arena->cursor;
    return result;
}
// 0x004E8800. EA rounds the alignment mask up to eight, not to a power of two.
void* ResourceArena::AllocateAligned(uint32_t bytes, uint32_t alignment, uint32_t offset, bool grow)
{
    const uint32_t mask = (alignment - 1) | 7;
    const uint32_t quantum = mask + 1;
    const uint32_t rounded = (bytes + 7) & ~uint32_t(7);
    const uintptr_t wideMask = ~static_cast<uintptr_t>(mask);
    uintptr_t result = ((reinterpret_cast<uintptr_t>(cursor) + mask + offset) & wideMask) - offset;
    if (grow && result + rounded > reinterpret_cast<uintptr_t>(limit)) {
        if (!Grow(offset + rounded + quantum)) return nullptr;
        result = ((reinterpret_cast<uintptr_t>(cursor) + mask + offset) & wideMask) - offset;
    }
    cursor = reinterpret_cast<uint8_t*>(result + rounded);
    usedEnd = cursor;
    return reinterpret_cast<void*>(result);
}
// 0x0054ABB0
char16_t* ResourceStringPool::CopyString(const char16_t* text)
{
    uint32_t length = 0;
    while (text[length]) ++length;
    auto* result = static_cast<char16_t*>(AllocateResource(2 * length + 2, &arena));
    auto* out = result;
    do { *out++ = *text; } while (*text++);
    return result;
}
// 0x004A7450 / 0x004A7490 / 0x004A7560: no second implementation of verified dependencies.
bool PackedFileAdapter::SeekPacked(uint32_t position) { return SeekInFile(position); }
uint32_t PackedFileAdapter::ReadPacked(void* output, uint32_t size) { return ReadAtPosition(output, size); }
bool PackedFileAdapter::ResizePacked(uint32_t size) { return SetFileSize(size); }
// 0x004AFD30
int32_t ExchangeResourceSetting(int32_t value)
{
#if defined(_M_IX86)
    auto& setting = *reinterpret_cast<int32_t*>(uintptr_t(0x011C78D8));
    const int32_t previous = setting;
    setting = value;
    return previous;
#else
    (void)value;
    ts3::AbortNotRewritten("ResourceSystem game-global setting");
    return 0;
#endif
}
}
namespace ts3::resourcesystem {
namespace {
template<class T> uint32_t ElementCount(const T* first, const T* last)
{
    return static_cast<uint32_t>((reinterpret_cast<uintptr_t>(last) - reinterpret_cast<uintptr_t>(first)) / sizeof(T));
}
}
// 0x0093B2C0
uintptr_t PathVector::Insert(const char16_t** position, const char16_t* const* value)
{
    if (end != capacity) {
        auto* source = value;
        if (reinterpret_cast<uintptr_t>(source) >= reinterpret_cast<uintptr_t>(position) &&
            reinterpret_cast<uintptr_t>(source) < reinterpret_cast<uintptr_t>(end)) ++source;
        if (end) *end = *(end - 1);
        auto* result = std::memmove(position + 1, position, sizeof(*end) * ElementCount(position, end - 1));
        *position = *source;
        ++end;
        return reinterpret_cast<uintptr_t>(result);
    }
    const uint32_t count = ElementCount(begin, end);
    const uint32_t newCount = count ? count * 2 : 1;
    auto* storage = newCount ? ts3::CallGameVirtual<const char16_t**, 2>(allocator,
        static_cast<uint32_t>(sizeof(*end) * newCount), name, flags) : nullptr;
    const uint32_t prefix = ElementCount(begin, position);
    if (prefix) std::memmove(storage, begin, sizeof(*end) * prefix);
    auto* out = reinterpret_cast<const char16_t**>(reinterpret_cast<uintptr_t>(storage) + sizeof(*end) * prefix);
    if (out) *out = *value;
    const uint32_t tail = ElementCount(position, end);
    if (tail) std::memmove(out + 1, position, sizeof(*end) * tail);
    auto* next = out + 1 + tail;
    uintptr_t result = reinterpret_cast<uintptr_t>(begin);
    if (begin) result = ts3::CallGameVirtual<uintptr_t, 3>(allocator, begin,
        static_cast<uint32_t>(sizeof(*end) * ElementCount(begin, capacity)));
    begin = storage; end = next;
    capacity = reinterpret_cast<const char16_t**>(reinterpret_cast<uintptr_t>(storage) + sizeof(*end) * newCount);
    return result;
}
// 0x004A1BA0
uintptr_t PathVector::Append(const char16_t* const* value)
{
    auto* result = end;
    if (reinterpret_cast<uintptr_t>(result) >= reinterpret_cast<uintptr_t>(capacity)) return Insert(result, value);
    end = reinterpret_cast<const char16_t**>(reinterpret_cast<uintptr_t>(result) + sizeof(*end));
    if (result) *result = *value;
    return reinterpret_cast<uintptr_t>(result);
}
// 0x004AB770
uintptr_t PairVector::Insert(ResourcePair* position, const ResourcePair* value)
{
    if (end != capacity) {
        auto* source = value;
        if (reinterpret_cast<uintptr_t>(source) >= reinterpret_cast<uintptr_t>(position) &&
            reinterpret_cast<uintptr_t>(source) < reinterpret_cast<uintptr_t>(end)) ++source;
        if (end) { end->key = (end - 1)->key; end->value = (end - 1)->value; }
        auto* from = end - 1;
        auto* out = end;
        while (from != position) {
            --from; --out; out->key = from->key; out->value = from->value;
        }
        position->key = source->key; position->value = source->value;
        ++end;
        return source->key;
    }
    const uint32_t count = ElementCount(begin, end);
    const uint32_t newCount = count ? count * 2 : 1;
    auto* storage = newCount ? ts3::CallGameVirtual<ResourcePair*, 2>(allocator,
        static_cast<uint32_t>(sizeof(ResourcePair) * newCount), name, flags) : nullptr;
    auto* out = ts3::CallGameFunction<ResourcePair*, 0x005CC4A0>(begin, position, storage);
    if (out) { out->key = value->key; out->value = value->value; }
    auto* next = ts3::CallGameFunction<ResourcePair*, 0x005CC4A0>(position, end, out + 1);
    uintptr_t result = reinterpret_cast<uintptr_t>(begin);
    if (begin) result = ts3::CallGameVirtual<uintptr_t, 3>(allocator, begin,
        static_cast<uint32_t>(sizeof(ResourcePair) * ElementCount(begin, capacity)));
    begin = storage; end = next;
    capacity = reinterpret_cast<ResourcePair*>(reinterpret_cast<uintptr_t>(storage) + sizeof(ResourcePair) * newCount);
    return result;
}
}
namespace ts3::resourcesystem {
// 0x0054B1B0. Unwritten fields (including values when count is zero) stay poisoned.
ResourceMapEntry* ResourceStringPool::CreateEntry(const ResourceKey12* first, uint32_t kind, const ResourceKey12* second, uint32_t count)
{
    auto* entry = static_cast<ResourceMapEntry*>(AllocateResource(sizeof(ResourceMapEntry), &arena));
    if (entry) entry->links.next = entry->links.previous = &entry->links;
    entry->first = *first; entry->second = *second;
    entry->kind = kind; entry->count = count;
    if (count) {
        const uint32_t bytes = 4 * count;
        auto* values = static_cast<uint32_t*>(AllocateResource(bytes, &arena));
        if (values) std::memset(values, 0, bytes);
        entry->values = values;
    }
    return entry;
}
// 0x0054C770. The three pushes preceding factory slot 2 belong to AllocateAligned:
// slot 2 takes no arguments (the decompiler assigns them to the wrong call).
void* ResourceObjectPool::Create(uint32_t code)
{
    auto* factory = ts3::CallGameFunction<void*, 0x0054C370>(code);
    if (!factory) return nullptr;
    const uint32_t bytes = ts3::CallGameVirtual<uint32_t, 2>(factory);
    auto* memory = arena.AllocateAligned(bytes, 16, 0, true);
    auto* result = ts3::CallGameVirtual<void*, 0>(factory, this, memory);
    if (result) {
        auto* link = freeLinks;
        if (link) freeLinks = link->next;
        // A missing free link is an EA invariant violation, not a recoverable failure.
        if (reinterpret_cast<uintptr_t>(link) + offsetof(ResourceObjectLink, object)) link->object = result;
        link->next = reinterpret_cast<ResourceObjectLink*>(&objects);
        link->previous = reinterpret_cast<ResourceObjectLink*>(objects.previous);
        objects.previous->next = reinterpret_cast<ResourceLinks*>(link);
        objects.previous = reinterpret_cast<ResourceLinks*>(link);
    }
    return result;
}
}
namespace ts3::resourcesystem {
// 0x005833C0. The minimum-priority scan is redundant but reads every node in EA.
void* ResourceProviders::BuildAggregate(ResourceObjectPool* pool, uintptr_t unused)
{
    (void)unused;
    auto* sentinel = reinterpret_cast<ProviderLink*>(&providers);
    uint32_t count = 0;
    for (auto* p = reinterpret_cast<ProviderLink*>(providers.next); p != sentinel; p = p->next) ++count;
    if (!count) return nullptr;
    auto* object = pool->Create(0x93DFFDC3u);
    auto* aggregate = ts3::CallGameVirtual<void*, 1>(object, uint32_t(0x93DFFDC3));
    uint32_t minimum = ~uint32_t(0);
    for (auto* p = reinterpret_cast<ProviderLink*>(providers.next); p != sentinel; p = p->next)
        if (p->priority <= minimum) minimum = p->priority;
    (void)minimum;
    for (auto* p = reinterpret_cast<ProviderLink*>(providers.next); p != sentinel; p = p->next) {
        if (!p->provider) continue;
        auto* value = ts3::CallGameVirtual<void*, 3>(p->provider, pool);
        if (value) ts3::CallGameMethod<void, 0x00554EE0>(aggregate, p->priority, value);
        else {
#if defined(_M_IX86)
            const uint32_t flags = *reinterpret_cast<const uint32_t*>(uintptr_t(0x011E90FC));
#else
            ts3::AbortNotRewritten("resource provider diagnostic flags");
            const uint32_t flags = 0;
#endif
            auto* state = ts3::CallGameVirtual<const uint8_t*, 9>(manager);
            uint32_t active; std::memcpy(&active, state + 44, sizeof active);
            if ((flags & active) && p->priority == 10000)
                ts3::CallGameVirtual<void, 3>(p->provider, pool);
        }
    }
    return ts3::CallGameMethod<int32_t, 0x00554D10>(aggregate) ? aggregate : nullptr;
}
}
#include "ts3/io/file_stream.h"
namespace ts3::resourcesystem {
namespace { struct MarkerString { uint8_t *begin, *end, *capacity; const char* category; }; }
// 0x004AE870. Preserve scanning stale tail bytes, the 497-byte step, and the
// unsigned read-count-minus-16 comparison, including its underflow on short reads.
int32_t FindEmbeddedPackage(const wchar_t* filename)
{
    ts3::io::FileStream file;
    file.Construct(static_cast<const char*>(nullptr));
    file.SetPath(filename);
    if (!file.Open(1, 6, 1, 0)) { file.Destruct(); return -1; }
    const uint32_t size = file.GetSize();
#if defined(_M_IX86)
    const auto* signature = reinterpret_cast<const uint8_t*>(uintptr_t(0x00FB2AFC));
    const auto* debugFile = reinterpret_cast<const char*>(uintptr_t(0x00F98980));
#else
    const uint8_t signatureBytes[16]{0x7F,0x9D,0x88,0xEC,0x8F,0x24,0x03,0x6C,0xC9,0xA6,0x31,0x56,0x5B,0xCF,0x77,0x20};
    const auto* signature = signatureBytes;
    const char* debugFile = nullptr;
#endif
    auto* run = ts3::CallGameFunction<uint8_t*, 0x004E3F90>(513, "ResourceSystem/ResourceMan/DBPFUtil/RunBuffer", 0, 0, debugFile, 231);
    std::memset(run, 0, 513);
    const char* category = "ResourceSystem/ResourceMan/DBPFUtil/Signature";
    MarkerString marker;
    ts3::CallGameMethod<void, 0x00406B10>(&marker, signature, 16, &category);
    marker.begin[0] = static_cast<uint8_t>(~marker.begin[0]);
    uint32_t filled = 0, base = 0;
    int32_t found = -1;
    bool startsWithSignature = false;
    if (size) {
        const auto length = static_cast<uint32_t>(marker.end - marker.begin);
        do {
            const uint32_t read = file.Read(run + filled, 512 - filled);
            if (!base && read >= 4) startsWithSignature = std::memcmp(run, signature, 4) == 0;
            uint32_t offset = ~uint32_t(0);
            if (length <= 512) {
                const auto* at = ts3::CallGameFunction<const uint8_t*, 0x004AE460>(run, run + 512, marker.begin, marker.end);
                if (at != run + 512 || !length) offset = static_cast<uint32_t>(at - run);
            }
            if (offset != ~uint32_t(0)) {
                found = offset <= read - 16 ? static_cast<int32_t>(offset + base + 16) : -1;
                break;
            }
            std::memmove(run, run + 497, 15);
            base += 497; filled = 15;
        } while (base < size);
    }
    file.Close();
    if (marker.capacity - marker.begin > 1) ts3::CallGameFunction<void, 0x004E3F70>(marker.begin);
    if (run) ts3::CallGameFunction<void, 0x004E3F70>(run);
    file.Destruct();
    return found == -1 && startsWithSignature ? 0 : found;
}
}
