#pragma once
#include "ts3/layout.h"
namespace ts3::misc {
struct WideRange {
    const uint16_t* begin;
    const uint16_t* end;
    // TS3W.exe 0x009D1960: compare a bounded substring with a zero-terminated string.
    int CompareSubstring(uint32_t offset, uint32_t count, const uint16_t* text) const;
    int HookCompareSubstring(uint32_t offset, uint32_t count, void* text) const { return CompareSubstring(offset, count, static_cast<const uint16_t*>(text)); }
};
TS3_X86_SIZE(WideRange, 8);
TS3_X86_OFFSET(WideRange, end, 4);
// TS3W.exe 0x00446750: test zero-terminated UTF16 against a range.
int EqualWide(const uint16_t* text, const WideRange* range);
// TS3W.exe 0x00446820: unsigned UTF16 lexicographic prefix comparison.
int CompareWideN(const uint16_t* a, const uint16_t* b, uint32_t count);
// TS3W.exe 0x00446B50: compare two UTF16 ranges, then their signed lengths.
int CompareWide(const uint16_t* a, const uint16_t* ae, const uint16_t* b, const uint16_t* be);
struct WideString { const uint16_t* begin; const uint16_t* end; const uint16_t* capacity; uint32_t flags; const uint16_t* local; uint8_t payload[32]; };
TS3_X86_SIZE(WideString, 52);
TS3_X86_OFFSET(WideString, capacity, 8);
TS3_X86_OFFSET(WideString, local, 16);
struct WideNode { WideNode* right; WideNode* left; WideNode* parent; uint32_t color; WideString key; WideString value; };
TS3_X86_OFFSET(WideNode, key, 16);
TS3_X86_OFFSET(WideNode, value, 68);
TS3_X86_SIZE(WideNode, 120);
struct WideTree {
    void* allocator0; WideNode* sentinel[3]; uint32_t unused; uint32_t count; void* allocator;
    // TS3W.exe 0x00446D60: lower bound in the reverse-link tree.
    WideNode** LowerBound(WideNode** out, const WideRange* key);
    void* HookLowerBound(void* out, void* key) { return LowerBound(static_cast<WideNode**>(out), static_cast<const WideRange*>(key)); }
    // TS3W.exe 0x00449650: recursively release right trees then iterate left links.
    void DestroyNodes(WideNode* node);
    // TS3W.exe 0x004A47D0: allocate/copy/link a tree node through original dependencies.
    WideNode** Insert(WideNode** out, WideNode* parent, const WideRange* key, uint8_t forceLeft);
};
TS3_X86_OFFSET(WideTree, sentinel, 4);
TS3_X86_OFFSET(WideTree, count, 20);
TS3_X86_OFFSET(WideTree, allocator, 24);
TS3_X86_SIZE(WideTree, 28);
struct HashNode { void* key; uint32_t hash; HashNode* next; };
struct HashIterator { HashNode* node; HashNode** bucket; };
struct WideHash {
    void* words[3]; uint32_t count; void* words10[3]; void* allocator;
    // TS3W.exe 0x0044C6F0: free each bucket chain and clear bucket heads; no defined return.
    void Clear(HashNode** buckets, uint32_t size);
    // TS3W.exe 0x0044C750: unlink one node, advancing across empty buckets without a bound.
    HashIterator* Erase(HashIterator* out, HashNode* node, HashNode** bucket);
};
TS3_X86_OFFSET(HashNode, next, 8);
TS3_X86_SIZE(HashNode, 12);
TS3_X86_OFFSET(WideHash, allocator, 28);
TS3_X86_SIZE(WideHash, 32);
TS3_X86_SIZE(HashIterator, 8);
struct RankedWide { int32_t rank; WideRange text; };
TS3_X86_OFFSET(RankedWide, text, 4);
TS3_X86_SIZE(RankedWide, 12);
// TS3W.exe 0x009A39A0: descending signed-rank/string comparator, stdcall on x86.
#if defined(_M_IX86)
int __stdcall RankedGreater(const RankedWide* a, const RankedWide* b);
#else
int RankedGreater(const RankedWide* a, const RankedWide* b);
#endif
// TS3W.exe 0x009A39E0: select the median of three records, preserving tie identity.
const RankedWide* MedianRanked(const RankedWide* a, const RankedWide* b, const RankedWide* c);
// TS3W.exe 0x0044D900: fetch singleton, lazily assigning static storage when requested.
void* GetSingleton(uint8_t create);
// TS3W.exe 0x0044D920: exchange callback/global pointer and return its previous value.
void* ExchangeCallback(void* value);
// TS3W.exe 0x0044D9A0: shut down, destroy and free the global service.
void ShutdownService();
struct WideBundle {
    WideString s0;
    WideString s1;
    WideString s2;
    WideString s3;
    WideString s4;
    WideString s5;
    WideString s6;
    WideString s7;
    WideString s8;
    WideString s9;
    WideString s10;
    WideString s11;
    WideString s12;
    WideString s13;
    WideString s14;
    WideString s15;
    WideString s16;
    WideString s17;
    WideString s18;
    WideString s19;
    WideString s20;
    uint8_t gap21[20];
    WideString s21;
    WideString s22;
    uint8_t gap23[992];
    WideString s23;
    uint8_t gap24[992];
    WideString s24;
    // TS3W.exe 0x00446F00: release the 25 owned strings in reverse order.
    void Release();
};
TS3_X86_OFFSET(WideBundle, s0, 0x0);
TS3_X86_OFFSET(WideBundle, s1, 0x34);
TS3_X86_OFFSET(WideBundle, s2, 0x68);
TS3_X86_OFFSET(WideBundle, s3, 0x9c);
TS3_X86_OFFSET(WideBundle, s4, 0xd0);
TS3_X86_OFFSET(WideBundle, s5, 0x104);
TS3_X86_OFFSET(WideBundle, s6, 0x138);
TS3_X86_OFFSET(WideBundle, s7, 0x16c);
TS3_X86_OFFSET(WideBundle, s8, 0x1a0);
TS3_X86_OFFSET(WideBundle, s9, 0x1d4);
TS3_X86_OFFSET(WideBundle, s10, 0x208);
TS3_X86_OFFSET(WideBundle, s11, 0x23c);
TS3_X86_OFFSET(WideBundle, s12, 0x270);
TS3_X86_OFFSET(WideBundle, s13, 0x2a4);
TS3_X86_OFFSET(WideBundle, s14, 0x2d8);
TS3_X86_OFFSET(WideBundle, s15, 0x30c);
TS3_X86_OFFSET(WideBundle, s16, 0x340);
TS3_X86_OFFSET(WideBundle, s17, 0x374);
TS3_X86_OFFSET(WideBundle, s18, 0x3a8);
TS3_X86_OFFSET(WideBundle, s19, 0x3dc);
TS3_X86_OFFSET(WideBundle, s20, 0x410);
TS3_X86_OFFSET(WideBundle, s21, 0x458);
TS3_X86_OFFSET(WideBundle, s22, 0x48c);
TS3_X86_OFFSET(WideBundle, s23, 0x8a0);
TS3_X86_OFFSET(WideBundle, s24, 0xcb4);
} // namespace ts3::misc

// ABI adapters for hookgen's opaque pointer words. Read-only pointees remain live on
// both calls; only LowerBound's explicit out cell needs snapshot/restore.
namespace ts3::misc {
inline int HookEqualWide(void* a, void* b) { return EqualWide(static_cast<const uint16_t*>(a), static_cast<const WideRange*>(b)); }
inline int HookCompareWideN(void* a, void* b, void* n) { return CompareWideN(static_cast<const uint16_t*>(a), static_cast<const uint16_t*>(b), static_cast<uint32_t>(reinterpret_cast<uintptr_t>(n))); }
inline int HookCompareWide(void* a, void* ae, void* b, void* be) { return CompareWide(static_cast<const uint16_t*>(a), static_cast<const uint16_t*>(ae), static_cast<const uint16_t*>(b), static_cast<const uint16_t*>(be)); }
inline void* HookMedianRanked(void* a, void* b, void* c) { return const_cast<RankedWide*>(MedianRanked(static_cast<const RankedWide*>(a), static_cast<const RankedWide*>(b), static_cast<const RankedWide*>(c))); }
} // namespace ts3::misc
