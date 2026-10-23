#include "ts3/cas/cls_6b1bb0_CAS.h"

#include "ts3/layout.h"

namespace ts3::cas {
namespace {

void* CASGlobal()
{
#if defined(_M_IX86)
    // 0x011CECDC is the original image's CAS lookup singleton.
    const auto* slot = reinterpret_cast<const volatile uint32_t*>(uintptr_t{0x011CECDC});
    return reinterpret_cast<void*>(static_cast<uintptr_t>(*slot));
#else
    AbortNotRewritten("CAS lookup singleton");
    return nullptr;
#endif
}

int32_t ResolveProperty(uint32_t key, uint32_t method)
{
    void* entry = nullptr;
    if (method == 0x00617500) {
        entry = CallGameMethod<void*, 0x00617500>(CASGlobal(), key);
    } else {
        entry = CallGameMethod<void*, 0x00617540>(CASGlobal(), key);
    }
    if (!entry)
        return 0;
    return *static_cast<const int32_t*>(entry);
}

}  // namespace

// Original 0x006B1BB0. The game helper has `thiscall`, one stack key, and ret 4.
int32_t CASObject::GetFirstProperty() const
{
    return ResolveProperty(0x2FCD2B4C, 0x00617540);
}

// Original 0x006B1BD0. The game helper has `thiscall`, one stack key, and ret 4.
int32_t CASObject::GetSecondProperty() const
{
    return ResolveProperty(0x36535688, 0x00617500);
}

// Original 0x006B4220. Base initialization is outside this unit and remains bridged.
CASObject* CASObject::Initialize()
{
    CallGameMethod<void, 0x006B4050>(this);
    constexpr uint32_t kVtableAddress = 0x00FF3DB0;
    vtableAddress = kVtableAddress;
    return this;
}

}  // namespace ts3::cas
