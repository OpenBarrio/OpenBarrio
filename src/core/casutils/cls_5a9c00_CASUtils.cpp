#include "ts3/casutils/cls_5a9c00_CASUtils.h"

#include "ts3/layout.h"
#include <intrin.h>

namespace ts3::casutils {
namespace {
constexpr uint32_t kCallbackInitialVtable = 0x00FDE818;
constexpr uint32_t kCallbackFinalVtable = 0x00FDE844;
}

// TS3W.exe 0x005A9C00. XCHG with zero is intentionally atomic on the original.
CASSwapReadyCallback* ConstructCASSwapReadyCallback(CASSwapReadyCallback* self,
                                                     int32_t modelId, int32_t outfitId,
                                                     int32_t userData)
{
    self->vtable = kCallbackInitialVtable;
    _InterlockedExchange(reinterpret_cast<volatile long*>(&self->refCount), 0);
    self->userData = userData;
    self->vtable = kCallbackFinalVtable;
    self->modelId = modelId;
    self->outfitId = outfitId;
    return self;
}

// TS3W.exe 0x005A9D60. Calls into the game's object manager, callback allocator,
// callback vtables and script dispatcher exactly at the original call boundaries.
void SetModelInGameObjectImpl(int32_t objectId, int32_t modelId, int32_t outfitId,
                              int32_t userData)
{
    if (!objectId)
        return;

    void* manager = ts3::CallGameFunction<void*, 0x007692B0>();
    void* model = ts3::CallGameVirtual<void*, 51>(manager, objectId);

    struct ScriptEvent {
        int32_t type;
        uint32_t vtable;
        int32_t modelId;
        int32_t outfitId;
        void* model;
        void* reserved;
        void* callback;
    } event{-2051284107, 0x0115586C, modelId, outfitId, nullptr, nullptr, nullptr};
    if (model) {
        ts3::CallGameVirtual<void, 0>(model);
        event.model = model;
    }

    void* callback = ts3::CallGameFunction<void*, 0x004E3F90>(
        24, reinterpret_cast<const char*>(static_cast<uintptr_t>(0x00FDE85C)), 0);
    if (callback) {
        auto* record = static_cast<CASSwapReadyCallback*>(callback);
        ConstructCASSwapReadyCallback(record, modelId, outfitId, userData);
        ts3::CallGameVirtual<void, 0>(callback);
    }
    event.callback = callback;
    // The original invokes slot 0 once when it creates the callback and again when
    // replacing the (initially null) callback held in its local event state.
    if (callback)
        ts3::CallGameVirtual<void, 0>(callback);
    ts3::CallGameFunction<void, 0x00692F70>(&event);
    if (callback)
        ts3::CallGameVirtual<void, 1>(callback);
    // The event's second field is a process-wide listener object, not the callback.
    ts3::CallGameVirtual<void, 0>(reinterpret_cast<void*>(static_cast<uintptr_t>(event.vtable)),
                                  &event.model);
}

}  // namespace ts3::casutils

extern "C" ts3::casutils::CASSwapReadyCallback* RewriteCASUtilsCallbackConstruct(
    ts3::casutils::CASSwapReadyCallback* self, int32_t modelId, int32_t outfitId,
    int32_t userData)
{
    return ts3::casutils::ConstructCASSwapReadyCallback(self, modelId, outfitId, userData);
}

extern "C" void RewriteCASUtilsSetModel(int32_t objectId, int32_t modelId,
                                        int32_t outfitId, int32_t userData)
{
    ts3::casutils::SetModelInGameObjectImpl(objectId, modelId, outfitId, userData);
}
