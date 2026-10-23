#pragma once
#include "ts3/layout.h"

namespace ts3::renderer {
// Prefix consumed by the original packed-record lookup (0x0089F7D0).
struct RendererRecordReader {
    uint32_t reserved0[2];
    void* records;
    uint32_t reserved1[2];
    uint32_t recordCount;

    // TS3W.exe 0x0089F940: copy a DWORD from the first matching record only when its element count is one.
    bool ReadScalar(uint32_t tag, uint32_t* output);
};
TS3_X86_OFFSET(RendererRecordReader, records, 0x08);
TS3_X86_OFFSET(RendererRecordReader, recordCount, 0x14);
TS3_X86_SIZE(RendererRecordReader, 0x18);
} // namespace ts3::renderer
