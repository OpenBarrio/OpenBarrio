#pragma once
#include "ts3/layout.h"

namespace ts3::sacs {
struct Vector16 {
    std::byte* begin;
    std::byte* end;
    std::byte* capacityEnd;
    void* allocator;

    // TS3W.exe 0x00D750A0: reserve 16-byte elements; returns begin if unchanged, end after growth.
    std::byte* Reserve(uint32_t count);
};
TS3_X86_OFFSET(Vector16, begin, 0);
TS3_X86_OFFSET(Vector16, end, 4);
TS3_X86_OFFSET(Vector16, capacityEnd, 8);
TS3_X86_OFFSET(Vector16, allocator, 12);
TS3_X86_SIZE(Vector16, 16);
}
