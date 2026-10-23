#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cwchar>
#include <cstdlib>
#include <cstring>
#include "ts3/services/cls_4a4ba0_Services.h"
#include "ts3/resourcesystem/clu_455970_ResourceSystem.h"

namespace ts3::services {
namespace {
bool Less(const ServiceKey& a, const ServiceKey& b)
{
    if (a.first != b.first) return a.first < b.first;
    if (a.second != b.second) return a.second < b.second;
    return a.last < b.last;
}
bool OwnsString(const ServiceString& s)
{
    const uintptr_t difference = reinterpret_cast<uintptr_t>(s.capacity) - reinterpret_cast<uintptr_t>(s.begin);
    return static_cast<intptr_t>(difference & ~uintptr_t(1)) > 2 && s.begin;
}
void InitializeTree(ServiceTree& t, void* allocator, const char* name)
{
    // The comparator word is deliberately untouched by the original constructor.
    t.anchor.right = &t.anchor;
    t.anchor.left = &t.anchor;
    t.anchor.parent = nullptr;
    // Original zeroes the entire color dword, then its low byte again.
    std::memset(&t.anchor.color, 0, sizeof(ServiceNodeBase) - offsetof(ServiceNodeBase, color));
    t.count = 0;
    t.allocator = allocator;
    t.flags = 0;
    t.name = name;
}
template <unsigned Address> void* GameData()
{
#if defined(_M_IX86)
    return reinterpret_cast<void*>(static_cast<uintptr_t>(Address));
#else
    return nullptr;
#endif
}
}

// 0x004A4BA0
ServiceValue* CopyValue(ServiceValue* destination, const ServiceValue* source)
{
    destination->key.first = source->key.first;
    destination->key.second = source->key.second;
    destination->key.last = source->key.last;
    CallGameMethod<void, 0x00408EB0>(&destination->payload.path, &source->payload.path);
    destination->payload.low = source->payload.low;
    destination->payload.high = source->payload.high;
    auto& links = destination->payload.links;
    links.count = source->payload.links.count;
    links.next = &links;
    links.previous = &links;
    // 5E1210 is callee-cleanup with four stack arguments. The method bridge supplies
    // exactly those arguments; its extra ECX value is unused by this static helper.
    CallGameMethod<void, 0x005E1210>(&links, &links, source->payload.links.next, &source->payload.links, uint8_t(0));
    return destination;
}
// 0x004A4CD0
ServiceValue* MakeValue(ServiceValue* destination, const ServiceKey* key, const ServicePayload* payload)
{
    destination->key.first = key->first;
    destination->key.second = key->second;
    destination->key.last = key->last;
    CallGameMethod<void, 0x00408EB0>(&destination->payload.path, &payload->path);
    destination->payload.low = payload->low;
    destination->payload.high = payload->high;
    auto& links = destination->payload.links;
    links.count = payload->links.count;
    links.next = &links;
    links.previous = &links;
    CallGameMethod<void, 0x005E1210>(&links, &links, payload->links.next, &payload->links, uint8_t(0));
    return destination;
}
// 0x004A5140
ServiceNodeBase** ServiceTree::Erase(ServiceNodeBase** out, ServiceNode* node)
{
    --count;
    auto* successor = CallGameFunction<ServiceNodeBase*, 0x004E0B20>(&node->base);
    CallGameFunction<void, 0x004E0E20>(&node->base, &anchor);
    CallGameMethod<void, 0x006CC920>(&node->value.payload.links);
    if (OwnsString(node->value.payload.path))
        CallGameFunction<void, 0x004E3F70>(node->value.payload.path.begin);
    CallGameVirtual<void, 3>(allocator, node, sizeof(ServiceNode));
    *out = successor;
    return out;
}
// 0x004A51A0
ServiceNodeBase** ServiceTree::InsertAt(ServiceNodeBase** out, ServiceNodeBase* position, const ServiceValue* value, uint8_t forceLeft)
{
    const int direction = forceLeft || position == &anchor || Less(value->key, reinterpret_cast<ServiceNode*>(position)->value.key) ? 0 : 1;
    auto* node = CallGameVirtual<ServiceNode*, 2>(allocator, sizeof(ServiceNode), name, flags);
    // EA checks node+16 rather than node, retaining its null-allocation bug.
    if (reinterpret_cast<uintptr_t>(node) != uintptr_t(0) - offsetof(ServiceNode, value))
        CopyValue(&node->value, value);
    CallGameFunction<void, 0x004E0C40>(&node->base, position, &anchor, direction);
    ++count;
    *out = &node->base;
    return out;
}
// 0x004A5410
ServiceInsertResult* ServiceTree::Insert(ServiceInsertResult* out, const ServiceValue* value, uintptr_t unused)
{
    (void)unused;
    auto* node = anchor.parent;
    auto* position = &anchor;
    bool left = true;
    while (node) {
        left = Less(value->key, reinterpret_cast<ServiceNode*>(node)->value.key);
        position = node;
        node = left ? node->left : node->right;
    }
    auto* predecessor = position;
    if (left) {
        if (position == anchor.left) {
            InsertAt(&out->node, position, value, 0);
            out->inserted = 1;
            return out;
        }
        predecessor = CallGameFunction<ServiceNodeBase*, 0x004E0B60>(position);
    }
    if (Less(reinterpret_cast<ServiceNode*>(predecessor)->value.key, value->key)) {
        InsertAt(&out->node, position, value, 0);
        out->inserted = 1;
    } else {
        out->node = predecessor;
        out->inserted = 0;
    }
    return out;
}
// 0x004A5500
ResourceMan* ResourceMan::Initialize(const wchar_t* source, void* coreAllocator)
{
    secondaryVtable = static_cast<void* const*>(GameData<0x00FEBF38>());
    vtable = static_cast<void* const*>(GameData<0x00FB21F8>());
    secondaryVtable = static_cast<void* const*>(GameData<0x00FB21D8>());
    InterlockedExchange(reinterpret_cast<volatile LONG*>(&references), 0);
    vtable = static_cast<void* const*>(GameData<0x00FB2420>());
    secondaryVtable = static_cast<void* const*>(GameData<0x00FB2410>());
    active = 0;
    allocator = coreAllocator ? coreAllocator : CallGameFunction<void*, 0x00931020>();
    flag20 = 0;
    state = nullptr;
    path.begin = static_cast<wchar_t*>(GameData<0x011C7E54>());
    path.end = path.begin;
    path.capacity = static_cast<wchar_t*>(GameData<0x011C7E56>());
    path.allocator = GameData<0x00FB22E4>();
    InitializeTree(entryTree, allocator, static_cast<const char*>(GameData<0x00FB23EC>()));
    InitializeTree(recordTree, allocator, static_cast<const char*>(GameData<0x00FB23C8>()));
    generation = unknown78 = unknown7C = 0;
    CallGameMethod<void, 0x004E17E0>(&mutex, 0, 1);
    if (source) CallGameMethod<void, 0x00408E20>(&path, source, source + std::wcslen(source));
    return this;
}
// 0x004A5620
uintptr_t ResourceMan::Destroy()
{
    vtable = static_cast<void* const*>(GameData<0x00FB2420>());
    secondaryVtable = static_cast<void* const*>(GameData<0x00FB2410>());
    if (state) CallGameMethod<void, 0x004A5240>(this);
    if (active) {
        active = 0;
        CallGameVirtual<void, 7>(this);
    }
    CallGameMethod<void, 0x004E1670>(&mutex);
    CallGameMethod<void, 0x004A4D40>(&recordTree, recordTree.anchor.parent);
    CallGameMethod<void, 0x004A4A10>(&entryTree, entryTree.anchor.parent);
    uintptr_t result = reinterpret_cast<uintptr_t>(path.begin);
    if (OwnsString(path)) result = CallGameFunction<uintptr_t, 0x004E3F70>(path.begin);
    secondaryVtable = static_cast<void* const*>(GameData<0x00FB21D8>());
    secondaryVtable = static_cast<void* const*>(GameData<0x00FA1B78>());
    vtable = static_cast<void* const*>(GameData<0x00FB84D0>());
    return result;
}
// 0x004A4F10
uint8_t ResourceMan::LoadDefinitions()
{
    if (!active) return 0;
    // Original stack slots at +0x580 and +0x788 are consecutive 260-word
    // SplitPathReverse outputs. Its SECOND output is the next search directory.
    // Keeping that alias is essential: a separate scratch buffer never updates it.
    wchar_t parentParts[2][260];
    auto& destination = parentParts[1];
    std::wcsncpy(destination, path.begin, 260);
    destination[259] = 0;
    wchar_t filename[260];
    for (;;) {
        CallGameFunction<void, 0x004EC6D0>(filename, destination, GameData<0x00FB2184>());
        if (CallGameFunction<uint8_t, 0x004DBBA0>(filename)) break;
        CallGameFunction<void, 0x004EC750>(filename, 8);
        if (resourcesystem::SplitPathReverse(reinterpret_cast<const char16_t*>(filename),
                reinterpret_cast<char16_t*>(parentParts), 2) != 2) return 0;
    }
    // Owned by the unreimplemented file-reader dependency. Its storage is only
    // passed to original methods; no embedded x86 pointers are interpreted here.
    alignas(8) unsigned char reader[0x638];
    CallGameMethod<void, 0x004DB970>(reader, filename);
    uint8_t result = 0;
    if (CallGameMethod<uint8_t, 0x004DB4C0>(reader, 1, 6, 1, 0)) {
        mutex.Lock();
        bool cleared = false;
        char line[256];
        uint32_t length = CallGameFunction<uint32_t, 0x004DD970>(reader, line, 256);
        while (length < 0xFFFFFFFEu) {
            if (length >= 12 && line[0] == '0' && (line[1] == 'x' || line[1] == 'X')) {
                line[10] = 0;
                const uint32_t key = static_cast<uint32_t>(std::strtoul(line + 2, nullptr, 16));
                wchar_t name[32];
                size_t i = 0;
                while (i < 31 && static_cast<int8_t>(line[i + 11]) > 32) {
                    name[i] = static_cast<uint8_t>(line[i + 11]);
                    ++i;
                }
                name[i] = 0;
                if (!cleared) {
                    CallGameMethod<void, 0x004A4A10>(&entryTree, entryTree.anchor.parent);
                    entryTree.anchor.right = &entryTree.anchor;
                    entryTree.anchor.left = &entryTree.anchor;
                    entryTree.anchor.parent = nullptr;
                    entryTree.anchor.color = 0;
                    entryTree.count = 0;
                    cleared = true;
                }
                CallGameVirtual<void, 20>(this, name, key);
            } else if (length >= 5 && line[0] == 'g' && line[2] == '0' && (line[3] == 'x' || line[3] == 'X')) {
                generation = static_cast<uint32_t>(std::strtoul(line + 4, nullptr, 16));
            }
            length = CallGameFunction<uint32_t, 0x004DD970>(reader, line, 256);
        }
        result = 1;
        mutex.Unlock();
    }
    CallGameMethod<void, 0x004DB9D0>(reader);
    return result;
}
// 0x004A6050
uintptr_t ResourceMan::AddRecord(const ServiceKey* key, const wchar_t* filename, const void* source)
{
    const uint64_t stamp = CallGameFunction<uint64_t, 0x004DBCF0>(filename, 2);
    ServicePayload payload;
    CallGameMethod<void, 0x004A4860>(&payload, source, static_cast<uint32_t>(stamp), static_cast<uint32_t>(stamp >> 32));
    ServiceValue value;
    MakeValue(&value, key, &payload);
    ServiceInsertResult inserted;
    recordTree.Insert(&inserted, &value, 0);
    CallGameMethod<void, 0x006CC920>(&value.payload.links);
    if (OwnsString(value.payload.path)) CallGameFunction<void, 0x004E3F70>(value.payload.path.begin);
    CallGameMethod<void, 0x006CC920>(&payload.links);
    uintptr_t result = reinterpret_cast<uintptr_t>(payload.path.begin);
    if (OwnsString(payload.path)) result = CallGameFunction<uintptr_t, 0x004E3F70>(payload.path.begin);
    return result;
}
// 0x004A6E00
uint8_t GetTimes(resource::DatabasePackedFile* self, uint32_t* created, uint32_t* modified)
{
    self->recordMutex.Lock();
    const bool ready = self->openFlags || self->EnsureOpen();
    if (ready) {
        created[0] = self->createdTime[0];
        created[1] = self->createdTime[1];
        modified[0] = self->modifiedTime[0];
        modified[1] = self->modifiedTime[1];
    } else {
        created[0] = 0;
        created[1] = 0;
        modified[0] = 0;
        modified[1] = 0;
    }
    self->recordMutex.Unlock();
    return static_cast<uint8_t>(ready);
}
// 0x004A6F10
uint8_t ExceedsFreeRatio(resource::DatabasePackedFile* self)
{
    self->recordMutex.Lock();
    bool result = false;
    if (self->openRecords.size() == 0 && (self->openFlags || self->EnsureOpen())) {
        const int32_t denominator = self->freeSpace.total;
        const int32_t numerator = CallGameMethod<int32_t, 0x004AA4E0>(&self->freeSpace);
        float threshold;
        std::memcpy(&threshold, self->unknown340, sizeof(threshold));
        result = static_cast<float>(numerator) / static_cast<float>(denominator) > threshold;
    }
    self->recordMutex.Unlock();
    return static_cast<uint8_t>(result);
}
// 0x004A7030, implementation already verified in the packed-file module.
uint8_t ChooseWriteCodec(resource::DatabasePackedFile* self, const void* source, uint32_t size, void** output, uint32_t* outputSize, uint16_t* type)
{
    return static_cast<uint8_t>(self->ChooseWriteCodec(source, size, output, outputSize, type));
}
}
