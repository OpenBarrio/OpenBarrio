#include "ts3/animation/cls_7f1620_Animation.h"

#include <cstring>

namespace ts3::animation {

// Original 0x007F1620: lea eax,[ecx+0x20]; ret.
float* ClipBinding::GetMatrix()
{
    return matrix;
}

// Original 0x00951480.
ClipBinding* ClipBinding::Destroy(uint32_t deleteFlags)
{
    vtableAddress = 0x0104415C;
    CallGameMethod<void, 0x0080D810>(this);  // base destructor: vtable 0x1020A30, +0x14 = 0
    if (deleteFlags & 1)
        CallGameFunction<void, 0x004E3F70>(static_cast<void*>(this));  // free
    return this;
}

// Original 0x007FE1F0.
ClipPatternSet* ClipPatternSet::Initialize()
{
    CallGameMethod<void, 0x007FE180>(this);
    unk64 = unk68 = unk6c = unk70 = 0.0f;
    unk58 = 10000;
    unk5c = 10000;
    unk50 = 1;
    unk54 = -1;
    unk60 = 1;
    unk74 = 1.0f;  // dword at 0x0107A538
    unk78 = 0;
    vtableAddress = 0x0101EFE0;
    for (auto& z : zero80)
        z = 0;
    for (auto& p : patterns) {
        p.d = 0x0101F088;  // "SACS/ClipPattern"
        p.a = 0x011C7E54;  // empty string
        p.b = 0x011C7E54;
        p.c = 0x011C7E56;
    }
    for (auto& z : zeroC0)
        z = 0;
    return this;
}

// Original 0x00806240 (`ret 8`).
// EAX on return: the growth helper's result (0x00805D80), the store helper's result
// (0x00805990) or 0 when the end slot address was null (EAX still held the null slot).
uint32_t ClipList::PushBack(const ClipEntry* entry, void* arg)
{
    if (entry->owner)
        CallGameMethod<void, 0x00566170>(static_cast<uint8_t*>(entry->owner) + 0xC, this, arg);
    uint8_t* slot = end;
    if (slot >= capacity)
        return CallGameMethod<uint32_t, 0x00805D80>(reinterpret_cast<uint8_t*>(this) + 4, slot, entry);
    // The original advances `end` *before* testing the slot for null, so a null end pointer
    // still leaves the list pointing at 0x18 (kept as-is).
    end = slot + 0x18;
    uint32_t result = 0;  // EAX held the null slot on that path
    if (slot)
        result = CallGameMethod<uint32_t, 0x00805990>(slot, entry);
    return result;
}

// Original 0x0093EBE0 (`ret 4`).
void* ClipControllerHost::CreateController(const ClipControllerParams* params)
{
    CallGameVirtual<void, 12>(this);
    void* controller = nullptr;
    // Without a source, or with a source that holds no clips, the original returns EAX = 0
    // right after the vtable call: every later step (and the returned controller) is skipped.
    if (source && source->clips) {
        void* mem = CallGameFunction<void*, 0x004E3F90>(0x600, "Animation/ClipController", 0, 0, 0, 0);
        if (mem)
            controller = CallGameMethod<void*, 0x00568820>(mem, static_cast<void*>(controllerSlot), 0.0f);
        // The original keeps calling through a null controller if allocation failed.
        void* resource = params->resource;
        if (!resource) {
            void* manager = *reinterpret_cast<void* const*>(uintptr_t{0x011CD800});
            resource = CallGameMethod<void*, 0x005719D0>(manager);
        }
        CallGameMethod<void, 0x00569C50>(controller, resource);
        CallGameMethod<void, 0x00569360>(controller, params->mode);
        CallGameMethod<void, 0x005694F0>(controller, params);
        CallGameMethod<void, 0x00569330>(controller, static_cast<uint32_t>(params->byte1d8));
        CallGameMethod<void, 0x00568200>(controller, static_cast<uint32_t>(params->byte1d9));
        if (params->extra)
            CallGameMethod<void, 0x00568330>(controller, params->extra);
        if (params->byte1da)
            CallGameMethod<void, 0x005681E0>(controller, 0u);
    }
    return controller;
}

// Original 0x00953170 (`ret 4`).
ClipBinding* ClipBindingOwner::AddBinding(void* target)
{
    ClipBinding* result = nullptr;  // EAX = 0 on every bail-out path
    if (source && target) {
        int32_t index = CallGameMethod<int32_t, 0x00575B00>(static_cast<void*>(source->lookup), target);
        if (index >= 0) {
            ClipBinding* b = CallGameMethod<ClipBinding*, 0x004E8530>(reinterpret_cast<void*>(uintptr_t{0x011E9118}));
            if (b) {
                b->target = nullptr;
                b->flags = 0;
                b->unk0c = 0;
                b->target2 = nullptr;
                b->unk14 = 0;
                b->unk1c = 0;
                b->unk18 = 1;
                b->vtableAddress = 0x0104415C;
            }
            // EA bug kept: a failed allocation (null) is dereferenced here and faults.
            b->flags |= 2;
            b->target2 = target;
            b->target = target;
            alignas(16) float m[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
            const uint8_t* transform = source->transforms + static_cast<size_t>(index) * 48;
            ClipBinding* slotValue = b;
            CallGameMethod<void, 0x00577160>(const_cast<uint8_t*>(transform), m);
            std::memcpy(b->matrix, m, sizeof m);
            if (end < capacity) {
                ClipBinding** slot = end++;
                if (slot)
                    *slot = b;
            } else {
                CallGameMethod<void, 0x008621C0>(static_cast<void*>(&unk54), end, &slotValue);
            }
            // The original re-reads the slot it just wrote: end[-1] (and it faults when end was
            // null, because end[-1] then reads 0xFFFFFFFD).
            result = end[-1];
        }
    }
    return result;
}

}  // namespace ts3::animation
