// Reimplementation of TS3W.exe 0x00774D50, 0x00774E30 and 0x00774E90.
#include "ts3/casutils/clu_774d50_CASUtils.h"

#include "ts3/layout.h"

namespace ts3::casutils {
namespace {

constexpr uint32_t kObjectCacheId = 0x011D6838;
constexpr uint32_t kObjectCachePointer = 0x011D683C;

uint32_t& CachedId()
{
#if defined(_M_IX86)
    return *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(kObjectCacheId));
#else
    static uint32_t id = 0;
    return id;
#endif
}

uint32_t& CachedPointer()
{
#if defined(_M_IX86)
    return *reinterpret_cast<uint32_t*>(static_cast<uintptr_t>(kObjectCachePointer));
#else
    static uint32_t pointer = 0;
    return pointer;
#endif
}

}  // namespace

// TS3W.exe 0x00774D50. The size-4 arm returns AL from the destination pointer; the
// size-3 arm returns the high byte of value. Those are EA's actual register effects.
uint8_t StoreBytes774D50(void* destination, uint32_t value, int32_t size)
{
    auto* out = static_cast<uint8_t*>(destination);
    switch (size) {
    case 1:
        out[0] = static_cast<uint8_t>(value);
        return static_cast<uint8_t>(value);
    case 2:
        out[0] = static_cast<uint8_t>(value);
        out[1] = static_cast<uint8_t>(value >> 8);
        return static_cast<uint8_t>(size);
    case 3:
        out[0] = static_cast<uint8_t>(value);
        out[1] = static_cast<uint8_t>(value >> 8);
        out[2] = static_cast<uint8_t>(value >> 16);
        return static_cast<uint8_t>(value >> 16);
    case 4:
        out[0] = static_cast<uint8_t>(value);
        out[1] = static_cast<uint8_t>(value >> 8);
        out[2] = static_cast<uint8_t>(value >> 16);
        out[3] = static_cast<uint8_t>(value >> 24);
        return static_cast<uint8_t>(reinterpret_cast<uintptr_t>(destination));
    default:
        return static_cast<uint8_t>(size);
    }
}

// TS3W.exe 0x00774E30. Cache fields are executable globals on x86; x64 keeps an isolated
// fallback because there is no game image to call.
uint32_t ResolveCasObject774E30(uint32_t id)
{
    if (id == CachedId()) {
        CachedId() = 0;
        CachedPointer() = 0;
    }
    uint32_t result = id & 0xFF000000u;
    if ((result & 0xFF000000u) != 0x60000000u)
        return result;
    void* manager = ts3::CallGameFunction<void*, 0x007692B0>();
    void* object = ts3::CallGameVirtual<void*, 54>(manager, id);
    if (object)
        return ts3::CallGameVirtual<uint32_t, 2>(object);
    return 0;
}

// TS3W.exe 0x00774E90.
int32_t SetCasProperty774E90(uint32_t id, uint32_t value, int32_t size)
{
    uint32_t object = 0;
    if (id == CachedId()) {
        object = CachedPointer();
    } else if ((id & 0xFF000000u) == 0x60000000u) {
        void* manager = ts3::CallGameFunction<void*, 0x007692B0>();
        object = reinterpret_cast<uintptr_t>(ts3::CallGameVirtual<void*, 51>(manager, id));
        if (object) {
            CachedPointer() = object;
            CachedId() = id;
        }
    }
    if (!object)
        return -2;
    if (size > 4)
        return -7;
    uint32_t local = id;
    StoreBytes774D50(&local, value, size);
    const uint32_t accepted = ts3::CallGameVirtual<uint32_t, 14>(reinterpret_cast<void*>(object), &local, size);
    return static_cast<uint8_t>(accepted) ? 0 : -3;
}

}  // namespace ts3::casutils
