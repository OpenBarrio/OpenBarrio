#include "ts3/anim/m05_M56.h"
#include "ts3/layout.h"
namespace ts3::anim::m56 {
void* __stdcall CreateCD0(void* context, void* storage) {
    if (!storage) return nullptr;
    return CallGameMethod<void*, 0x005551D0>(storage, context);
}
void* __stdcall CreateCF0(void* context, void* storage) {
    if (!storage) return nullptr;
    return CallGameMethod<void*, 0x0055BC90>(storage, context);
}
void* __stdcall CreateD10(void* context, void* storage) {
    if (!storage) return nullptr;
    return CallGameMethod<void*, 0x0055BFB0>(storage, context);
}
void* __stdcall CreateD30(void* context, void* storage) {
    if (!storage) return nullptr;
    return CallGameMethod<void*, 0x0055D3B0>(storage, context);
}
void* __stdcall CreateD60(void* context, void* storage) {
    if (!storage) return nullptr;
    return CallGameMethod<void*, 0x00561B70>(storage, context);
}
// Start from verified m01::FileType; only the immediate differs.
uint32_t __fastcall Size(void*, void*) { return 0x34u; }
}
