#include "ts3/io/m01_streams_004dede0.h"

namespace ts3::io::m01 {
bool __fastcall AttachShared(MemoryStream* self, void*, SharedBuffer* buffer, uint32_t size)
{
    if (self->buffer != buffer) {
        // Acquire first: destruction of the previous holder may observe this ref.
        if (buffer)
            ++buffer->refCount;
        if (self->buffer)
            self->buffer->Release();
        self->buffer = buffer;
    }
    self->size = self->buffer ? size : 0;
    self->capacity = self->buffer ? size : 0;
    self->position = 0;
    return self->buffer != nullptr;
}
}
