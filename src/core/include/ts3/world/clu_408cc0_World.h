#pragma once
#include "ts3/layout.h"

namespace ts3::world408cc0 {
struct WorldString {
    const uint16_t* begin;
    const uint16_t* end;
    const uint16_t* capacity;
    const void* allocator;
};
TS3_X86_SIZE(WorldString, 16);
TS3_X86_OFFSET(WorldString, end, 4);
struct WorldKey { uint32_t word[4]; };
TS3_X86_SIZE(WorldKey, 16);
struct WorldOrderA {
    WorldKey key[4];
    // TS3W.exe 0x00A517B0: compare keys in order 0, 3, 1, 2.
    bool Less(const WorldOrderA* other) const;
    bool CompareBuffer(const void* other) const { return Less(static_cast<const WorldOrderA*>(other)); }
};
struct WorldOrderB {
    WorldKey key[4];
    // TS3W.exe 0x00A8C8A0: compare keys in order 0, 1, 2, 3.
    bool Less(const WorldOrderB* other) const;
    bool CompareBuffer(const void* other) const { return Less(static_cast<const WorldOrderB*>(other)); }
};
TS3_X86_SIZE(WorldOrderA, 64);
TS3_X86_SIZE(WorldOrderB, 64);
// TS3W.exe 0x00408CC0: compare the byte spans of two UTF-16 string objects (not NUL terminated).
int EqualWorldString(const WorldString* a, const WorldString* b);
// TS3W.exe 0x004A3A50: unsigned key ordering, word priority 0, 1, 3, 2.
int LessWorldKey(const WorldKey* a, const WorldKey* b);
// TS3W.exe 0x00594CF0: find the first matching string in a contiguous range.
WorldString* FindWorldString(WorldString* begin, WorldString* end, const WorldString* key);
struct WorldReference {
    void* vtable;
    uint8_t pad[576];
    uint32_t refs;
};
TS3_X86_OFFSET(WorldReference, refs, 580);
struct WorldOwnedString {
    WorldString key;
    WorldReference* value;
    // TS3W.exe 0x00408DD0: release the referenced value and any heap string storage.
    void* Release();
};
struct WorldStringNode {
    WorldString key;
    void* value;
    WorldStringNode* next;
};
TS3_X86_OFFSET(WorldStringNode, next, 20);
struct WorldInsertResult;
struct WorldIterator { WorldStringNode* node; WorldStringNode** bucket; };
struct WorldStringTable {
    void* unused;
    WorldStringNode** buckets;
    uint32_t bucketCount;
    uint32_t size;
    uint32_t policy[3];
    void* allocator;
    uint32_t flags;
    const char* name;
    // TS3W.exe 0x00409320: insert a string/reference pair, preserving collision order.
    WorldInsertResult* Insert(WorldInsertResult* out, const WorldOwnedString* pair, uint32_t ignored);
    // TS3W.exe 0x004094D0: get or default-insert a string/reference mapping.
    WorldReference** Get(const WorldString* key);
    // TS3W.exe 0x004091F0: look up the FNV-1 hash bucket, falling back to the sentinel.
    WorldIterator* Find(WorldIterator* out, const WorldString* key) const;
    void* CompareFind(void* out, const void* key) const { return Find(static_cast<WorldIterator*>(out), static_cast<const WorldString*>(key)); }
};
TS3_X86_OFFSET(WorldStringTable, buckets, 4);
TS3_X86_OFFSET(WorldStringTable, bucketCount, 8);
struct WorldChainNode {
    WorldString key;
    uint8_t remainder[584];
    WorldChainNode* next;
};
TS3_X86_OFFSET(WorldChainNode, next, 600);
// TS3W.exe 0x009A3900: find the first matching linked node; the third argument is ignored.
#if defined(_M_IX86)
WorldChainNode* __stdcall FindWorldChain(WorldChainNode* head, const WorldString* key, void* ignored);
#else
WorldChainNode* FindWorldChain(WorldChainNode* head, const WorldString* key, void* ignored);
#endif
struct WorldNameOwner {
    uint8_t prefix[132];
    WorldString names[2];
    uint8_t gap[40];
    uint32_t flags;
    // TS3W.exe 0x0085A760: clear dirty bit 1 on name changes, then assign both strings.
    void* SetNames(const WorldString* pair);
};
TS3_X86_OFFSET(WorldNameOwner, names, 132);
TS3_X86_OFFSET(WorldNameOwner, flags, 204);
struct WorldProductName {
    uint8_t prefix[40];
    uint32_t hashLow, hashHigh;
    uint8_t gap[8];
    WorldString name;
    // TS3W.exe 0x00C93410: assign the inline name for the FNV empty key, else localize the hash.
    uint8_t GetName(WorldString* out) const;
};
TS3_X86_OFFSET(WorldProductName, name, 56);
// TS3W.exe 0x004A4290: order two string ranges using the game's locale comparison.
bool LessWorldString(const WorldString* a, const WorldString* b);
struct WorldInsertResult { WorldStringNode* node; WorldStringNode** bucket; uint8_t inserted; };
TS3_X86_SIZE(WorldOwnedString,20);
TS3_X86_SIZE(WorldStringNode,24);
TS3_X86_SIZE(WorldIterator,8);
TS3_X86_SIZE(WorldInsertResult,12);
TS3_X86_OFFSET(WorldInsertResult, inserted, 8);
TS3_X86_SIZE(WorldStringTable,40);
struct WorldPairNode { WorldString key; WorldString value; WorldPairNode* next; };
// Nested hashtable copied by 0x0058BFF0: its pointers must grow in x64 builds.
struct WorldRecordValue {
    void* unused;
    void** buckets;
    uint32_t bucketCount, size, policy[3];
    void* allocator;
    uint32_t flags;
    const char* name;
};
TS3_X86_SIZE(WorldRecordValue,40);
struct WorldRecordNode { WorldString key; WorldRecordValue value; WorldRecordNode* next; };
TS3_X86_OFFSET(WorldPairNode, next, 32);
TS3_X86_OFFSET(WorldRecordNode, next, 56);
template<class Node> struct WorldNodeResult { Node* node; Node** bucket; uint8_t inserted; };
template<class Node> struct WorldNodeTable {
    void* unused;
    Node** buckets;
    uint32_t bucketCount, size;
    uint32_t policy[3];
    void* allocator;
    uint32_t flags;
    const char* name;
};
struct WorldPairTable : WorldNodeTable<WorldPairNode> {
    // TS3W.exe 0x0058B8A0: rehash the string-pair table, preserving EA's return quirk.
    uint32_t Rehash(uint32_t count);
    // TS3W.exe 0x0058C240: insert a pair of strings, or return the existing node.
    WorldNodeResult<WorldPairNode>* Insert(WorldNodeResult<WorldPairNode>* out, const WorldPairNode* pair, uint32_t ignored);
};
struct WorldRecordTable : WorldNodeTable<WorldRecordNode> {
    // TS3W.exe 0x0058B980: rehash the string/record table, preserving EA's return quirk.
    uint32_t Rehash(uint32_t count);
    // TS3W.exe 0x0058C710: insert a string/record pair, or return the existing node.
    WorldNodeResult<WorldRecordNode>* Insert(WorldNodeResult<WorldRecordNode>* out, const WorldRecordNode* pair, uint32_t ignored);
};
TS3_X86_SIZE(WorldPairTable, 40);
TS3_X86_SIZE(WorldRecordTable, 40);
// TS3W.exe 0x0044C340: register English defaults when the active locale matches the default.
uint8_t EnsureWorldLocale();
// TS3W.exe 0x0044C390: restore the default locale and clear both localization caches.
uint8_t ResetWorldLocale();
struct WorldProduct {
    void* vtable;
    uint32_t unknown;
    WorldKey key;
    uint8_t gap24[16];
    uint32_t hashLow, hashHigh;
    uint8_t gap48[8];
    WorldString name;
    uint8_t gap72[16];
    float price;
    uint8_t gap92[12];
    int32_t priority;
    uint8_t gap108[16];
    int32_t order;
    uint8_t gap128[56];
    uint64_t tags[3];
    uint32_t tag32;
};
TS3_X86_OFFSET(WorldProduct, priority, 104);
TS3_X86_OFFSET(WorldProduct, order, 124);
TS3_X86_OFFSET(WorldProduct, tags, 184);
TS3_X86_OFFSET(WorldProduct, tag32, 208);
struct WorldProductFilter { uint32_t unused[2]; uint64_t masks[3]; uint32_t mask32; };
TS3_X86_OFFSET(WorldProductFilter, masks, 8);
TS3_X86_OFFSET(WorldProductFilter, mask32, 32);
// TS3W.exe 0x00CE8360: product comparison; this is the portable body, not the EAX/ECX/EDI shim.
uint8_t LessWorldProduct(const WorldProduct* a, const WorldProductFilter* filter, const WorldProduct* b, int32_t mode);
#if defined(_M_IX86)
// TS3W.exe 0x00CE8360: original usercall ABI adapter (EAX=a, ECX=filter, EDI=b, caller pops mode).
void LessWorldProductUsercall();
#endif
// TS3W.exe 0x00CE8980: stable merge product-pointer ranges into the destination.
WorldProduct** MergeWorldProducts(WorldProduct** a, WorldProduct** ae, WorldProduct** b, WorldProduct** be,
    WorldProduct** out, int32_t mode, const WorldProductFilter* filter);
// TS3W.exe 0x00CE9490: stable merge-sort products with alternating source and scratch ranges.
uintptr_t SortWorldProducts(WorldProduct** begin, WorldProduct** end, WorldProduct** scratch, int32_t mode, const WorldProductFilter* filter);
struct WorldSortEntry { WorldProduct* product; uint8_t data[108]; uint32_t order; };
TS3_X86_SIZE(WorldSortEntry,116);
// TS3W.exe 0x005DE510: introsort partition stage; EA's self-key comparisons deliberately remain ties.
intptr_t PartitionWorldEntries(WorldSortEntry* begin, WorldSortEntry* end, int32_t depth, uint32_t context);
// Conversion-only adapters for hookgen's void-buffer and scalar-pointer descriptors.
namespace compare {
inline int Equal(const void* a,const void* b) { return EqualWorldString(static_cast<const WorldString*>(a),static_cast<const WorldString*>(b)); }
inline int Key(const void* a,const void* b) { return LessWorldKey(static_cast<const WorldKey*>(a),static_cast<const WorldKey*>(b)); }
#if defined(_M_IX86)
// Ranges and linked nodes are read-only shared pointees, NOT fixed-size copied buffers.
// Keep their pointer arguments and returns as raw words to avoid incorrect remapping
// when a search result aliases the copied key object.
inline uint32_t Range(uint32_t a,uint32_t end,const void* key) {
    return uint32_t(reinterpret_cast<uintptr_t>(FindWorldString(reinterpret_cast<WorldString*>(uintptr_t(a)),reinterpret_cast<WorldString*>(uintptr_t(end)),static_cast<const WorldString*>(key))));
}
inline uint32_t Chain(uint32_t a,const void* key,uint32_t ignored) {
    return uint32_t(reinterpret_cast<uintptr_t>(FindWorldChain(reinterpret_cast<WorldChainNode*>(uintptr_t(a)),static_cast<const WorldString*>(key),reinterpret_cast<void*>(uintptr_t(ignored)))));
}
#endif
} // namespace compare
} // namespace ts3::world408cc0
