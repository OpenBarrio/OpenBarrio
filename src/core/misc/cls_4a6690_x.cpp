// Rewrites of DatabasePackedFile's stream / index / resource-manager vtable methods.
// See ts3/misc/cls_4a6690_x.h for the per-function contracts.
#include "ts3/misc/cls_4a6690_x.h"

#include "ts3/services/cls_4a4ba0_Services.h"

namespace ts3::misc {

void* PackedFileLifecycle::SetIndexIfClosed(void* newIndex)
{
    void* old = index;
    if (!openFlags)
        index = static_cast<resource::IPackageIndex*>(newIndex);
    return old;
}

void* PackedFileLifecycle::GetIndex()
{
    return index;
}

io::IStream* PackedFileLifecycle::GetStream()
{
    return stream;
}

bool PackedFileLifecycle::SetStream(io::IStream* newStream)
{
    if (openFlags)
        return false;
    io::IStream* old = stream;
    if (newStream != old) {
        if (newStream)
            newStream->AddRef();
        stream = newStream;
        if (old)
            old->Release();
    }
    return true;
}

bool PackedFileLifecycle::SetMemoryImage(void* data, uint32_t size)
{
    if (openFlags)
        return false;
    wchar_t* begin = const_cast<wchar_t*>(path);
    auto* end = reinterpret_cast<wchar_t**>(unknown028);
    if (begin != *end) {
        *begin = 0;
        *end = begin;
    }
    memorySize = size;  // the original stores the size before the data pointer
    memoryData = static_cast<const uint8_t*>(data);
    return true;
}

uint64_t PackedFileLifecycle::ModifiedTime(int32_t)
{
    // 0x004A6E90 hands 0x004A6E00 the created output at frame+0x10 and the modified one at
    // frame+0x00, then returns the doubleword pair at frame+0x00/+0x04: the modified time.
    // GetTimes writes both outputs on every path and the created slots are never read here,
    // so they need no initial value.
    uint32_t created[2];
    uint32_t modified[2];
    services::GetTimes(this, created, modified);
    return static_cast<uint64_t>(modified[0]) | (static_cast<uint64_t>(modified[1]) << 32);
}

uint32_t PackedFileLifecycle::CountOpenRecords(const resource::ResourceKey* key)
{
    uint32_t count = 0;
    recordMutex.Lock();
    if (openFlags) {
        for (auto it = openRecords.find(*key); it != openRecords.end(); ++it) {
            if (it->first.instanceLow != key->instanceLow || it->first.instanceHigh != key->instanceHigh ||
                it->first.type != key->type || it->first.group != key->group)
                break;
            ++count;
        }
    }
    recordMutex.Unlock();
    return count;
}

bool PackedFileLifecycle::AttachResourceManager(bool attach, resource::IResourceManager* manager, bool force)
{
    if (!attach) {
        if (manager && manager != resourceManager)
            return false;
        if (force) {
            resourceManager = nullptr;
            return true;
        }
        return CallGameVirtual<bool, 13>(resourceManager, 0u, this, 0u);
    }
    if (resourceManager && manager != resourceManager)
        CallGameVirtual<bool, 18>(this, false, resourceManager, force);  // detach the old one; result ignored
    if (!manager)
        return false;
    if (!force)
        return CallGameVirtual<bool, 13>(manager, 1u, this, 0u);
    resourceManager = manager;
    return true;
}

resource::IPackageIndex* PackedFileLifecycle::LoadIndexImage(const void* image, uint32_t size, bool store, uint32_t kind)
{
    resource::IPackageIndex* created = CallGameVirtual<resource::IPackageIndex*, 25>(this, kind);
    if (created) {
        if (created->Load(image, size, 1)) {
            if (store)
                CallGameVirtual<void, 23>(this, created);
        }
        return created;
    }
    return nullptr;
}

}  // namespace ts3::misc