#pragma once
#include "ts3/layout.h"

namespace ts3::ui {
struct UiAppendRecord {
    uint32_t values[4];
    uint32_t tag;
    uint8_t flag;
    uint8_t padding[3];
};
TS3_X86_OFFSET(UiAppendRecord, tag, 16);
TS3_X86_OFFSET(UiAppendRecord, flag, 20);
TS3_X86_SIZE(UiAppendRecord, 24);

struct UiRecordList {
    UiAppendRecord* begin;
    UiAppendRecord* end;
    UiAppendRecord* capacity;
    void* allocator;
    // TS3W.exe 0x005E69B0: append a record and return it; preserve the three trailing bytes.
    UiAppendRecord* Append(uint32_t tag, uint8_t flag, uint32_t a, uint32_t b,
                           uint32_t c, uint32_t d);
};
TS3_X86_OFFSET(UiRecordList, begin, 0);
TS3_X86_OFFSET(UiRecordList, end, 4);
TS3_X86_OFFSET(UiRecordList, capacity, 8);
TS3_X86_OFFSET(UiRecordList, allocator, 12);
TS3_X86_SIZE(UiRecordList, 16);
} // namespace ts3::ui
