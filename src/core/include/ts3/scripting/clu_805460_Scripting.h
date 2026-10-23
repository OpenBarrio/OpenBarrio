#pragma once

#include <cstddef>
#include <cstdint>

#include "ts3/layout.h"

namespace ts3::scripting {

struct ScriptItem805460 {
    uint8_t prefix[8];
    uint32_t value;
    uint8_t padding12;
    uint8_t active;
    uint8_t padding14[10];
};

class ScriptState805460 {
public:
    uint32_t reserved0;
    ScriptItem805460* begin;
    ScriptItem805460* end;
    uint32_t reserved0C;
    uint32_t reserved10;
    uint32_t key;

    // TS3W.exe 0x00805460.
    bool HasActiveOrKeyMatch() const;
};

// TS3W.exe 0x0082AB70: cdecl lower-bound search over sorted 32-bit keys.
uint32_t* LowerBoundKey0082AB70(uint32_t* begin, uint32_t* end, const uint32_t* key);

struct SearchResult82DCD0 {
    uint32_t* position;
    uint8_t inserted;
};

class SortedWordVector82DCD0 {
public:
    uint32_t* begin;
    uint32_t* end;
    uint32_t* capacity;
    uint32_t allocator0C;
    uint8_t option10;
    uint8_t allocatorPadding[7];

    // TS3W.exe 0x00AD3080.
    uint32_t* InsertAt(uint32_t* position, const uint32_t* value);
    // TS3W.exe 0x0082DCD0.
    SearchResult82DCD0* FindOrInsert(SearchResult82DCD0* result, const uint32_t* key);
};

struct CoordinateBoundsAF03C0 {
    uint8_t prefix[0x174];
    int32_t maxX;
    int32_t maxY;
};

class CoordinateMapAF03C0 {
public:
    uint32_t reserved0;
    CoordinateBoundsAF03C0* bounds;
    uint8_t prefix08[0x128];
    SortedWordVector82DCD0 points;

    // TS3W.exe 0x00AF03C0. The valid path returns the address of its local
    // SearchResult, matching the original's dangling-pointer output.
    void* GetOrAddCoordinate(int32_t x, int32_t y, float z);
};

TS3_X86_OFFSET(ScriptItem805460, value, 8);
TS3_X86_OFFSET(ScriptItem805460, active, 13);
TS3_X86_SIZE(ScriptItem805460, 24);
TS3_X86_OFFSET(ScriptState805460, begin, 4);
TS3_X86_OFFSET(ScriptState805460, end, 8);
TS3_X86_OFFSET(ScriptState805460, key, 20);
TS3_X86_SIZE(ScriptState805460, 24);
TS3_X86_OFFSET(SortedWordVector82DCD0, end, 4);
TS3_X86_OFFSET(SortedWordVector82DCD0, capacity, 8);
TS3_X86_OFFSET(SortedWordVector82DCD0, option10, 16);
TS3_X86_SIZE(SortedWordVector82DCD0, 24);
TS3_X86_OFFSET(CoordinateBoundsAF03C0, maxX, 0x174);
TS3_X86_OFFSET(CoordinateBoundsAF03C0, maxY, 0x178);
TS3_X86_OFFSET(CoordinateMapAF03C0, bounds, 4);
TS3_X86_OFFSET(CoordinateMapAF03C0, points, 0x130);

}  // namespace ts3::scripting
