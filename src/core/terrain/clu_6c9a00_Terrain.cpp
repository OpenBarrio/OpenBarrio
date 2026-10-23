// TS3W.exe 0x006C9A00, 0x006C9B60, 0x006CF0E0.
#include "ts3/terrain/clu_6c9a00_Terrain.h"
namespace ts3::terrain {
namespace {
void* Manager() {
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(0x011D3C84);
#else
    return nullptr;
#endif
}
}
// All known callers ignore EAX; scratch empty-list EAX becomes zero.
int32_t TerrainCluster::VisitNodes() {
    int32_t result = 0;
    void* const sentinel = &head4C;
    void* link = head4C;
    while (link != sentinel) {
        auto* node = link ? reinterpret_cast<TerrainCluster*>(static_cast<uint8_t*>(link) - offsetof(TerrainCluster, next)) : nullptr;
        result = node->UpdateNode();
        link = *static_cast<void**>(link);
    }
    return result;
}
// Equality-only end test keeps EA's ragged-end scan behavior.
void* TerrainCluster::FindEntry(const char* name) {
    const uint32_t hash = CallGameFunction<uint32_t, 0x007CFE70>(name);
    uint8_t* entry = entries64;
    uint8_t* const end = entries68;
    while (entry != end) {
        if (*reinterpret_cast<const uint32_t*>(entry) == hash) return entry;
        entry += 0x20;
    }
    return nullptr;
}
// Thiscall callees pop their arguments: there is no stack bug. Inactive-refcount
// tail EAX is scratch, and VisitNodes' callers ignore it.
int32_t TerrainCluster::UpdateNode() {
    if (void* manager = Manager()) {
        CallGameVirtual<void, 16>(manager, &field24);
        if ((manager = Manager()) != nullptr) {
            CallGameVirtual<void, 16>(manager, &field2C);
            if ((manager = Manager()) != nullptr) CallGameVirtual<void, 16>(manager, &field30);
        }
    }
    if ((flags42 & 1) != 0) {
        void* tool = CallGameFunction<void*, 0x006079C0>();
        void* value = (reinterpret_cast<uintptr_t>(field2C) & 3) ? nullptr : field2C;
        CallGameMethod<void, 0x006CE970>(tool, value);
    }
    return CallGameMethod<int32_t, 0x006CF0A0>(this);
}
}
