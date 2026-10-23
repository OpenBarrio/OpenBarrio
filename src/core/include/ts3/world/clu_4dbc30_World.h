#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::world {

// clu_4dbc30_World: the World/LoadSaveManager cluster of TS3W.exe.
//
// Game objects are byte records (pointer fields are 32-bit on the original ABI); the
// member helpers use the byte offsets the original code reads and writes. Callees from
// other units are called through the ts3::CallGame* bridges and abort on x64.

// 0x004DBC30 (cdecl, 3 args, leaf): wraps CopyFileW, returning 0/1; the second
// argument of CopyFileW is (failIfExists == 0).
int32_t CopyFileDbc30(const wchar_t* existing, const wchar_t* destination, uint8_t failIfExists);

// 0x004FA500 (cdecl, 5 args, leaf): 64-bit table-driven state hash over `count` bytes.
// The game returns the pair in EDX:EAX, so the result is packed as (hi << 32) | lo.
// `invert` complements both halves.
uint64_t Hash4FA500(const uint8_t* data, int32_t count, uint32_t seedLo, uint32_t seedHi,
                    uint8_t invert);

// 0x0056CD30 (thiscall, 2 args, ret 8): delegates to 0x004DDD20 with the two handles at
// +8/+0x10, then forwards the byte result through 0x007D77C0.
struct LoadSaveWrapper56CD30 {
    uint8_t bytes[0x14];

    int32_t Invoke(int32_t a2, int32_t a3);
};
TS3_X86_SIZE(LoadSaveWrapper56CD30, 0x14);

// 0x0059F5B0 (thiscall, 2 args, ret 8): delegates to 0x004DDAB0 with the handle at +8.
struct LoadSaveWrapper59F5B0 {
    uint8_t bytes[0xC];

    int32_t Invoke(int32_t a2, int32_t a3);
};
TS3_X86_SIZE(LoadSaveWrapper59F5B0, 0xC);

// 0x005FE3A0 (thiscall, 1 arg, ret 4): delegates to 0x004DDA90 with the handle at +8.
struct LoadSaveWrapper5FE3A0 {
    uint8_t bytes[0xC];

    int32_t Invoke(int32_t a2);
};
TS3_X86_SIZE(LoadSaveWrapper5FE3A0, 0xC);

// 0x005C9280 (thiscall, no arguments): folds the 0x50-byte records in [0xa4, 0xa8) and
// then the four dwords at +8..+0x14 plus the dword at +0x1c through 0x004FA500.
struct LoadSaveHash5C9280 {
    uint8_t bytes[0xAC];

    int32_t Hash();
};
TS3_X86_SIZE(LoadSaveHash5C9280, 0xAC);

// 0x005D7090 (thiscall, 3 args, ret 0xC): hash-map lookup over the 0x74-byte records in
// [0xc, 0x10); the bucket array is at +0x114 with +0x118 buckets, chain nodes are linked
// at +0x30 and keyed by their first dword.
struct LoadSaveLookup5D7090 {
    uint8_t bytes[0x2F0];

    uint8_t Lookup(int32_t selector, void* key, void* vector);
};
TS3_X86_SIZE(LoadSaveLookup5D7090, 0x2F0);

// 0x00742B40 (cdecl, 3 args): script-side LoadSaveManager native-writer destroy entry.
int32_t LoadSaveManagerDestroyNativeWriter742B40(int32_t a1, int32_t a2, int32_t a3);

// 0x00742CB0 (cdecl, 3 args): 5-byte tail thunk over 0x00742B40.
int32_t BridgeLoadSaveManagerDestroyNativeWriter742CB0(int32_t a1, int32_t a2, int32_t a3);

// 0x007D84E0 (thiscall, 2 args, ret 8): runs two writer steps and folds their low bytes
// into the flag at +0x14.
struct LoadSaveWriter7D84E0 {
    uint8_t bytes[0x18];

    int32_t Write(int32_t a1, int32_t a2);
};
TS3_X86_SIZE(LoadSaveWriter7D84E0, 0x18);

// 0x007D8520 (thiscall, 2 args, ret 8): as 0x007D84E0, with the second step gated on the
// folded flag remaining non-zero.
struct LoadSaveWriter7D8520 {
    uint8_t bytes[0x18];

    int32_t Write(int32_t a1, int32_t a2);
};
TS3_X86_SIZE(LoadSaveWriter7D8520, 0x18);

// The 16-byte string record used by 0x007D9C50 / 0x007D9CC0 and the LoadSaveManager
// writer: {begin, end, cap, alloc-name}. 0x00408830 reserves, 0x00408B90 appends.
struct TSString4dbc30 {
    void* begin;
    void* end;
    void* cap;
    const char* allocName;
};
TS3_X86_SIZE(TSString4dbc30, 0x10);

// 0x007D9C50 (cdecl, 3 args): concatenates two string records into `out`.
TSString4dbc30* Concat7D9C50(TSString4dbc30* out, const TSString4dbc30* a,
                             const TSString4dbc30* b);

// 0x007D9CC0 (cdecl, 3 args): concatenates a string record and a NUL-terminated wide
// string into `out`.
TSString4dbc30* ConcatAppend7D9CC0(TSString4dbc30* out, const TSString4dbc30* a,
                                   const wchar_t* src);

// 0x00AB0020 (thiscall, 3 args, ret 0xC): the LoadSaveManager native-writer expansion
// entry. `self` is the object obtained from 0x00AAB380, `data`/`count` describe the
// buffer to hash into the package and `name` is the caller-supplied wide name.
struct LoadSaveManagerAb0020Self {
    uint8_t bytes[0x50];
};
TS3_X86_SIZE(LoadSaveManagerAb0020Self, 0x50);

int32_t LoadSaveManagerWriteAb0020(LoadSaveManagerAb0020Self* self, const uint8_t* data,
                                   int32_t count, const wchar_t* name);

}  // namespace ts3::world