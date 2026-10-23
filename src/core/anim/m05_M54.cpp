#include "ts3/anim/m05_M54.h"
#include "ts3/layout.h"

namespace ts3::anim {
void* __stdcall Scratch570A10(void* context, void* storage)
{
    if (storage == nullptr) return nullptr;
    return CallGameMethod<void*, 0x0054DDB0>(storage, context);
}

void* __stdcall Scratch570A40(void* context, void* storage)
{
    if (storage == nullptr) return nullptr;
    return CallGameMethod<void*, 0x0055DED0>(storage, context);
}

void* __stdcall Scratch570A60(void* context, void* storage)
{
    if (storage == nullptr) return nullptr;
    return CallGameMethod<void*, 0x0054EE20>(storage, context);
}

void* __stdcall Scratch570A80(void* context, void* storage)
{
    if (storage == nullptr) return nullptr;
    return CallGameMethod<void*, 0x0054CE90>(storage, context);
}

void* __stdcall Scratch570AC0(void* context, void* storage)
{
    if (storage == nullptr) return nullptr;
    return CallGameMethod<void*, 0x0054CF40>(storage, context);
}

void* __stdcall Scratch570AE0(void* context, void* storage)
{
    if (storage == nullptr) return nullptr;
    return CallGameMethod<void*, 0x0054CFA0>(storage, context);
}

}
