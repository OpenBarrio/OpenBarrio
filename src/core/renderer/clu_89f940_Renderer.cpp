#include "ts3/renderer/clu_89f940_Renderer.h"

namespace ts3::renderer {
// 0x0089F940 (__thiscall, ret 8). Only AL is the return value.
bool RendererRecordReader::ReadScalar(uint32_t tag, uint32_t* output)
{
    uint32_t count = tag;
    uint32_t byteSize = 0;
    const auto* data = CallGameMethod<const uint32_t*, 0x0089F7D0>(
        this, tag, &count, &byteSize);
    // EA checks element count, not byte size: even a short/zero-sized record
    // copies a full DWORD. Failure leaves output untouched (and permits null).
    if (!data || count != 1)
        return false;
    *output = *data;
    return true;
}
} // namespace ts3::renderer
