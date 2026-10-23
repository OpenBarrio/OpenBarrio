// Unit cls_4a9480_ResourceLoad: the five functions of the unit that the pilot packed-file
// module (src/core/resource/packed_file.cpp, packed_record.cpp) did not cover yet.
// See the header for the per-function addresses and the ABI notes.
#include "ts3/resourceload/cls_4a9480_ResourceLoad.h"

#include <cstddef>
#include <cstring>

#include "ts3/io/memory_stream.h"
#include "ts3/layout.h"
#include "ts3/resource/packed_file.h"
#include "ts3/resource/packed_record.h"

namespace ts3::resourceload {

// The view in the header must stay byte-compatible with the pilot module's FreeSpaceMap
// on x86 (both 0x24 bytes, the block map at +0x08).
static_assert(sizeof(void*) != 4 || sizeof(ts3::resource::FreeSpaceMap) == sizeof(FreeSpaceMapObject),
              "FreeSpaceMapObject must match ts3::resource::FreeSpaceMap on x86");
static_assert(sizeof(void*) != 4 || offsetof(ts3::resource::FreeSpaceMap, blocks) ==
                                        offsetof(FreeSpaceMapObject, comparator),
              "the block map must start at +0x08 in both views");

#if defined(_M_IX86)
namespace {
constexpr uintptr_t kRecordVtable = 0x00FB2A70;       // PackedFileRecord
constexpr uintptr_t kRecordStreamVtable = 0x00FB2A34; // its IStream part at +0x28
constexpr uintptr_t kMemoryStreamVtable = 0x00FB5BCC; // io::MemoryStream at +0x30
constexpr uintptr_t kEaBaseVtable = 0x00FA99FC;       // the base destructor's vtable
constexpr uintptr_t kSharedBufferVtable = 0x00FB84D0; // the base vtable the object keeps
}  // namespace
#endif

namespace {
// "ResourceSystem/ResourceMan/PackedFile/HoleTable": the game's string in x86 builds, a
// local literal elsewhere so the field still points at valid text.
const char* HoleTableName()
{
#if defined(_M_IX86)
    return reinterpret_cast<const char*>(0x00FB28AC);
#else
    static const char kName[] = "ResourceSystem/ResourceMan/PackedFile/HoleTable";
    return kName;
#endif
}
}  // namespace

FreeSpaceMapObject* FreeSpaceMapObject::Construct4AB5C0()
{
    // Writes exactly the fields 0x004AB5C0 writes; the comparator dword at +0x08 keeps
    // whatever the caller had there. The original zeroes +0x18 as a whole dword (its
    // color byte and padding), then writes its low byte again.
    total = -1;
    limit = 0x7FFFFFFFu;
    anchorLeft = &anchorLeft;  // the map's sentinel node points at itself (+0x0C)
    firstNode = &anchorLeft;
    anchorParent = nullptr;
    anchorColor = 0;
    nodeCount = 0;
    allocatorName = HoleTableName();
    return this;
}

int32_t FreeSpaceMapObject::SumBlockSizes4AA4E0()
{
    uint32_t sum = 0;
    void* node = firstNode;
    void* const sentinel = &anchorLeft;  // +0x0C, where the walk stops
    while (node != sentinel) {
        sum += *reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(node) + 0x14);
        node = CallGameFunction<void*, 0x004E0B20>(node);
    }
    return static_cast<int32_t>(sum);
}

uint32_t FreeSpaceMapDestruct4AB060(FreeSpaceMapObject* self)
{
    // ECX is the block map at +0x08, the argument the node 0x006A0850 starts from (+0x14);
    // 0x006A0850 is still game code (thiscall with one stack argument).
    return CallGameMethod<uint32_t, 0x006A0850>(&self->comparator, self->anchorParent);
}

ZeroedBufferObject* ZeroedBufferObject::Construct4A9480(uint32_t size, void* allocationContext)
{
    // 0x004A8BE0 allocates through the context and fills begin/end; its return value is
    // ignored. The buffer it hands out is zeroed, then `current` is set past the data.
    (void)CallGameMethod<void*, 0x004A8BE0>(this, size, allocationContext);
    if (size)
        std::memset(begin, 0, size);
    current = static_cast<uint8_t*>(begin) + size;
    return this;
}

#if defined(_M_IX86)
void* DestructPackedFileRecord4AE080(ts3::resource::PackedFileRecord* self)
{
    self->vtable = reinterpret_cast<void* const*>(kRecordVtable);
    self->streamVtable = reinterpret_cast<void* const*>(kRecordStreamVtable);
    auto* innerSlot = reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(self) + 0x2C);
    void* inner = *innerSlot;
    if (inner) {
        CallGameVirtual<void, 6>(inner);
        inner = *innerSlot;  // reloaded across the call, as in the original
        if (inner)
            CallGameVirtual<void, 0>(inner, 1);
    }
    if (self->accessFlags)
        CallGameVirtual<bool, 15>(self->owner, static_cast<void*>(self));
    ts3::io::SharedBuffer* buffer = self->memoryStream.buffer;
    self->memoryStream.vtable = reinterpret_cast<void* const*>(kMemoryStreamVtable);
    if (buffer)
        buffer->Release();
    self->memoryStream.vtable = reinterpret_cast<void* const*>(kEaBaseVtable);
    self->streamVtable = reinterpret_cast<void* const*>(kEaBaseVtable);
    self->vtable = reinterpret_cast<void* const*>(kSharedBufferVtable);
    return reinterpret_cast<void*>(kEaBaseVtable);
}
#endif  // _M_IX86

}  // namespace ts3::resourceload