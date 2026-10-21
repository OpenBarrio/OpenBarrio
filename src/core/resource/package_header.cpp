#include "ts3/resource/package_header.h"

namespace ts3::resource {

bool IsValidPackageHeader(const PackageHeader& h, uint32_t fileSize)
{
    // 32-bit wrap-around in `indexSize + indexOffsetLow` is part of the original check.
    const bool valid = h.magic == kDbpfMagic && h.majorVersion <= 2 && h.indexCount < 0x7FFFFFF &&
                       h.indexOffsetLow < fileSize && static_cast<uint32_t>(h.indexSize + h.indexOffsetLow) <= fileSize &&
                       h.indexVersion != 0;

    if ((h.indexOffsetLow | h.indexOffsetHigh) != 0 && h.indexOffsetV1 != 0)
        return h.indexOffsetLow == h.indexOffsetV1 && h.indexOffsetHigh == 0;
    return valid;
}

}  // namespace ts3::resource
