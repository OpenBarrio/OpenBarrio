// Cluster unit clu_7d69b0_ResourceLoad (subsystem ResourceLoad): one constructor for
// the 132-byte ResourceTableIOContext object - the per-load context holding the four
// named table specs (KeyToHandleMap, HandleToKeyMap, LocalKeyHandleSet, RemapHashMap).
//
//   0x007D69B0  ResourceTableIOContext::Construct  thiscall, ret 4: fills `this` with
//                               the constant layout below and returns this
//
// Layout (the x86 game's): +0x00 is never written (vtable slot / padding); four 32-byte
// spec blocks at +0x04/+0x24/+0x44/+0x64, each {id 0x0114C900, count 1, 0, 1.0f, 2.0f,
// 0, name, tail}. The tail dword of the first three blocks is never written; the last
// block's tail gets word 1 at +0x1C and the constructor's char argument at +0x1E, with
// +0x1F left untouched. The 1.0f/2.0f come from the float globals 0x0107A538/0x00F98A58
// (x86; 1.0f/2.0f in TS3W.exe 1.67.2).
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::resourceload {

// One 32-byte table-spec block (same shape as the UiWorld spec blocks at +0x58/+0x78).
struct ResourceTableSpec {
    uintptr_t id;        // +0x00 = 0x0114C900
    int32_t count;      // +0x04 = 1
    int32_t field08;    // +0x08 = 0
    float min0C;        // +0x0C = 1.0f
    float max10;        // +0x10 = 2.0f
    int32_t field14;    // +0x14 = 0
    const char* name18; // +0x18 rdata string
    union {
        uint32_t tail1C;  // +0x1C: never written in the first three blocks
        struct {          // the last block's tail (the RemapHashMap one)
            uint16_t tailWord;  // +0x1C = 1
            uint8_t flag1E;     // +0x1E = the constructor's char argument
            uint8_t pad1F;      // +0x1F never written
        } last;
    };
};
TS3_X86_OFFSET(ResourceTableSpec, id, 0x00);
TS3_X86_OFFSET(ResourceTableSpec, count, 0x04);
TS3_X86_OFFSET(ResourceTableSpec, field08, 0x08);
TS3_X86_OFFSET(ResourceTableSpec, min0C, 0x0C);
TS3_X86_OFFSET(ResourceTableSpec, max10, 0x10);
TS3_X86_OFFSET(ResourceTableSpec, field14, 0x14);
TS3_X86_OFFSET(ResourceTableSpec, name18, 0x18);
TS3_X86_OFFSET(ResourceTableSpec, tail1C, 0x1C);
TS3_X86_SIZE(ResourceTableSpec, 0x20);

// The 132-byte context object.
class ResourceTableIOContext {
public:
    // 0x007D69B0 (thiscall, ret 4): initializes all four spec blocks with the constant
    // layout above, stores the char argument at +0x82, and returns this.
    void* Construct(char flag);

    // Layout fields (public so the TS3_X86_OFFSET checks can offsetof them, as in
    // cls_6dea00_World.h).
    uint32_t pad00;                  // +0x00 never written
    ResourceTableSpec keyToHandleMap;    // +0x04
    ResourceTableSpec handleToKeyMap;    // +0x24
    ResourceTableSpec localKeyHandleSet; // +0x44
    ResourceTableSpec remapHashMap;      // +0x64
};
TS3_X86_OFFSET(ResourceTableIOContext, pad00, 0x00);
TS3_X86_OFFSET(ResourceTableIOContext, keyToHandleMap, 0x04);
TS3_X86_OFFSET(ResourceTableIOContext, handleToKeyMap, 0x24);
TS3_X86_OFFSET(ResourceTableIOContext, localKeyHandleSet, 0x44);
TS3_X86_OFFSET(ResourceTableIOContext, remapHashMap, 0x64);
TS3_X86_SIZE(ResourceTableIOContext, 0x84);

}  // namespace ts3::resourceload
