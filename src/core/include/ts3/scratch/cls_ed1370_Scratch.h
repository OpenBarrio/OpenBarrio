#pragma once

#include <cstddef>
#include <cstdint>

#include "ts3/layout.h"

namespace ts3::scratch {

// Common decoder prefix initialized by 0x00ED1370 and allocated by 0x00ED79D0.
struct ScratchDecoderBase {
    uint8_t bytes[0x34];
    ScratchDecoderBase* Construct(uint8_t flags);
};

// Descriptor consumed by 0x00ED79D0. Fields not named here are not read by that entry.
struct ScratchDecoderOps {
    uint32_t (__cdecl* decodedSize)(uint32_t argument, uint32_t* blockCount); // +0
    bool (__cdecl* initialize)(ScratchDecoderBase* decoder);                  // +4
    void* vtable;                                                              // +8
    uint32_t state;                                                            // +0x0c
    uint32_t reserved10;                                                       // +0x10
    uint32_t state14;                                                          // +0x14
    uint16_t blockWidth;                                                       // +0x18
};

struct ScratchDecoderContext {
    uint8_t prefix[0x14];
    void* allocator; // +0x14, used only when descriptor.blockWidth is nonzero
};

void* __stdcall CreateScratchDecoder(ScratchDecoderOps* ops, uint32_t argument,
                                      uint32_t blockCount, ScratchDecoderContext* context);

// The vtable at 0x00EEDF90 uses a larger derived object; only offsets +0x48 and +0x2c0
// are relevant to its destructor.
struct ScratchDecoderDerived {
    uint8_t bytes[0x2c4];
    ScratchDecoderDerived* Destroy(uint32_t flags);
};

TS3_X86_SIZE(ScratchDecoderBase, 0x34);
TS3_X86_SIZE(ScratchDecoderOps, 0x1c);
TS3_X86_OFFSET(ScratchDecoderOps, blockWidth, 0x18);
TS3_X86_OFFSET(ScratchDecoderContext, allocator, 0x14);
TS3_X86_SIZE(ScratchDecoderContext, 0x18);
TS3_X86_OFFSET(ScratchDecoderDerived, bytes, 0x00);
TS3_X86_SIZE(ScratchDecoderDerived, 0x2c4);

} // namespace ts3::scratch
