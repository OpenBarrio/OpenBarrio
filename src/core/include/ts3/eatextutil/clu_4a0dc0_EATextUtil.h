#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::eatextutil {

struct PathEntry4A0DC0 {
    uint32_t type;
};

// x86 record: entry/path/pattern pointers followed by two signed priorities.
struct PathRecord4A0DC0 {
    const PathEntry4A0DC0* entry;
    const char16_t* path;
    const char16_t* pattern;
    int16_t priority;
    int16_t subpriority;
    uint32_t reserved;
};

TS3_X86_SIZE(PathRecord4A0DC0, 20);
TS3_X86_OFFSET(PathRecord4A0DC0, priority, 12);
TS3_X86_OFFSET(PathRecord4A0DC0, subpriority, 14);

struct PathRecordRange4A0DC0 {
    const PathRecord4A0DC0* begin;
    const PathRecord4A0DC0* end;
};

TS3_X86_SIZE(PathRecordRange4A0DC0, 8);
TS3_X86_OFFSET(PathRecordRange4A0DC0, end, 4);

// TS3W.exe 0x004A0DC0.
const PathEntry4A0DC0* FindPathEntry004A0DC0(const PathRecordRange4A0DC0* records, uint32_t type,
                                             const char16_t* query);

// TS3W.exe 0x004EC3C0.
bool IsTextMode004EC3C0(int32_t mode);

// TS3W.exe 0x00500010.
bool MatchWildcardWide00500010(const char16_t* text, const char16_t* pattern);

// TS3W.exe 0x005000F0.
bool MatchTextPatternWide005000F0(const char16_t* text, const char16_t* pattern, uint8_t caseSensitive);

}  // namespace ts3::eatextutil
