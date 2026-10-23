#include "ts3/lotrenderer/clu_c7ccc0_LotRenderer.h"

#include <cstring>

namespace ts3::lotrenderer {
namespace {
int32_t ReadI32(const void* self, size_t offset)
{
    int32_t value;
    std::memcpy(&value, static_cast<const unsigned char*>(self) + offset, sizeof value);
    return value;
}
} // namespace

// TS3W.exe 0x00C7CCC0 (64 bytes), thiscall: bool(this, first, second), ret 8.
bool LotRendererC7CCC0::Matches(int32_t first, int32_t second) const
{
    const void* self = this;
    if (first != ReadI32(self, 0x48) || second != ReadI32(self, 0x4c))
        return false;
    const int32_t state = ReadI32(self, 0x5c);
    if (state < 0)
        return false;
    if (state > 0)
        return true;
    const int32_t kind = ReadI32(self, 0x54);
    return kind == 201 || kind == 202 || kind == 204;
}

} // namespace ts3::lotrenderer
