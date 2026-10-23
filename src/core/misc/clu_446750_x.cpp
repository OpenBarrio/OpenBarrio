#include "ts3/misc/clu_446750_x.h"
#include <algorithm>
#include <cstring>
namespace ts3::misc {
namespace {
int32_t Length(const uint16_t* a, const uint16_t* b) {
    return static_cast<int32_t>((reinterpret_cast<intptr_t>(b) - reinterpret_cast<intptr_t>(a)) >> 1);
}
void ReleaseString(WideString& s) {
    const auto bytes = reinterpret_cast<uintptr_t>(s.capacity) - reinterpret_cast<uintptr_t>(s.begin);
    if (static_cast<intptr_t>(bytes & ~uintptr_t(1)) > 2 && s.begin && s.begin != s.local)
        CallGameFunction<void, 0x004E3F70>(s.begin);
}
template<unsigned Address> void*& Global() {
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(Address);
#else
    AbortNotRewritten("clu_446750_x game global");
    static void* unavailable; return unavailable;
#endif
}
}
// 0x00446750
int EqualWide(const uint16_t* text, const WideRange* range) {
    const uint16_t* end = text;
    while (*end) ++end;
    const auto n = Length(text, end);
    return n == Length(range->begin, range->end) && (!n || std::memcmp(text, range->begin, size_t(n) * 2) == 0);
}
// 0x00446820
int CompareWideN(const uint16_t* a, const uint16_t* b, uint32_t count) {
    while (count--) { if (*a != *b) return *a < *b ? -1 : 1; ++a; ++b; }
    return 0;
}
// 0x00446B50
int CompareWide(const uint16_t* a, const uint16_t* ae, const uint16_t* b, const uint16_t* be) {
    const int32_t an = Length(a, ae), bn = Length(b, be);
    const int cmp = CompareWideN(a, b, static_cast<uint32_t>(std::min(an, bn)));
    return cmp ? cmp : an < bn ? -1 : an > bn;
}
// 0x00446D60
WideNode** WideTree::LowerBound(WideNode** out, const WideRange* key) {
    WideNode* candidate = reinterpret_cast<WideNode*>(&sentinel[0]);
    for (WideNode* node = sentinel[2]; node;) {
        if (CompareWide(node->key.begin, node->key.end, key->begin, key->end) >= 0) {
            candidate = node; node = node->left;
        } else node = node->right;
    }
    *out = candidate; return out;
}
// 0x00449650 (ECX is preserved for recursive calls; decompiler's stdcall is wrong).
void WideTree::DestroyNodes(WideNode* node) {
    while (node) {
        DestroyNodes(node->right);
        WideNode* next = node->left;
        ReleaseString(node->value); ReleaseString(node->key);
        CallGameFunction<void, 0x004E3F70>(node);
        node = next;
    }
}
// 0x0044C6F0: EAX is unspecified when no node was visited; original callers ignore it.
void WideHash::Clear(HashNode** buckets, uint32_t size) {
    for (uint32_t i = 0; i < size; ++i) {
        for (HashNode* node = buckets[i]; node;) {
            HashNode* next = node->next;
            CallGameVirtual<void, 3>(allocator, node, uint32_t(12));
            node = next;
        }
        buckets[i] = nullptr;
    }
}
// 0x0044C750
HashIterator* WideHash::Erase(HashIterator* out, HashNode* node, HashNode** bucket) {
    out->node = node->next; out->bucket = bucket;
    while (!out->node) out->node = *++out->bucket;
    HashNode** link = bucket;
    while (*link != node) link = &(*link)->next;
    *link = node->next;
    CallGameVirtual<void, 3>(allocator, node, uint32_t(12));
    --count; return out;
}
// 0x0044D900
void* GetSingleton(uint8_t create) {
    void*& value = Global<0x011B1394>();
#if defined(_M_IX86)
    if (!value && create) value = reinterpret_cast<void*>(0x011B13A0);
#else
    (void)create;
#endif
    return value;
}
// 0x0044D920
void* ExchangeCallback(void* value) {
    void*& current = Global<0x011B1398>();
    void* previous = current; current = value; return previous;
}
// 0x0044D9A0: callback may replace or clear the service, so reload after slot 3.
void ShutdownService() {
    void*& service = Global<0x011B1390>();
    if (!service) return;
    if (Global<0x011B138C>() == service) Global<0x011B138C>() = nullptr;
    CallGameVirtual<void, 3>(service);
    void* owner = Global<0x011B1380>();
    void* old = service;
    if (old) {
        CallGameVirtual<void, 0>(old, uint32_t(0));
        CallGameVirtual<void, 3>(owner, old, uint32_t(0));
    }
    service = nullptr;
}
// 0x004A47D0: allocator takes six arguments, not the three inferred by IDA.
WideNode** WideTree::Insert(WideNode** out, WideNode* parent, const WideRange* key, uint8_t forceLeft) {
    auto* header = reinterpret_cast<WideNode*>(&sentinel[0]);
    const int right = !forceLeft && parent != header && CompareWide(key->begin, key->end, parent->key.begin, parent->key.end) >= 0;
    void* raw = CallGameFunction<void*, 0x004E3F90>(uint32_t(32), allocator, uint32_t(0), uint32_t(0), uintptr_t(0x00F98980), uint32_t(0xE7));
    auto* node = static_cast<WideNode*>(raw);
    // EA tests allocation+16, not allocation itself: null still calls the constructor.
    auto* dest = reinterpret_cast<uint8_t*>(reinterpret_cast<uintptr_t>(raw) + offsetof(WideNode, key));
    if (dest) CallGameMethod<void, 0x00408EB0>(dest, key);
    CallGameFunction<void, 0x004E0C40>(node, parent, header, right);
    ++count; *out = node; return out;
}
// 0x009A39A0
#if defined(_M_IX86)
int __stdcall RankedGreater(const RankedWide* a, const RankedWide* b) {
#else
int RankedGreater(const RankedWide* a, const RankedWide* b) {
#endif
    return a->rank > b->rank || (a->rank == b->rank && CompareWide(b->text.begin, b->text.end, a->text.begin, a->text.end) < 0);
}
// 0x009A39E0: do not sort a temporary array: equal keys retain specific identities.
const RankedWide* MedianRanked(const RankedWide* a, const RankedWide* b, const RankedWide* c) {
    if (RankedGreater(a, b)) {
        if (!RankedGreater(b, c)) return RankedGreater(a, c) ? c : a;
        return b;
    }
    if (RankedGreater(a, c)) return a;
    return RankedGreater(b, c) ? c : b;
}
// 0x009D1960
int WideRange::CompareSubstring(uint32_t offset, uint32_t count, const uint16_t* text) const {
    const uint16_t* textEnd = text;
    while (*textEnd) ++textEnd;
    const uint32_t remaining = static_cast<uint32_t>(Length(begin, end)) - offset;
    return CompareWide(begin + offset, begin + offset + std::min(remaining, count), text, textEnd);
}
// 0x00446F00: requested entry is an instance destructor, not IDA's global thunk.
void WideBundle::Release() {
    ReleaseString(s24);
    ReleaseString(s23);
    ReleaseString(s22);
    ReleaseString(s21);
    ReleaseString(s20);
    ReleaseString(s19);
    ReleaseString(s18);
    ReleaseString(s17);
    ReleaseString(s16);
    ReleaseString(s15);
    ReleaseString(s14);
    ReleaseString(s13);
    ReleaseString(s12);
    ReleaseString(s11);
    ReleaseString(s10);
    ReleaseString(s9);
    ReleaseString(s8);
    ReleaseString(s7);
    ReleaseString(s6);
    ReleaseString(s5);
    ReleaseString(s4);
    ReleaseString(s3);
    ReleaseString(s2);
    ReleaseString(s1);
    ReleaseString(s0);
}
} // namespace ts3::misc
