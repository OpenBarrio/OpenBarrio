// Unit clu_7d69b0_ResourceLoad (ResourceLoad): the 132-byte ResourceTableIOContext
// constructor. Semantics from the TS3W.exe 1.67.2 listing (original address noted).
#include "ts3/resourceload/clu_7d69b0_ResourceLoad.h"

namespace ts3::resourceload {
namespace {

// A game rdata string address, the way cls_6dea00_World stores its name fields.
inline const char* AtString(uintptr_t address)
{
    return reinterpret_cast<const char*>(address);
}

}  // namespace

// TS3W.exe 0x007D69B0 (thiscall, ret 4): loads the two float globals once (xmm0/xmm1),
// then stores, per spec block, {esi=0x0114C900, 1, 0, xmm0, xmm1, 0, name}; the last
// block additionally gets word 1 at +0x80 and the char argument at +0x82 (read from
// [esp+8] after the prologue's the original instruction sequence). +0x00, the first three blocks' +0x1C tails
// and +0x83 are never written. Returns this.
void* ResourceTableIOContext::Construct(char flag)
{
    // The original reads the 1.0f/2.0f from the float globals 0x0107A538/0x00F98A58
    // (1.0f/2.0f in TS3W.exe 1.67.2); other code can write them, so the x86 build
    // reads them through the game's memory like the original.
#if defined(_M_IX86)
    const float one = *reinterpret_cast<const float*>(0x107A538);
    const float two = *reinterpret_cast<const float*>(0xF98A58);
#else
    const float one = 1.0f;
    const float two = 2.0f;
#endif

    keyToHandleMap.id = 0x0114C900;
    keyToHandleMap.count = 1;
    keyToHandleMap.field08 = 0;
    keyToHandleMap.min0C = one;
    keyToHandleMap.max10 = two;
    keyToHandleMap.field14 = 0;
    keyToHandleMap.name18 = AtString(0x0101A4C8);

    handleToKeyMap.id = 0x0114C900;
    handleToKeyMap.count = 1;
    handleToKeyMap.field08 = 0;
    handleToKeyMap.min0C = one;
    handleToKeyMap.max10 = two;
    handleToKeyMap.field14 = 0;
    handleToKeyMap.name18 = AtString(0x0101A494);

    localKeyHandleSet.id = 0x0114C900;
    localKeyHandleSet.count = 1;
    localKeyHandleSet.field08 = 0;
    localKeyHandleSet.min0C = one;
    localKeyHandleSet.max10 = two;
    localKeyHandleSet.field14 = 0;
    localKeyHandleSet.name18 = AtString(0x0101A45C);

    remapHashMap.id = 0x0114C900;
    remapHashMap.count = 1;
    remapHashMap.field08 = 0;
    remapHashMap.min0C = one;
    remapHashMap.max10 = two;
    remapHashMap.field14 = 0;
    remapHashMap.name18 = AtString(0x0101A428);
    remapHashMap.last.tailWord = 1;
    remapHashMap.last.flag1E = static_cast<uint8_t>(flag);

    return this;
}

}  // namespace ts3::resourceload
