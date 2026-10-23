#pragma once

#include <cstdint>

namespace ts3::casutils {

// TS3W.exe 0x005A9C00, __thiscall, ret 0x0c. Initializes the callback record.
struct CASSwapReadyCallback {
    uint32_t vtable;
    int32_t refCount;
    int32_t userData;
    uint32_t secondaryVtable;
    int32_t modelId;
    int32_t outfitId;
};

CASSwapReadyCallback* ConstructCASSwapReadyCallback(CASSwapReadyCallback* self,
                                                     int32_t modelId, int32_t outfitId,
                                                     int32_t userData);

// TS3W.exe 0x005A9D60, __cdecl, ret 0. Script bridge; non-null calls use live game state.
void SetModelInGameObjectImpl(int32_t objectId, int32_t modelId, int32_t outfitId,
                              int32_t userData);

}  // namespace ts3::casutils
