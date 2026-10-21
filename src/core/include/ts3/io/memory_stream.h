// EA::IO::MemoryStream (EAIO "EAStreamMemory"; vtable 0x00FB5BCC, constructor 0x004DF120):
// an IStream over a reference-counted memory buffer that can grow. Records of packages
// embed one for data held in memory. Partial layout, x86 offsets.
#pragma once

#include <cstdint>

#include "ts3/allocator/core_allocator.h"
#include "ts3/io/stream.h"
#include "ts3/layout.h"

namespace ts3::io {

// EA's allocator-aware operator new (TS3W.exe 0x004E0510, and its twin 0x004E3DB0 that
// fetches the default allocator without the cache): allocates size + 16 bytes (alignment 4,
// offset 16) and keeps the allocator in that 16-byte header, for delete.
void* EaNew(size_t size, allocator::ICoreAllocator* allocator, const char* name);
// EA's matching delete (TS3W.exe 0x004E4230): frees through the allocator in the header.
void EaDelete(void* object);
// The engine's default allocator (TS3W.exe 0x004D9AD0, a global), not rewritten yet.
allocator::ICoreAllocator* DefaultAllocator();
// The uncached getter behind it (TS3W.exe 0x00931020), which some code calls directly.
allocator::ICoreAllocator* DefaultAllocatorDirect();

// The buffer a MemoryStream points to, shared between copies (vtable 0x00FB84D0). In x86
// builds buffers made here carry the game's vtable, because game code may release them.
struct SharedBuffer {
    void* const* vtable;
    allocator::ICoreAllocator* allocator;  // 0x04
    uint8_t* data;                         // 0x08
    int32_t refCount;                      // 0x0C
    bool ownsData;                         // 0x10

    // EaNew + constructor 0x004DEBE0: `capacity` bytes from `allocator` (the default one when
    // null), with reference count 0. nullptr when the holder can't be allocated; `data` is
    // null when the bytes can't be.
    static SharedBuffer* Create(uint32_t capacity, allocator::ICoreAllocator* allocator, const char* streamName);
    // TS3W.exe 0x004DEA80: drops one reference; the last one frees the data (when owned) and
    // the holder. Returns the references left.
    int32_t Release();
};
TS3_X86_OFFSET(SharedBuffer, data, 0x08);
TS3_X86_SIZE(SharedBuffer, 20);

struct MemoryStream {
    void* const* vtable;
    SharedBuffer* buffer;     // 0x04
    const char* name;         // 0x08 allocation name
    int32_t refCount;         // 0x0C (AddRef, 0x004559F0)
    uint32_t size;            // 0x10
    uint32_t capacity;        // 0x14
    uint32_t position;        // 0x18
    bool resizable;           // 0x1C
    float growthFactor;       // 0x20 1.5
    int32_t growthIncrement;  // 0x24

    // IStream slots 7, 9, 10, 11, 12, 8 and 14: TS3W.exe 0x00D1AA10, 0x004DEB70, 0x004DEF30,
    // 0x00455A40, 0x004DEFA0, 0x004DF190, 0x004DEFF0.
    size_type GetSize() const { return size; }
    // TS3W.exe 0x004ADBB0: the buffer's bytes; null without a buffer.
    uint8_t* Data() const { return buffer ? buffer->data : nullptr; }
    off_type GetPosition(PositionType type) const;
    // Moving past the end grows a resizable stream's buffer (its size stays); otherwise it
    // clamps to the end and fails.
    bool SetPosition(off_type position, PositionType type);
    size_type GetAvailable() const { return size - position; }
    size_type Read(void* dst, size_type count);
    bool SetSize(size_type newSize);
    // Grows the buffer to capacity * growthFactor + growthIncrement (at least what is needed)
    // when resizable; otherwise writes what fits within the size and reports failure.
    // EA bug, kept: on a stream with no buffer (emptied by SetData) it dereferences null.
    bool Write(const void* src, size_type count);

    // TS3W.exe 0x004DEE40: moves the data to a new shared buffer of the given capacity (none
    // for 0), copying what fits; the old buffer loses this stream's reference.
    bool Realloc(uint32_t newCapacity);

    // Constructor, TS3W.exe 0x004DF120, on uninitialised memory (x86 builds give the object
    // the game's vtable, 0x00FB5BCC). With data, as SetData.
    void Construct(const void* src, uint32_t dataSize, bool usePointer, bool freePointer,
                   allocator::ICoreAllocator* dataAllocator, const char* allocationName);
    // TS3W.exe 0x004DEC90: the stream's data becomes `src` itself (usePointer) or a copy of
    // it; the old buffer loses this stream's reference. freePointer makes the new buffer own
    // its data. No data (src null and size 0) empties the stream.
    bool SetData(const void* src, uint32_t dataSize, bool usePointer, bool freePointer,
                 allocator::ICoreAllocator* dataAllocator);
    // Copy constructor, TS3W.exe 0x004DEC30: shares the other stream's buffer (one more
    // reference). Quirk kept: the copy's capacity is the other's size, not its capacity.
    void ConstructCopy(const MemoryStream& other);
};
TS3_X86_OFFSET(MemoryStream, size, 0x10);
TS3_X86_OFFSET(MemoryStream, position, 0x18);
TS3_X86_OFFSET(MemoryStream, resizable, 0x1C);
TS3_X86_OFFSET(MemoryStream, growthIncrement, 0x24);

}  // namespace ts3::io
