// Cluster unit clu_c398c0_x (subsystem misc): rewrites of the fourteen TS3W.exe functions
// declared in ts3/misc/clu_c398c0_x.h. Addresses are in the header's comments.
//
// The original compiler's signed divisions survive as plain C++ `/` here: 0x2E8BA2E9
// (positive magic) is x/44 and 0xD1745D17 is x/-44, both truncating toward zero, so
// DivideBy44/DivideByMinus44 stay bit-exact for every 32-bit input.
//
// EA behaviours kept on purpose (they affect outputs):
//  - FindRouteByKey compares the key bytes in the order 2,0,1,5,3,4.
//  - CopyRouteUnitsForward/Reverse add 65537*base to each copied dword, so a low-u16
//    carry can leak into the high u16; the odd trailing unit gets a plain 16-bit add.
//  - FindNodeByFloatKey's x87 compare treats NaN as unequal (unordered -> next node).
//  - EraseRouteSpan's end pointer moves by 44*((first-dest)/-44) (divisor -44).
//  - 0x00C3AC20/0x00C3B1B0 release a node's value only when its refcount was exactly 1,
//    re-storing 1 before the vtable call.
//  - 0x00C50370's inserted node copies two uninitialised stack bytes into key bytes
//    6..7; this rewrite zeroes them (no function in the unit reads those bytes).

#include "ts3/misc/clu_c398c0_x.h"

#include <cmath>
#include <cstdint>
#include <cstring>

#include "ts3/layout.h"

namespace ts3::misc {
namespace {

constexpr float kRouteKeyEpsilon = 0.001f;  // 0x3A83126F at 0x010771A4

int32_t DivideBy44(int32_t value)
{
    return value / 44;
}

uint32_t RouteKeyHash(const uint8_t* key)
{
    const int32_t b0 = static_cast<int8_t>(key[0]);
    const int32_t b1 = static_cast<int8_t>(key[1]);
    const int32_t b2 = static_cast<int8_t>(key[2]);
    const int32_t b3 = static_cast<int8_t>(key[3]);
    const int32_t b4 = static_cast<int8_t>(key[4]);
    return static_cast<uint32_t>((b0 + b3 + 200 * b2) * 100 + b4 + b1);
}

bool KeyEquals(const uint8_t* nodeKey, const uint8_t* key)
{
    return nodeKey[2] == key[2] && nodeKey[0] == key[0] && nodeKey[1] == key[1] &&
           nodeKey[5] == key[5] && nodeKey[3] == key[3] && nodeKey[4] == key[4];
}

}  // namespace

void* __stdcall FindRouteByKey(void* head, const uint8_t* key, uint32_t hash)
{
    (void)hash;
    RouteHashNode* node = static_cast<RouteHashNode*>(head);
    while (node) {
        if (KeyEquals(node->key, key))
            return node;
        node = node->next;
    }
    return nullptr;
}

void FreeRouteChains(RouteTable* self, void** heads, uint32_t count)
{
    for (uint32_t i = 0; i < count; ++i) {
        RouteHashNode* node = static_cast<RouteHashNode*>(heads[i]);
        while (node) {
            RouteHashNode* const sentinel = self->freeSentinel;
            RouteHashNode* const next = node->next;
            node->freeNext = sentinel->freeNext;
            sentinel->freeNext = node;
            node = next;
        }
        heads[i] = nullptr;
    }
}

uintptr_t __stdcall CountRouteTotalsForward(RouteTableView* view)
{
    uint8_t** const buckets = reinterpret_cast<uint8_t**>(view->buckets);
    view->records44 = 0;
    view->units2 = 0;
    uint8_t** bucket = buckets;
    uint8_t* node = *bucket;
    if (!node) {
        do {
            ++bucket;
        } while (!*bucket);
        node = *bucket;
    }
    while (node != buckets[view->bucketCount]) {
        RouteSpan* const route = *reinterpret_cast<RouteSpan**>(node + 8);
        const int32_t records = DivideBy44(static_cast<int32_t>(
            reinterpret_cast<intptr_t>(route->recordsEnd) -
            reinterpret_cast<intptr_t>(route->recordsBegin)));
        view->records44 += static_cast<uint32_t>(records);
        view->units2 += static_cast<uint32_t>(
            static_cast<int32_t>(reinterpret_cast<intptr_t>(route->unitsEnd) -
                                 reinterpret_cast<intptr_t>(route->unitsBegin)) >>
            1);
        node = *reinterpret_cast<uint8_t**>(node + 0xC);
        if (!node) {
            do {
                ++bucket;
            } while (!*bucket);
            node = *bucket;
        }
    }
    return view->bucketCount;
}

uintptr_t __stdcall CountRouteTotalsReverse(RouteTableView* view)
{
    uint8_t** const buckets = reinterpret_cast<uint8_t**>(view->buckets);
    view->records44 = 0;
    view->units2 = 0;
    uint8_t** bucket = buckets;
    uint8_t* node = *bucket;
    if (!node) {
        do {
            ++bucket;
        } while (!*bucket);
        node = *bucket;
    }
    while (node != buckets[view->bucketCount]) {
        RouteSpan* const route = *reinterpret_cast<RouteSpan**>(node + 0xC);
        const int32_t records = DivideBy44(static_cast<int32_t>(
            reinterpret_cast<intptr_t>(route->recordsEnd) -
            reinterpret_cast<intptr_t>(route->recordsBegin)));
        view->records44 += static_cast<uint32_t>(records);
        view->units2 += static_cast<uint32_t>(
            static_cast<int32_t>(reinterpret_cast<intptr_t>(route->unitsEnd) -
                                 reinterpret_cast<intptr_t>(route->unitsBegin)) >>
            1);
        node = *reinterpret_cast<uint8_t**>(node + 0x10);
        if (!node) {
            do {
                ++bucket;
            } while (!*bucket);
            node = *bucket;
        }
    }
    return view->bucketCount;
}

uint32_t RemoveRoutesByKey(RouteTable* self, const uint8_t* key)
{
    const uint32_t before = self->count;
    RouteHashNode** const buckets = self->buckets;
    const uint32_t bucketCount = self->bucketCount;
    RouteHashNode** slot = buckets + (RouteKeyHash(key) % bucketCount);
    RouteHashNode* node = *slot;
    if (node) {
        for (;;) {
            if (KeyEquals(node->key, key))
                break;
            slot = &node->next;
            node = *slot;
            if (!node)
                break;
        }
        while (*slot) {
            node = *slot;
            if (!KeyEquals(node->key, key))
                break;
            *slot = node->next;
            RouteHashNode* const sentinel = self->freeSentinel;
            node->freeNext = sentinel->freeNext;
            sentinel->freeNext = node;
            --self->count;
        }
    }
    return before - self->count;
}

NodePair* FindRouteOrEnd(RouteTable* self, NodePair* out, const uint8_t* key)
{
    RouteHashNode** const buckets = self->buckets;
    const uint32_t bucketCount = self->bucketCount;
    const uint32_t hash = RouteKeyHash(key) % bucketCount;
    RouteHashNode** const slot = buckets + hash;
    RouteHashNode* const found = static_cast<RouteHashNode*>(FindRouteByKey(*slot, key, hash));
    if (found) {
        out->node = found;
        out->slot = slot;
    } else {
        out->node = buckets[bucketCount];
        out->slot = buckets + bucketCount;
    }
    return out;
}

void* __stdcall FindNodeByFloatKey(void* head, const void* key0, uint32_t unused)
{
    (void)unused;
    const uint8_t* const key = static_cast<const uint8_t*>(key0);
    uint8_t* node = static_cast<uint8_t*>(head);
    while (node) {
        const float* const nodeFloats = reinterpret_cast<const float*>(node);
        const float* const keyFloats = reinterpret_cast<const float*>(key);
        if (*reinterpret_cast<const uint32_t*>(node + 8) ==
                *reinterpret_cast<const uint32_t*>(key + 8) &&
            std::fabs(nodeFloats[0] - keyFloats[0]) <= kRouteKeyEpsilon &&
            std::fabs(nodeFloats[1] - keyFloats[1]) <= kRouteKeyEpsilon)
            return node;
        node = *reinterpret_cast<uint8_t* const*>(node + 0x10);
    }
    return nullptr;
}

uintptr_t ReleaseRouteChains(RouteTable* self, void** heads, uint32_t count)
{
    uintptr_t result = 0;
    for (uint32_t i = 0; i < count; ++i) {
        RouteHashNode* node = static_cast<RouteHashNode*>(heads[i]);
        while (node) {
            RouteHashNode* const next = node->next;
            RouteSpan* const value = static_cast<RouteSpan*>(node->value);
            if (value) {
                const uint32_t refs = value->refCount;
                value->refCount = refs - 1;
                if (refs == 1) {
                    value->refCount = 1;
                    reinterpret_cast<void(__thiscall*)(RouteSpan*, int)>(
                        value->vtable[0])(value, 1);
                }
            }
            RouteHashNode* const sentinel = self->freeSentinel;
            node->freeNext = sentinel->freeNext;
            sentinel->freeNext = node;
            result = reinterpret_cast<uintptr_t>(sentinel);
            node = next;
        }
        heads[i] = nullptr;
    }
    return result;
}

NodePair* UnlinkRouteNode(RouteTable* self, NodePair* out, RouteHashNode* node, RouteHashNode** slot)
{
    RouteHashNode* link = node->next;
    out->node = link;
    out->slot = slot;
    if (!link) {
        do {
            out->slot = out->slot + 1;
            link = *out->slot;
            out->node = link;
        } while (!link);
    }
    if (*slot == node) {
        *slot = node->next;
    } else {
        RouteHashNode* prev = *slot;
        RouteHashNode* cur = prev->next;
        while (cur != node) {
            prev = cur;
            cur = cur->next;
        }
        prev->next = cur->next;
    }
    RouteSpan* const value = static_cast<RouteSpan*>(node->value);
    if (value) {
        const uint32_t refs = value->refCount;
        value->refCount = refs - 1;
        if (refs == 1) {
            value->refCount = 1;
            reinterpret_cast<void(__thiscall*)(RouteSpan*, int)>(value->vtable[0])(value, 1);
        }
    }
    RouteHashNode* const sentinel = self->freeSentinel;
    node->freeNext = sentinel->freeNext;
    sentinel->freeNext = node;
    --self->count;
    return out;
}

void* EraseRouteSpan(void* vec, void* dest, void* first)
{
    uint8_t* const end = *reinterpret_cast<uint8_t**>(static_cast<uint8_t*>(vec) + 4);
    uint8_t* src = static_cast<uint8_t*>(first);
    uint8_t* dst = static_cast<uint8_t*>(dest);
    if (src != end) {
        do {
            for (size_t i = 0; i < 44; ++i)
                dst[i] = src[i];
            src += 44;
            dst += 44;
        } while (src != end);
    }
    const intptr_t delta = static_cast<intptr_t>(reinterpret_cast<uintptr_t>(first)) -
                           static_cast<intptr_t>(reinterpret_cast<uintptr_t>(dest));
    const intptr_t moved = static_cast<int32_t>(delta) / -44;
    *reinterpret_cast<uint8_t**>(static_cast<uint8_t*>(vec) + 4) = end + 44 * moved;
    return dest;
}

uint32_t* AppendRouteEntry(RouteManagerView* self, uint32_t a2, uint32_t a3, uint32_t a4,
                           uint32_t* a5, uint32_t* a6, uint32_t* a7)
{
    uint32_t key[2] = {a2, *a5};
    uint32_t inserted = 0;
    // 0x0066F5A0 inserts into the tree at this+0x3BC; its third argument is unused (the
    // original passes its own a2 argument slot with the low byte cleared).
    CallGameMethod<void, 0x0066F5A0>(reinterpret_cast<void*>(self->tree), &inserted, key,
                                     static_cast<uint32_t>(a2 & ~0xFFu));
    const uint32_t counter6 = *a6;
    const uint32_t counter7 = *a7;
    RouteEntry* const cur = self->entryEnd;
    const uint32_t third = a4 / 3;
    const RouteEntry entry{a3, third, counter6, counter7, a2};
    if (reinterpret_cast<uintptr_t>(cur) < reinterpret_cast<uintptr_t>(self->entryCapacity)) {
        self->entryEnd = cur + 1;
        if (cur)
            *cur = entry;
    } else {
        CallGameMethod<void, 0x00839220>(reinterpret_cast<void*>(&self->entryBegin), cur,
                                         const_cast<RouteEntry*>(&entry));
    }
    *a6 += a3;
    *a7 += a4;
    ++*a5;
    return a7;
}

void* GetOrInsertRoutePayload(RouteTable* self, const uint8_t* key)
{
    NodePair pair;
    FindRouteOrEnd(self, &pair, key);
    RouteHashNode* node = pair.node;
    if (node == self->buckets[self->bucketCount]) {
        // The 0x00C4E680 insert copies a 12-byte key; the original leaves +6/+7 as stack
        // padding, which this rewrite zeroes (no unit function reads those bytes).
        alignas(4) uint8_t insertKey[12] = {};
        std::memcpy(insertKey, key, 4);
        std::memcpy(insertKey + 4, key + 4, 2);
        uintptr_t inserted[3] = {};
        CallGameMethod<void, 0x00C4E680>(self, inserted, insertKey, 0);
        node = reinterpret_cast<RouteHashNode*>(inserted[0]);
    }
    return reinterpret_cast<uint8_t*>(node) + 8;
}

namespace {

// Shared body of the two copy routines: route link offset and chain-link offset.
uintptr_t CopyRouteUnits(RouteManagerView* self, RouteTableView* view, uint16_t* out,
                         uint32_t entryIndex, size_t routeOffset, size_t nextOffset)
{
    uint32_t offset = self->entryBegin[entryIndex].counter7;
    uint8_t** const buckets = reinterpret_cast<uint8_t**>(view->buckets);
    uint8_t** bucket = buckets;
    uint8_t* node = *bucket;
    if (!node) {
        do {
            ++bucket;
        } while (!*bucket);
        node = *bucket;
    }
    uint8_t* const endNode = *reinterpret_cast<uint8_t**>(buckets + view->bucketCount);
    uint32_t base = 0;
    uintptr_t result = reinterpret_cast<uintptr_t>(endNode);
    while (node != endNode) {
        RouteSpan* const route = *reinterpret_cast<RouteSpan**>(node + routeOffset);
        // (unitsEnd - unitsBegin) >> 1 as a 32-bit unsigned byte difference.
        const uint32_t count = static_cast<uint32_t>(
                                   reinterpret_cast<uintptr_t>(route->unitsEnd) -
                                   reinterpret_cast<uintptr_t>(route->unitsBegin)) >>
                               1;
        uint32_t* const src = reinterpret_cast<uint32_t*>(route->unitsBegin);
        // The original writes dwords at the byte address out + 2*offset (a u16 index),
        // so the destination can be only 2-byte aligned.
        uint32_t* const dst =
            reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(out) + 2u * offset);
        const uint32_t packed = 65537u * base;
        const uint32_t words = count >> 1;
        for (uint32_t i = 0; i < words; ++i)
            dst[i] = packed + src[i];
        if (count & 1)
            out[offset + count - 1] =
                static_cast<uint16_t>(base + route->unitsBegin[count - 1]);
        offset += count;
        const int32_t records = DivideBy44(static_cast<int32_t>(
            reinterpret_cast<intptr_t>(route->recordsEnd) -
            reinterpret_cast<intptr_t>(route->recordsBegin)));
        base += static_cast<uint32_t>(records);
        result = static_cast<uintptr_t>(static_cast<uint32_t>(records));
        node = *reinterpret_cast<uint8_t**>(node + nextOffset);
        while (!node) {
            ++bucket;
            result = reinterpret_cast<uintptr_t>(bucket);  // the scan's slot address
            node = *bucket;
        }
    }
    return result;
}

}  // namespace

uintptr_t CopyRouteUnitsForward(RouteManagerView* self, RouteTableView* view, uint16_t* out,
                                uint32_t entryIndex)
{
    return CopyRouteUnits(self, view, out, entryIndex, 8, 0xC);
}

uintptr_t CopyRouteUnitsReverse(RouteManagerView* self, RouteTableView* view, uint16_t* out,
                                uint32_t entryIndex)
{
    return CopyRouteUnits(self, view, out, entryIndex, 0xC, 0x10);
}

}  // namespace ts3::misc