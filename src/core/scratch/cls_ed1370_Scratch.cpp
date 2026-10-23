// Rewrites for the Scratch decoder lifecycle at 0x00ED1370, 0x00ED79D0 and 0x00EEDF90.
#include "ts3/scratch/cls_ed1370_Scratch.h"

#include <cstring>

#include "ts3/allocator/core_allocator.h"
#if !defined(_M_IX86)
#include "ts3/platform/core_allocator.h"
#endif

namespace ts3::scratch {
namespace {

allocator::ICoreAllocator* DecoderAllocator()
{
#if defined(_M_IX86)
    // 0x00ED79D0
    auto* manager = *reinterpret_cast<uint8_t**>(uintptr_t(0x011FFC28));
    return *reinterpret_cast<allocator::ICoreAllocator**>(manager + 0x14);
#else
    return platform::CoreAllocator();
#endif
}

void* AllocateDecoder(size_t size, unsigned align)
{
    auto* allocator = DecoderAllocator();
#if defined(_M_IX86)
    // The game's slot 1 is Alloc(size, name, flags, align, alignOffset).
    return CallGameVirtual<void*, 1>(allocator, size, static_cast<const char*>(nullptr), 1u,
                                     align, 0u);
#else
    return allocator->Alloc(size, nullptr, 0u, align, 0u);
#endif
}

void FreeDecoder(void* block)
{
    if (!block) return;
#if defined(_M_IX86)
    // 0x00ED7230 is the allocator's release helper; retain its exact game call order.
    auto* manager = *reinterpret_cast<uint8_t**>(uintptr_t(0x011FFC28));
    CallGameMethod<void, 0x00ED7230>(manager, block, 0u);
#else
    auto* allocator = DecoderAllocator();
    allocator->Free(block, 0);
#endif
}

} // namespace

ScratchDecoderBase* ScratchDecoderBase::Construct(uint8_t flags)
{
    // 0x00ED1370
    *reinterpret_cast<uint32_t*>(bytes) = 0x010F61A0;
    if (flags & 1u)
        CallGameFunction<int32_t, 0x004E3F70>(static_cast<void*>(this));
    return this;
}

void* __stdcall CreateScratchDecoder(ScratchDecoderOps* ops, uint32_t argument,
                                      uint32_t blockCount, ScratchDecoderContext* context)
{
    // 0x00ED79D0: preserve the game's 32-bit size arithmetic and its two alignment stages.
    uint32_t descriptorCount = 0;
    const uint32_t dataSize = ops->decodedSize(argument, &descriptorCount);
    const uint32_t descriptorBytes = blockCount * 20u;
    uint32_t total = descriptorBytes + ((dataSize + 7u) & ~7u);
    const bool hasBlocks = ops->blockWidth != 0;
    uint32_t alignmentArgument = descriptorCount;
    if (hasBlocks) {
        total = ((total + 15u) & ~15u) + 20u;
        if (descriptorCount <= 16u) alignmentArgument = 16u;
    }
    const uint32_t allocationSize = total ? total : 52u;
    auto* object = static_cast<uint8_t*>(AllocateDecoder(allocationSize, alignmentArgument));
    if (!object) return nullptr;

    *reinterpret_cast<uint32_t*>(object) = 0x010F61A0;
    *reinterpret_cast<uint32_t*>(object + 4) = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(context));
    *reinterpret_cast<uint32_t*>(object + 12) = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ops->vtable));
    *reinterpret_cast<uint32_t*>(object + 16) = 0;
    *reinterpret_cast<uint32_t*>(object + 20) = ops->state;
    object[46] = static_cast<uint8_t>(argument);

    if (!ops->initialize(reinterpret_cast<ScratchDecoderBase*>(object))) {
        CallGameMethod<int32_t, 0x00ED7990>(object);
        return nullptr;
    }

    *reinterpret_cast<uint32_t*>(object + 8) = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(object));
    *reinterpret_cast<uint32_t*>(object + 24) = ops->state14;
    *reinterpret_cast<uint32_t*>(object + 32) = total;
    *reinterpret_cast<uint32_t*>(object + 28) = 0;
    object[50] = static_cast<uint8_t>(blockCount);
    *reinterpret_cast<uint16_t*>(object + 44) = 0;
    object[47] = object[48] = 0;
    object[49] = 0;
    object[51] = hasBlocks;

    // Embedded decoder block list begins at align8(object + dataSize).
    const uintptr_t blocksAddress = (reinterpret_cast<uintptr_t>(object) + dataSize + 7u) & ~uintptr_t(7u);
    auto* blocks = reinterpret_cast<uint8_t*>(blocksAddress);
    *reinterpret_cast<uint32_t*>(object + 36) = static_cast<uint32_t>(blocks - object);
    *reinterpret_cast<uint32_t*>(object + 40) = 0;

    if (hasBlocks) {
        const uintptr_t storageAddress = (blocksAddress + descriptorBytes + 15u) & ~uintptr_t(15u);
        const uint32_t storageBytes = 4u * argument * ops->blockWidth;
        auto* storage = static_cast<uint8_t*>(context->allocator
            ? context->allocator : DecoderAllocator());
        void* data = nullptr;
#if defined(_M_IX86)
        data = CallGameVirtual<void*, 1>(storage, storageBytes, "Decoder block storage", 1u, 128u, 0u);
#else
        data = reinterpret_cast<allocator::ICoreAllocator*>(storage)->Alloc(
            storageBytes, "Decoder block storage", 1u, 128u, 0u);
#endif
        *reinterpret_cast<uint32_t*>(object + 16) = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(data));
        if (!data) {
            CallGameMethod<int32_t, 0x00ED7990>(object);
            return nullptr;
        }
        *reinterpret_cast<uint16_t*>(reinterpret_cast<void*>(storageAddress + 14)) = ops->blockWidth;
        *reinterpret_cast<uint32_t*>(reinterpret_cast<void*>(storageAddress)) =
            static_cast<uint32_t>(reinterpret_cast<uintptr_t>(context));
        *reinterpret_cast<uint16_t*>(reinterpret_cast<void*>(storageAddress + 12)) = 0;
        *reinterpret_cast<uint8_t*>(reinterpret_cast<void*>(storageAddress + 16)) = static_cast<uint8_t>(argument);
        *reinterpret_cast<uint32_t*>(reinterpret_cast<void*>(storageAddress + 4)) =
            static_cast<uint32_t>(reinterpret_cast<uintptr_t>(data));
        *reinterpret_cast<uint32_t*>(object + 40) = static_cast<uint32_t>(storageAddress - reinterpret_cast<uintptr_t>(object));
    }

    // EA initializes only the first and fourth dwords of each 20-byte descriptor.
    for (uint32_t i = 0; i < object[50]; ++i) {
        auto* row = blocks + 20u * i;
        *reinterpret_cast<uint32_t*>(row) = 0;
        *reinterpret_cast<uint32_t*>(row + 12) = 0;
    }
    return object;
}

ScratchDecoderDerived* ScratchDecoderDerived::Destroy(uint32_t flags)
{
    // 0x00EEDF90 is a destructor thunk omitted from IDA's function table. It frees the
    // two owned allocations, updates the vtable between each release, then calls base.
    uint32_t buffer = *reinterpret_cast<uint32_t*>(bytes + 0x2c0);
    *reinterpret_cast<uint32_t*>(bytes) = 0x010FD544;
    if (buffer) FreeDecoder(reinterpret_cast<void*>(static_cast<uintptr_t>(buffer)));
    buffer = *reinterpret_cast<uint32_t*>(bytes + 0x48);
    *reinterpret_cast<uint32_t*>(bytes) = 0x010FD4A8;
    if (buffer) FreeDecoder(reinterpret_cast<void*>(static_cast<uintptr_t>(buffer)));
    if (flags & 1u) CallGameFunction<int32_t, 0x004E3F70>(static_cast<void*>(this));
    return this;
}

} // namespace ts3::scratch
