#pragma once
#include "ts3/layout.h"

namespace ts3::misc {
struct MiscRecord {
    void* begin;
    void* end;
    void* limit;
    const char* name;
};
TS3_X86_OFFSET(MiscRecord, begin, 0);
TS3_X86_OFFSET(MiscRecord, end, 4);
TS3_X86_OFFSET(MiscRecord, limit, 8);
TS3_X86_OFFSET(MiscRecord, name, 12);
TS3_X86_SIZE(MiscRecord, 16);

// TS3W.exe 0x00F83DE0: initialize ten global records and return exit-handler registration status.
int InitMiscRecords();
} // namespace ts3::misc
