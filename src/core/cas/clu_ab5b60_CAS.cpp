#include "ts3/cas/clu_ab5b60_CAS.h"
#include <cstring>

namespace ts3::cas {
// 0x00AB5B60, cdecl; count is unsigned (TEST/JBE and TEST/JA).
void* Fill16(void* destination, std::uint32_t count, const void* source)
{
    auto cursor = reinterpret_cast<std::uintptr_t>(destination);
    while (count != 0) {
        if (cursor != 0) {
            // Do not cache the source record or use memcpy of all 16 bytes:
            // earlier stores can change later source words when buffers overlap.
            for (unsigned offset = 0; offset < 16; offset += 4) {
                std::uint32_t word;
                std::memcpy(&word, static_cast<const unsigned char*>(source) + offset, 4);
                std::memcpy(reinterpret_cast<void*>(cursor + offset), &word, 4);
            }
        }
        --count;
        cursor += 16;
    }
    return reinterpret_cast<void*>(cursor);
}
}
