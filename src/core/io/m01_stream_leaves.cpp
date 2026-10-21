#include "ts3/io/m01_stream_leaves.h"
#include <new>

namespace ts3::io::m01 {
namespace {
void* const* Vtable(uintptr_t address) {
#if defined(_M_IX86)
    return reinterpret_cast<void* const*>(address);
#else
    (void)address;
    return nullptr;
#endif
}
}
// Constant/field leaves start from ResourceManClassId/DDFRecordIndex and
// ResourceMapList::VtableConstant, adapting only their immediate/field offset.
uint32_t __fastcall FileType(void*, void*) { return 0x34722300u; }
int32_t __fastcall MemoryAddRef(MemoryStream* self, void*) {
    self->refCount = static_cast<int32_t>(static_cast<uint32_t>(self->refCount) + 1u);
    return self->refCount;
}
uint32_t __fastcall MemoryType(void*, void*) { return 0x347223d2u; }
bool __fastcall MemoryClose(MemoryStream* self, void*) {
    return self->SetData(nullptr, 0, false, true, nullptr);
}
// Same normalized deleting-dtor body as GlyphMetricsMap::DestroyBase;
// the base vtable and delete target are the only substitutions.
void* __fastcall DestroyStreamBase(void* self, void*, uint32_t flags) {
    *static_cast<void* const**>(self) = Vtable(0x00FA99FC);
    if (flags & 1) {
#if defined(_M_IX86)
        CallGameFunction<void, 0x004E3F70>(self);
#else
        ::operator delete(self);
#endif
    }
    return self;
}
SharedBuffer* __fastcall WrapBuffer(SharedBuffer* self, void*, void* data, bool owns,
                                   allocator::ICoreAllocator* allocator) {
    self->vtable = Vtable(0x00FB84D0);
    self->allocator = allocator ? allocator : DefaultAllocator();
    self->data = static_cast<uint8_t*>(data);
    self->refCount = 0;
    self->ownsData = owns;
    return self;
}
SharedBuffer* __fastcall AllocateBuffer(SharedBuffer* self, void*, uint32_t size,
                                       allocator::ICoreAllocator* allocator, const char* name) {
    self->vtable = Vtable(0x00FB84D0);
    self->allocator = allocator ? allocator : DefaultAllocator();
    if (!name) {
#if defined(_M_IX86)
        name = reinterpret_cast<const char*>(0x00FB84D4);
#else
        name = "Services/EAIO/EAStreamMemory/data";
#endif
    }
    self->data = static_cast<uint8_t*>(self->allocator->Alloc(size, name, 0));
    self->refCount = 0;
    self->ownsData = true;
    return self;
}
MemoryStream* __fastcall ConstructShared(MemoryStream* self, void*, SharedBuffer* buffer,
                                         uint32_t size, const char* name) {
    self->vtable = Vtable(kMemoryVtable);
    self->buffer = nullptr;
    self->name = name;
    self->refCount = 0;
    self->size = 0;
    self->capacity = 0;
    self->position = 0;
    self->resizable = false;
    self->growthFactor = 1.5f;
    self->growthIncrement = 0;
    if (buffer && size) {
#if defined(_M_IX86)
        CallGameMethod<bool, 0x004DEDE0>(self, buffer, size);
#else
        ++buffer->refCount;
        self->buffer = buffer;
        self->size = size;
        self->capacity = size;
#endif
    }
    return self;
}
int32_t __fastcall MemoryState(void*, void*) { return 0; }
SharedBuffer* __fastcall DestroyBuffer(SharedBuffer* self, void*, uint32_t flags) {
    self->vtable = Vtable(0x00FB84D0);
    if (flags & 1)
        EaDelete(self);
    return self;
}
int32_t __fastcall MemoryAccess(void*, void*) { return 3; }
uint32_t __fastcall MemorySize(MemoryStream* self, void*) { return self->size; }
const char* __fastcall BindingName(void*, void*) {
#if defined(_M_IX86)
    return reinterpret_cast<const char*>(0x0100C718);
#else
    return "StreamHost";
#endif
}
BindingEntry** __fastcall BindingTable(void*, void*, BindingEntry** out) {
#if defined(_M_IX86)
    *out = reinterpret_cast<BindingEntry*>(kBindingTable);
#else
    // Supplied by the native scripting host, never a fabricated empty table.
    *out = nativeBindingTable;
#endif
    return out;
}
}
