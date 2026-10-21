#include "ts3/io/m01_streams_004cdc40.h"
#include "ts3/io/m01_stream_leaves.h"

namespace ts3::io::m01 {
MemoryStream* __fastcall DestroyMemory(MemoryStream* self, void*, uint32_t flags) {
#if defined(_M_IX86)
    self->vtable = reinterpret_cast<void* const*>(kMemoryVtable);
#else
    self->vtable = nullptr;
#endif
    if (self->buffer)
        self->buffer->Release();
    // Verified base deleting destructor uses ordinary delete, NOT EaDelete.
    DestroyStreamBase(self, nullptr, flags);
    return self;
}
}
