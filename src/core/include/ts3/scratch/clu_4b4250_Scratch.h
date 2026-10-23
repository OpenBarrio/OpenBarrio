// Rewritten cluster unit clu_4b4250_Scratch (Scratch): the content-patch/version
// cluster (0x004B4250..0x009C2D60). The this-call entries are struct member functions
// (MSVC accepts __thiscall only on members); the harness wraps them in __fastcall
// shims. Every declaration carries the original TS3W.exe address.
#pragma once

#include <cstddef>
#include <cstdint>

#include "ts3/layout.h"

namespace ts3::scratch {

// EA dynamic narrow string: {begin, cur, capEnd, allocTag}. cur is one past the last
// character, capEnd is exclusive (usable capacity is capEnd - begin - 1); the 4th field
// is the allocation tag (the string literals this unit passes to the game's helpers).
struct String {
    char* begin;
    char* cur;
    char* cap;
    const char* tag;
};
// EA dynamic wide string, same shape.
struct WideString {
    wchar_t* begin;
    wchar_t* cur;
    wchar_t* cap;
    const char* tag;
};
// One 272-byte element of the token arrays used by 0x0099F0E0 / 0x009B01D0 /
// 0x009C2D60: a 16-byte String header followed by its 256-byte inline buffer, so the
// element stride is 272 and the element pointer is also the String pointer.
struct TokenElement {
    String text;
    char inlineBuffer[256];
};
TS3_X86_OFFSET(TokenElement, text, 0);
TS3_X86_OFFSET(TokenElement, inlineBuffer, 16);
TS3_X86_SIZE(TokenElement, 272);
struct TokenArray {
    TokenElement* begin;
    TokenElement* cur;
    TokenElement* cap;
};
enum { kString272Stride = 272, kString272Capacity = 4 };
// The same object shape, but 0x0089A3C0's element is a 16-byte String (no inline buffer).
struct StringArray {
    String* begin;
    String* cur;
    String* cap;
    const char* tag;
};

// 0x004B4250's this: two {begin, end} spans, each backed by an allocator interface whose
// single pointer is at +0x0C / +0x24. (Also used as the day pair's owner by 0x00882240? no.)
struct ScratchBlockPair {
    void* beginA;        // +0x00
    uint32_t padding04;
    void* endA;          // +0x08
    void* allocatorA;    // +0x0C
    uint32_t padding10[2];
    void* beginB;        // +0x18
    uint32_t padding1C;
    void* endB;          // +0x20
    void* allocatorB;    // +0x24

    // TS3W.exe 0x004B4250 (virtual slot 3 of the owning object).
    int FreeBlocks4B4250();
};

// {lo, hi} 64-bit pair used by the day arithmetic.
struct DayPair {
    uint32_t lo;
    uint32_t hi;

    // TS3W.exe 0x004F99E0: divide/modulo by 86400 (both flags set = no transform) then a
    // signed 64-bit three-way compare. Returns 1, 0 or -1.
    int Compare86400(const DayPair& other, uint8_t divide, uint8_t modulo) const;
};

// 0x004B4460 / 0x009C2D60's node value object. The map's tree header starts at +0x38
// (the sentinel address is object+0x38), values hang off node+0x20.
struct RecordNode {
    uint32_t padding00;
    uint32_t kind;       // +0x04, "value" nodes have 2
    String name;         // +0x08
    uint32_t padding18[8];
    uint32_t tree;       // +0x38, head of the node list

    // TS3W.exe 0x004B4460: value dword at node+0x20, 0 for the end node (and node -8).
    uint32_t MapLookup4B4460(const char* key);
};

// 0x00882240's this: a ref-counted wrapper; the wrapped pointer is at +0x0C.
struct RefHolder {
    uint32_t padding00[3];
    void* target;        // +0x0C

    // TS3W.exe 0x00882240: out[0] = target, AddRef it when set; returns out.
    void* CopyRef4_882240(void* out);
};

// 0x00992080's this: the content-patch file record (held by ContentPatchGroup::file).
struct PatchFile {
    uint32_t padding00[2];
    void* initialized;   // +0x08, null -> "not initialized"
    int32_t fnv;         // +0x0C
    uint32_t fileSize;   // +0x10
    uint32_t padding14;
    String path;         // +0x18, file name relative to the base url

    // TS3W.exe 0x00992080: check the file against fileSize/fnv. 0 = patch needed.
    uint8_t NeedToPatch992080(const void* const* services);
};

// 0x00993970 / 0x009941D0 / 0x00994C40's this: the content-patch record.
struct ContentPatchGroup {
    uint32_t padding00[4];
    DayPair date;              // +0x10, the record's day
    PatchFile* file;           // +0x18
    uint32_t padding1C[4];
    uint32_t baseUrl[2];       // +0x2C / +0x30 (begin / end)
    uint32_t versionUrl[2];    // +0x3C / +0x40
    uint32_t versionFilename[2];  // +0x4C / +0x50
    uint32_t padding54[6];
    int32_t version[4];        // +0x6C, parsed game/config version

    // TS3W.exe 0x00993970: download the patch date and compare it; 1 = ignore this patch.
    uint8_t IgnorePatchFile993970();
    // TS3W.exe 0x009941D0: file missing or ignore -> 0, else 0x00992080(file).
    uint8_t PatchCheck9941D0();
    // TS3W.exe 0x00994C40 (InitTrialData): 0 on the url checks, 1 otherwise.
    uint8_t InitTrialData994C40();
};

// 0x009B01D0 / 0x009B04C0's this: the parsed version, four dwords.
struct GameVersion {
    int32_t value[4];

    // TS3W.exe 0x009B01D0: parse "a.b.c.d"; returns the token count or the 4th build.
    int ParseVersion9B01D0(const char* src);
    // TS3W.exe 0x009B04C0: ParseVersion9B01D0(src); returns this.
    GameVersion* AssignVersion9B04C0(const char* src);
};

// 0x009C2D60's this (only forwarded to the recursion).
struct RecordTreeWalker {
    uint32_t opaque;

    // TS3W.exe 0x009C2D60: walk the list at a2+0x24, append {x,y,z64} records to a3.
    void* WalkRecordTree9C2D60(int32_t a2, int32_t a3);
};

// TS3W.exe 0x00994EC0: vtable target: InitTrialData on the global Services instance
// (0x11E9640 - 8) when one exists; 0 otherwise.
uint8_t __cdecl InitTrialDataGlobal994EC0();

// TS3W.exe 0x0099F0E0: splits the string a1 on any of the bytes in a3 into a2 (272-byte
// String elements) and stops after 4 tokens. Returns the last match index, -1, or (on
// the 4-token path) 2021161081 * byte size of a2 as a 32-bit quirk.
int __cdecl SplitTokens99F0E0(String* a1, TokenArray* a2, const char* a3);

// TS3W.exe 0x009B0910: parses the configured VersionString into a1 and folds the
// configured build number into a1[3]; returns a1.
int32_t* __cdecl ConfigVersion9B0910(int32_t* a1);

}  // namespace ts3::scratch