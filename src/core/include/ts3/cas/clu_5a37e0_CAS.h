// CAS part cache (cluster unit clu_5a37e0_CAS, TS3W.exe 0x005A37E0..0x005AEFFB).
//
// The unit is the hash map behind "CAS/CacheMap" plus its key matcher and one selector
// method of the class that owns two such maps:
//   0x005A37E0  bool PartKeyMatches(uint32_t a, uint32_t b)      (cdecl, leaf)
//   0x005A3EA0  CacheMap* CacheMap::Construct()                  (thiscall, leaf)
//   0x005A44A0  bool CacheMap::GetCachedPart(...)                (thiscall, ret 0x1c)
//   0x005AEF40  bool CasPartCache::GetPart(...)                  (thiscall, ret 0x1c)
//
// CacheMap's bucket array (field `buckets`) holds `bucketCount` bucket heads followed by
// one slot whose value is the sentinel 0xFFFFFFFF (`kNoEntry`); the constructors point a
// fresh, empty map at the read-only array 0x0114C900 = { 0, 0xFFFFFFFF, ... } with
// bucketCount = 1. Entries are 0x44 bytes: four key dwords, a 0x30-byte payload (the first
// four payload dwords are the type/mask fields the search filters on, the last four are
// the "part" key) and a next pointer at +0x40.
#pragma once

#include <cstdint>

#include "ts3/layout.h"

namespace ts3::cas {

// The sentinel value stored in the bucket array slot after the last bucket; it is never
// dereferenced (the walks compare an entry pointer against it).
constexpr uint32_t kNoEntry32 = 0xFFFFFFFFu;

struct CacheMapEntry;
struct CasPartCache;

// One cache-map record (0x44 bytes).
struct CacheMapEntry {
    uint32_t key[4];              // +0x00, copied to the caller's outKey
    uint32_t payload[12];         // +0x10: 0x30 bytes; [0]=type flags, [1]=part type,
                                  // [2]=mask, [8..11]=the "part" key compared with `key`
    CacheMapEntry* next;          // +0x40, chain inside one bucket, 0 ends the chain
};
TS3_X86_OFFSET(CacheMapEntry, key, 0x00);
TS3_X86_OFFSET(CacheMapEntry, payload, 0x10);
TS3_X86_OFFSET(CacheMapEntry, next, 0x40);
TS3_X86_SIZE(CacheMapEntry, 0x44);

// The "CAS/CacheMap" object (0x28 bytes; 0x005A3EA0 initializes +0x04..+0x27).
struct CacheMap {
    void* field00;                // +0x00, written by the deriving constructor
    CacheMapEntry** buckets;      // +0x04, bucketCount heads + the 0xFFFFFFFF sentinel
    uint32_t bucketCount;         // +0x08
    uint32_t count0C;             // +0x0C, entry count used for the weighted pick
    float weight10;               // +0x10 = 1.0f (0x0107A538)
    float weight14;               // +0x14 = 2.0f (0x00F98A58)
    uint32_t field18;             // +0x18 = 0
    const char* name;             // +0x1C = "CAS/CacheMap" (0x00FDE5C4)
    uint32_t field20;             // +0x20 = 0
    uint32_t field24;             // +0x24 = 0

    // TS3W.exe 0x005A3EA0, thiscall ret 0. Returns `this`. Every field except +0x00 is
    // written; the object is at least 0x28 bytes.
    CacheMap* Construct();

    // TS3W.exe 0x005A44A0, thiscall ret 0x1c (7 stack args). Returns 1 and fills outKey
    // (4 dwords) and outPayload (0x30 bytes) when a part was found (either an exact match,
    // or one of the collected candidates picked with the RNG at 0x011CE400 - which is a
    // side effect this function cannot skip), 0 when the map has no candidate. Note the
    // bucket walk: the sentinel is the *value* 0xFFFFFFFF, so the "bucket exhausted" step
    // walks the array until it reads it.
    uint8_t GetCachedPart(const uint32_t* key, uint32_t type0, uint32_t type1, uint32_t partType,
                          uint32_t exact, uint32_t* outKey, void* outPayload);
};
TS3_X86_OFFSET(CacheMap, buckets, 0x04);
TS3_X86_OFFSET(CacheMap, bucketCount, 0x08);
TS3_X86_OFFSET(CacheMap, count0C, 0x0C);
TS3_X86_OFFSET(CacheMap, name, 0x1C);
TS3_X86_SIZE(CacheMap, 0x28);

// TS3W.exe 0x005A37E0, cdecl ret 0. EA's key-compatibility test: the "bucket" bits are
// 0x307F and the type bits 0xCF00; a zero type field is read as 0x0100 on both sides. All
// tests are 32-bit (no 16-bit truncation of the result).
bool PartKeyMatches(uint32_t a, uint32_t b);

// The class holding the two maps 0x005AEF40 selects between (map A at +0x28, map B at
// +0x50; 0x005AEDF0 - another unit - materializes them the first time).
struct CasPartCache {
    uint8_t field00[0x28];
    CacheMap mapA;                // +0x28
    CacheMap mapB;                // +0x50
    uint8_t field78;              // +0x78: nonzero = the two maps are already built

    // TS3W.exe 0x005AEF40, thiscall ret 0x1c (7 stack args). Builds the maps when +0x78 is
    // clear, then, when `exact`'s low byte is set, seeds the RNG at 0x011CE430 (the
    // RNG_SEEDED bit is OR-ed in before the call, EA style) and asks GetCachedPart on one
    // of the two maps - the one picked by rand(countA + countB) >= countB. The second map
    // is only consulted when the first missed. Returns 1/0.
    uint8_t GetPart(const uint32_t* key, uint32_t type0, uint32_t type1, uint32_t partType,
                    uint32_t exact, uint32_t* outKey, void* outPayload);
};
TS3_X86_OFFSET(CasPartCache, mapA, 0x28);
TS3_X86_OFFSET(CasPartCache, mapB, 0x50);
TS3_X86_OFFSET(CasPartCache, field78, 0x78);

}  // namespace ts3::cas