#include "ts3/io/memory_stream.h"
#include "ts3/platform/core_allocator.h"

#include <cstring>

namespace ts3::io {

namespace {

constexpr const char* kPtrName = "Services/EAIO/EAStreamMemory/ptr";
constexpr const char* kDataName = "Services/EAIO/EAStreamMemory/data";

}  // namespace

void* EaNew(size_t size, allocator::ICoreAllocator* allocator, const char* name)
{
    if (!allocator)
        allocator = DefaultAllocator();
    auto* block = static_cast<uint8_t*>(allocator->Alloc(size + 16, name, 0, 4, 16));
    if (!block)
        return nullptr;
    *reinterpret_cast<allocator::ICoreAllocator**>(block) = allocator;
    return block + 16;
}

void EaDelete(void* object)
{
    if (!object)
        return;
    auto* block = static_cast<uint8_t*>(object) - 16;
    (*reinterpret_cast<allocator::ICoreAllocator**>(block))->Free(block, 0);
}

allocator::ICoreAllocator* DefaultAllocatorDirect()
{
#if defined(_M_IX86)
    return CallGameFunction<allocator::ICoreAllocator*, 0x00931020>();
#else
    return platform::CoreAllocator();
#endif
}

allocator::ICoreAllocator* DefaultAllocator()
{
#if defined(_M_IX86)
    return CallGameFunction<allocator::ICoreAllocator*, 0x004D9AD0>();
#else
    return platform::CoreAllocator();
#endif
}

SharedBuffer* SharedBuffer::Create(uint32_t capacity, allocator::ICoreAllocator* allocator, const char* streamName)
{
    if (!allocator)
        allocator = DefaultAllocator();
    void* memory = EaNew(sizeof(SharedBuffer), allocator, streamName ? streamName : kPtrName);
    if (!memory)
        return nullptr;
    auto* buffer = static_cast<SharedBuffer*>(memory);
#if defined(_M_IX86)
    buffer->vtable = reinterpret_cast<void* const*>(static_cast<uintptr_t>(0x00FB84D0));
#else
    buffer->vtable = nullptr;
#endif
    buffer->allocator = allocator;
    buffer->data = static_cast<uint8_t*>(allocator->Alloc(capacity, streamName ? streamName : kDataName, 0));
    buffer->refCount = 0;
    buffer->ownsData = true;
    return buffer;
}

int32_t SharedBuffer::Release()
{
    if (refCount > 1)
        return --refCount;
    if (ownsData)
        allocator->Free(data, 0);
#if defined(_M_IX86)
    CallGameVirtual<void*, 0>(this, 1);  // the game's deleting destructor: frees through EaNew's header
#else
    auto* block = reinterpret_cast<uint8_t*>(this) - 16;
    (*reinterpret_cast<allocator::ICoreAllocator**>(block))->Free(block, 0);
#endif
    return 0;
}

bool MemoryStream::Realloc(uint32_t newCapacity)
{
    allocator::ICoreAllocator* allocator = buffer && buffer->allocator ? buffer->allocator : DefaultAllocator();
    SharedBuffer* fresh = nullptr;
    if (newCapacity) {
        fresh = SharedBuffer::Create(newCapacity, allocator, name);
        if (!fresh)
            return false;
        ++fresh->refCount;
        if (!fresh->data) {
            fresh->Release();
            return false;
        }
    }
    if (buffer) {
        if (fresh)
            std::memcpy(fresh->data, buffer->data, newCapacity < capacity ? newCapacity : capacity);
        buffer->Release();
    }
    buffer = fresh;
    capacity = newCapacity;
    return true;
}

void MemoryStream::Construct(const void* src, uint32_t dataSize, bool usePointer, bool freePointer,
                             allocator::ICoreAllocator* dataAllocator, const char* allocationName)
{
    name = allocationName;
#if defined(_M_IX86)
    vtable = reinterpret_cast<void* const*>(static_cast<uintptr_t>(0x00FB5BCC));
#else
    vtable = nullptr;
#endif
    buffer = nullptr;
    refCount = 0;
    size = 0;
    capacity = 0;
    position = 0;
    resizable = false;
    growthFactor = 1.5f;
    growthIncrement = 0;
    if (src || dataSize)
        SetData(src, dataSize, usePointer, freePointer, dataAllocator);
}

void MemoryStream::ConstructCopy(const MemoryStream& other)
{
#if defined(_M_IX86)
    vtable = reinterpret_cast<void* const*>(static_cast<uintptr_t>(0x00FB5BCC));
#else
    vtable = nullptr;
#endif
    buffer = other.buffer;
    name = other.name;
    refCount = 0;
    size = other.size;
    capacity = other.size;
    position = other.position;
    resizable = other.resizable;
    growthFactor = other.growthFactor;
    growthIncrement = other.growthIncrement;
    if (buffer)
        ++buffer->refCount;
}

bool MemoryStream::SetData(const void* src, uint32_t dataSize, bool usePointer, bool freePointer,
                           allocator::ICoreAllocator* dataAllocator)
{
    bool ok = false;
    if (src || dataSize) {
        allocator::ICoreAllocator* allocator = dataAllocator ? dataAllocator : DefaultAllocator();
        void* data = const_cast<void*>(src);
        if (!usePointer)
            data = allocator->Alloc(dataSize, name ? name : kDataName, 0);
        if (data) {
            if (buffer)
                buffer->Release();
            auto* holder = static_cast<SharedBuffer*>(EaNew(sizeof(SharedBuffer), allocator, name ? name : kPtrName));
            if (holder) {
#if defined(_M_IX86)
                holder->vtable = reinterpret_cast<void* const*>(static_cast<uintptr_t>(0x00FB84D0));
#else
                holder->vtable = nullptr;
#endif
                holder->allocator = allocator;
                holder->data = static_cast<uint8_t*>(data);
                holder->refCount = 0;
                holder->ownsData = freePointer;
            }
            buffer = holder;
            if (holder) {
                ++holder->refCount;
                if (src && dataSize && !usePointer)
                    std::memcpy(data, src, dataSize);
                ok = true;
            } else if (!usePointer) {
                allocator->Free(data, 0);
            }
        }
    } else {
        if (buffer)
            buffer->Release();
        buffer = nullptr;
        ok = true;
    }
    position = 0;
    capacity = buffer ? dataSize : 0;
    size = buffer ? dataSize : 0;
    return ok;
}

off_type MemoryStream::GetPosition(PositionType type) const
{
    if (type == kPositionTypeBegin)
        return static_cast<off_type>(position);
    if (type == kPositionTypeEnd)
        return static_cast<off_type>(position - size);
    return 0;
}

bool MemoryStream::SetPosition(off_type newPosition, PositionType type)
{
    const uint32_t previous = position;
    const auto delta = static_cast<uint32_t>(newPosition);
    if (type == kPositionTypeBegin)
        position = delta;
    else if (type == kPositionTypeCurrent)
        position = previous + delta;
    else if (type == kPositionTypeEnd)
        position = size + delta;
    if (position > size) {
        if (!resizable) {
            position = size;
            return false;
        }
        if (!Realloc(position)) {
            position = previous;
            return false;
        }
    }
    return true;
}

size_type MemoryStream::Read(void* dst, size_type count)
{
    if (!count)
        return 0;
    const uint32_t available = size - position;
    if (!available)
        return 0;
    auto n = static_cast<uint32_t>(count);
    if (n > available)
        n = available;
    std::memcpy(dst, buffer->data + position, n);
    position += n;
    return n;
}

bool MemoryStream::SetSize(size_type requested)
{
    const auto newSize = static_cast<uint32_t>(requested);
    if (newSize == size)
        return true;
    if (!resizable)
        return false;
    if (newSize >= size) {
        if (!Realloc(newSize))
            return false;
        size = newSize;
        return true;
    }
    size = newSize;
    if (position > newSize)
        position = newSize;
    return true;
}

bool MemoryStream::Write(const void* src, size_type requested)
{
    const auto count = static_cast<uint32_t>(requested);
    if (!count)
        return true;
    const uint32_t end = position + count;
    uint32_t n = count;
    if (end <= capacity) {
        if (size < end)
            size = end;
    } else if (!resizable) {
        n = size - position;  // only what fits within the current size
    } else {
        auto grown = static_cast<int64_t>(static_cast<double>(capacity) * growthFactor + static_cast<double>(growthIncrement));
        if (grown < end)
            grown = end;
        if (!Realloc(static_cast<uint32_t>(grown)))
            return false;
        size = end;
    }
    std::memcpy(buffer->data + position, src, n);
    position += n;
    return n == count;
}

}  // namespace ts3::io
