// Reimplementation of CAS cluster unit clu_5a37e0_CAS (TS3W.exe 0x005A37E0..0x005AEFFB):
// the "CAS/CacheMap" hash map constructor (0x005A3EA0), the part-key matcher
// (0x005A37E0), the cached-part search 0x005A44A0 (the "CAS/CASCache/GetCachedPart" path)
// and the two-map selector 0x005AEF40. Layout and ABI notes in the header; the differential
// test lives in src/exehost/clu_5a37e0_CAS_test.cpp.
//
// Dependencies still in the game, called through ts3/layout.h:
//   0x005AEDF0  build the two maps (unit cls_5ae460_CAS)
//   0x004A3890  deep part-key compare (unit clu_49d1d0_x)
//   0x004E3F70  operator delete (unit clu_4e3f70_Scratch)
//   0x004FA800/0x004FA840  the LCG objects at 0x011CE400/0x011CE430: seed / rand(n)
//               (units clu_4fa800_Routing and clu_4fa840_World)
//   0x005A4470  vector<CacheMapEntry*>::push_back (unit clu_59ead0_World)
//   0x008621C0  vector<CacheMapEntry*>::insert for the growing case (not part of a unit)
#include "ts3/cas/clu_5a37e0_CAS.h"

#include <cstring>

namespace ts3::cas {
namespace {

// Payload dword indices (payload[0] is at entry +0x10).
constexpr int kPayloadType = 0;      // 0xCF00 type bits; 0 reads as 0x0100
constexpr int kPayloadPartType = 1;  // compared with the `partType` argument
constexpr int kPayloadMask = 2;      // 0xFFFFFFFF = no mask
constexpr int kPayloadKey = 8;       // four dwords: the "part" key

// The two RNG objects used by this unit (the game's rand object: state at +0, seeded bit +4).
enum RngId { kRngCachedPart, kRngSelector };

// vector<CacheMapEntry*>: begin/end/capacity, the game's 12-byte vector.
struct EntryVector {
    CacheMapEntry** begin;
    CacheMapEntry** end;
    CacheMapEntry** capacity;
};
TS3_X86_SIZE(EntryVector, 12);

#if defined(_M_IX86)
constexpr uintptr_t kRngAddress[2] = {0x011CE400, 0x011CE430};
constexpr uintptr_t kEmptyBuckets = 0x0114C900;  // read-only empty map bucket array
constexpr uintptr_t kCacheMapName = 0x00FDE5C4;  // "CAS/CacheMap"

uint32_t& GameDword(uintptr_t address) { return *reinterpret_cast<uint32_t*>(address); }

// The lazy "seed once" the game does before the first rand call; the OR lands in the state
// object's +4 dword, exactly like EA's the original instruction sequence.
void SeedRng(RngId rng)
{
    const uintptr_t address = kRngAddress[rng];
    if ((GameDword(address + 4) & 1) == 0) {
        GameDword(address + 4) |= 1;
        CallGameMethod<uint32_t, 0x004FA800>(reinterpret_cast<void*>(address), -1);
    }
}

uint32_t NextRandom(RngId rng, uint32_t bound)
{
    return CallGameMethod<uint32_t, 0x004FA840>(reinterpret_cast<void*>(kRngAddress[rng]), bound);
}

void FreeCandidateBuffer(void* p)
{
    if (p)
        CallGameFunction<uint32_t, 0x004E3F70>(p);
}

// The append from the 0x40000 retry path: 0x005A4470 itself checks the capacity and grows
// through 0x008621C0, so it must be the game's code (same allocator).
void PushCandidateDirect(EntryVector& vector, CacheMapEntry* value)
{
    CacheMapEntry* slot = value;
    CallGameMethod<int, 0x005A4470>(&vector, &slot);
}

// The append of the ordinary path: the inline fast path, 0x008621C0 (vector<T>::insert)
// when the vector is full.
void PushCandidate(EntryVector& vector, CacheMapEntry* value)
{
    if (reinterpret_cast<uintptr_t>(vector.end) >= reinterpret_cast<uintptr_t>(vector.capacity)) {
        CacheMapEntry* slot = value;
        CallGameMethod<int, 0x008621C0>(&vector, vector.end, &slot);
    } else {
        CacheMapEntry** where = vector.end;
        vector.end = where + 1;
        if (where)
            *where = value;
    }
}
#else
void SeedRng(RngId) { AbortNotRewritten("the CAS cache RNG"); }
uint32_t NextRandom(RngId, uint32_t) { AbortNotRewritten("the CAS cache RNG"); return 0; }
void FreeCandidateBuffer(void*) {}
void PushCandidateDirect(EntryVector&, CacheMapEntry*) { AbortNotRewritten("a CAS cache vector"); }
void PushCandidate(EntryVector&, CacheMapEntry*) { AbortNotRewritten("a CAS cache vector"); }
#endif

}  // namespace

// TS3W.exe 0x005A37E0, cdecl ret 0. Not a 16-bit function despite IDA's `unsigned __int16`
// locals: every test is 32-bit, and the result only depends on the low 16 bits, so the
// zero-extension of a1/a2 there is invisible.
bool PartKeyMatches(uint32_t a, uint32_t b)
{
    if ((b & ~a & 0x307Fu) != 0)
        return false;
    const uint32_t shiftedA = (a & 0xCF00u) != 0 ? a : (a | 0x100u);
    const uint32_t shiftedB = (b & 0xCF00u) != 0 ? b : (b | 0x100u);
    return ((shiftedA ^ shiftedB) & 0xCF00u) == 0;
}

// TS3W.exe 0x005A3EA0, thiscall ret 0. Writes +0x04..+0x27 and returns `this`; +0x00 is
// left for the deriving constructor.
CacheMap* CacheMap::Construct()
{
    weight10 = 1.0f;  // 0x0107A538
    weight14 = 2.0f;  // 0x00F98A58
    bucketCount = 1;
#if defined(_M_IX86)
    buckets = reinterpret_cast<CacheMapEntry**>(kEmptyBuckets);
    name = reinterpret_cast<const char*>(kCacheMapName);
#else
    static CacheMapEntry* const kEmpty[2] = {nullptr, reinterpret_cast<CacheMapEntry*>(kNoEntry32)};
    buckets = const_cast<CacheMapEntry**>(kEmpty);
    name = "CAS/CacheMap";
#endif
    count0C = 0;
    field18 = 0;
    field20 = 0;
    field24 = 0;
    return this;
}

// TS3W.exe 0x005A44A0, thiscall ret 0x1C (7 stack args).
uint8_t CacheMap::GetCachedPart(const uint32_t* key, uint32_t type0, uint32_t type1,
                                uint32_t partType, uint32_t exact, uint32_t* outKey,
                                void* outPayload)
{
    const uint32_t highBits = type1 & 0x1E000000u;  // v63
    const uint32_t typeBits = type0 & 0xCF00u;      // v62
    const uint32_t key2 = key[2];                   // only read when exact == 0

    // --- the bucket walk: buckets[0..count-1] are heads, buckets[count] the sentinel ----
    // The sentinel is the *value* 0xFFFFFFFF (never a real entry), so "chain ended" walks
    // the array forward until it reads it. `cursor` is the first non-null slot reached.
    CacheMapEntry** const buckets = this->buckets;
    CacheMapEntry** cursor = buckets;  // v65
    CacheMapEntry* i = *buckets;
    if (!i) {
        ++cursor;
        while (!*cursor)
            ++cursor;
        i = *cursor;
    }
    CacheMapEntry* const noEntry = buckets[this->bucketCount];  // v66

    // --- phase 1, the exact search used when the caller passes exact == 0 -----------------
    if (static_cast<uint8_t>(exact) == 0) {
        if (key2 == 0) {
            if (partType != 53 && partType != 54) {
                if (i == noEntry)
                    goto collect;
                {
                    const uint32_t wantMask = type1;  // v13
                    while (true) {
                        if (PartKeyMatches(i->payload[kPayloadType], type0) &&
                            i->payload[kPayloadPartType] == partType) {
                            const uint32_t mask = i->payload[kPayloadMask];
                            if (mask == 0xFFFFFFFFu || (wantMask & mask) == wantMask)
                                goto found;
                        }
                        i = i->next;
                        if (!i) {
                            do {
                                ++cursor;
                            } while (!*cursor);
                            i = *cursor;
                        }
                        if (i == noEntry)
                            goto collect;
                    }
                }
            }
            // Part type 53/54 also accepts the neighbouring type 0x400/0x500, the wildcard
            // 0xCF00, and either the exact high bits or the full 0x1E000000 mask.
            if (i == noEntry)
                goto collect;
            while (true) {
                const uint32_t entryType = i->payload[kPayloadType] & 0xCF00u;
                bool typeOk = typeBits == entryType;
                if (!typeOk) {
                    if (entryType == 0x400u)
                        typeOk = typeBits == 0x500u;
                    else if (entryType == 0x500u)
                        typeOk = typeBits == 0x400u;
                    else if (entryType == 0xCF00u)
                        typeOk = true;
                }
                if (typeOk && i->payload[kPayloadPartType] == partType &&
                    (highBits == (i->payload[kPayloadMask] & 0x1E000000u) ||
                     highBits == 0x1E000000u)) {
                    goto found;
                }
                i = i->next;
                if (!i) {
                    do {
                        ++cursor;
                    } while (!*cursor);
                    i = *cursor;
                }
                if (i == noEntry)
                    goto collect;
            }
        }
        // key2 != 0: the four payload key dwords, with 0x004A3890 as the deep fallback.
        if (i != noEntry) {
            while (true) {
                const uint32_t entryType = i->payload[kPayloadType];
                if (((type0 & entryType) & 0x307Fu) == (type0 & 0x307Fu)) {
                    uint32_t entryAdjusted = entryType;
                    uint32_t wantAdjusted = type0;
                    if ((entryType & 0xCF00u) == 0)
                        entryAdjusted |= 0x100u;
                    if (typeBits == 0)
                        wantAdjusted = type0 | 0x100u;
                    if (((wantAdjusted ^ entryAdjusted) & 0xCF00u) == 0 &&
                        i->payload[kPayloadPartType] == partType) {
                        const uint32_t mask = i->payload[kPayloadMask];
                        const bool maskOk = mask == 0xFFFFFFFFu || (type1 & mask) == type1;
                        const bool keyOk =
                            (i->payload[kPayloadKey] == key[0] &&
                             i->payload[kPayloadKey + 2] == key2 &&
                             i->payload[kPayloadKey + 3] == key[3] &&
                             i->payload[kPayloadKey + 1] == key[1]) ||
                            CallGameFunction<uint8_t, 0x004A3890>(i, key) != 0;
                        if (maskOk && keyOk)
                            goto found;
                    }
                }
                i = i->next;
                if (!i) {
                    do {
                        ++cursor;
                    } while (!*cursor);
                    i = *cursor;
                }
                if (i == noEntry)
                    break;
            }
        }
    }

collect:
    // --- phase 2: collect every candidate, then let the RNG pick one ----------------------
    {
        EntryVector vector{};
        uint32_t mask1 = type1;  // `a4` is live across iterations: the 0x40000 retry rewrites it
        if (key2 == 0) {
            if (partType != 53 && partType != 54) {
                if (i != noEntry) {
                    const uint32_t wantBits = type0 & 0x307Fu;
                    while (true) {
                        const uint32_t entryType = i->payload[kPayloadType];
                        if (((type0 & entryType) & 0x307Fu) == wantBits) {
                            uint32_t entryAdjusted = entryType;
                            uint32_t wantAdjusted = type0;
                            if ((entryType & 0xCF00u) == 0)
                                entryAdjusted |= 0x100u;
                            if (typeBits == 0)
                                wantAdjusted = type0 | 0x100u;
                            if (((wantAdjusted ^ entryAdjusted) & 0xCF00u) == 0 &&
                                i->payload[kPayloadPartType] == partType) {
                                const uint32_t mask = i->payload[kPayloadMask];
                                if (mask == 0xFFFFFFFFu || (mask1 & mask) == mask1) {
                                    PushCandidate(vector, i);
                                    goto advance1;
                                }
                                // EA retries with the cache-coherency bits folded in when
                                // the caller asks for a non-index part (partType outside 4..7).
                                if ((mask1 & 0x40000u) != 0 && (partType < 4 || partType > 7)) {
                                    mask1 = (mask1 & 0xFFFBFFFDu) | 2u;
                                    if ((mask1 & mask) == mask1)
                                        PushCandidateDirect(vector, i);
                                }
                            }
                        }
                    advance1:
                        i = i->next;
                        if (!i) {
                            do {
                                ++cursor;
                            } while (!*cursor);
                            i = *cursor;
                        }
                        if (i == noEntry)
                            goto finish;
                    }
                }
                return 0;
            }
            // Part type 53/54 candidates (same type pairing as phase 1).
            if (i != noEntry) {
                while (true) {
                    const uint32_t entryType = i->payload[kPayloadType] & 0xCF00u;
                    bool typeOk = typeBits == entryType;
                    if (!typeOk) {
                        if (entryType == 0x400u)
                            typeOk = typeBits == 0x500u;
                        else if (entryType == 0x500u)
                            typeOk = typeBits == 0x400u;
                        else if (entryType == 0xCF00u)
                            typeOk = true;
                    }
                    bool push = false;
                    if (typeOk && i->payload[kPayloadPartType] == partType &&
                        highBits == (i->payload[kPayloadMask] & 0x1E000000u))
                        push = true;
                    else if (highBits == 0x1E000000u)
                        push = true;
                    if (push)
                        PushCandidate(vector, i);
                    i = i->next;
                    if (!i) {
                        do {
                            ++cursor;
                        } while (!*cursor);
                        i = *cursor;
                    }
                    if (i == noEntry)
                        goto finish;
                }
            }
            return 0;
        }
        // key2 != 0: the payload key, then the entry's own four key dwords. The two
        // comparisons use different dword orders - an EA quirk kept.
        if (i == noEntry)
            return 0;
        while (true) {
            const uint32_t entryType = i->payload[kPayloadType];
            bool matches = false;
            if (((type0 & entryType) & 0x307Fu) == (type0 & 0x307Fu)) {
                uint32_t entryAdjusted = entryType;
                uint32_t wantAdjusted = type0;
                if ((entryType & 0xCF00u) == 0)
                    entryAdjusted |= 0x100u;
                if (typeBits == 0)
                    wantAdjusted = type0 | 0x100u;
                if (((wantAdjusted ^ entryAdjusted) & 0xCF00u) == 0 &&
                    i->payload[kPayloadPartType] == partType) {
                    const uint32_t mask = i->payload[kPayloadMask];
                    if (mask == 0xFFFFFFFFu || (mask1 & mask) == mask1) {
                        matches = i->payload[kPayloadKey] == key[0] &&
                                  i->payload[kPayloadKey + 2] == key2 &&
                                  i->payload[kPayloadKey + 3] == key[3] &&
                                  i->payload[kPayloadKey + 1] == key[1];
                        if (!matches)
                            matches = i->key[0] == key[0] && i->key[2] == key2 &&
                                      i->key[3] == key[3] && i->key[1] == key[1];
                    }
                }
            }
            if (matches)
                PushCandidate(vector, i);
            i = i->next;
            if (!i) {
                do {
                    ++cursor;
                } while (!*cursor);
                i = *cursor;
            }
            if (i == noEntry)
                goto finish;
        }

    finish:
        if (vector.begin == vector.end) {
            FreeCandidateBuffer(vector.begin);
            return 0;
        }
        // Pick one candidate with the RNG and re-find it in its own bucket (the hash below
        // is the one the game's insert path uses); the sentinel slot is what the walk falls
        // back to - the same node pointer the walks above use.
        SeedRng(kRngCachedPart);
        const uint32_t count = static_cast<uint32_t>(vector.end - vector.begin);
        CacheMapEntry* const picked = vector.begin[NextRandom(kRngCachedPart, count)];
        const uint32_t bucket = (picked->key[0] ^ picked->key[3]) % this->bucketCount;
        CacheMapEntry* walk = this->buckets[bucket];
        if (walk) {
            while (walk->key[0] != picked->key[0] || walk->key[2] != picked->key[2] ||
                   walk->key[3] != picked->key[3] || walk->key[1] != picked->key[1]) {
                walk = walk->next;
                if (!walk)
                    break;
            }
        }
        if (!walk)
            walk = this->buckets[this->bucketCount];
        std::memcpy(outKey, walk->key, sizeof walk->key);
        std::memcpy(outPayload, walk->payload, sizeof walk->payload);
        FreeCandidateBuffer(vector.begin);
        return 1;
    }

found:
    std::memcpy(outKey, i->key, sizeof i->key);
    std::memcpy(outPayload, i->payload, sizeof i->payload);
    return 1;
}

// TS3W.exe 0x005AEF40, thiscall ret 0x1C (7 stack args).
uint8_t CasPartCache::GetPart(const uint32_t* key, uint32_t type0, uint32_t type1,
                              uint32_t partType, uint32_t exact, uint32_t* outKey,
                              void* outPayload)
{
    if (field78 == 0)
        CallGameMethod<uint32_t, 0x005AEDF0>(this);

    CacheMap* first;
    CacheMap* second;
    if (static_cast<uint8_t>(exact) != 0) {
        // Map B is picked with probability countA / (countA + countB).
        SeedRng(kRngSelector);
        const uint32_t pick = NextRandom(kRngSelector, mapB.count0C + mapA.count0C);
        if (pick >= mapB.count0C) {
            first = &mapA;
            second = &mapB;
        } else {
            first = &mapB;
            second = &mapA;
        }
    } else {
        first = &mapB;
        second = &mapA;
    }
    if (first->GetCachedPart(key, type0, type1, partType, exact, outKey, outPayload) != 0)
        return 1;
    return second->GetCachedPart(key, type0, type1, partType, exact, outKey, outPayload) != 0;
}

}  // namespace ts3::cas