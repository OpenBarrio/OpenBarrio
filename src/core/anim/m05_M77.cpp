#include "ts3/anim/m05_M77.h"
namespace ts3::anim {
namespace {
M77Map* GlobalMap() {
#if defined(_M_IX86)
    return reinterpret_cast<M77Map*>(0x0114f690);
#else
    NotRewritten("m05_M77 global scratch registry");
#endif
}
void InitRecord(M77Record& r) {
#if defined(_M_IX86)
    r.vtable = reinterpret_cast<void*>(0x00fc2f44);
#else
    r.vtable = nullptr; // No original vtable exists in a native x64 process.
#endif
    r.listeners.next = r.listeners.previous = &r.listeners;
    r.dispatchDepth = 0;
    r.resource = nullptr;
    r.pendingFirst = r.pendingSecond = 0;
}
}
M77Iterator* __fastcall EraseM77(M77Map* self, void*, M77Iterator* out, M77Iterator at) {
    out->bucket = at.bucket;
    out->node = at.node->next;
    while (!out->node) out->node = *++out->bucket;
    if (*at.bucket == at.node) {
        *at.bucket = at.node->next;
    } else {
        auto previous = *at.bucket;
        while (previous->next != at.node) previous = previous->next;
        previous->next = at.node->next;
    }
    CallGameMethod<void,0x005659d0>(&at.node->record);
    CallGameFunction<void,0x004e3f70>(at.node);
    --self->size;
    return out;
}
void __fastcall SweepM77(M77Map* self, void*) {
    auto bucket = self->buckets;
    while (!*bucket) ++bucket;
    auto node = *bucket;
    auto end = self->buckets[self->bucketCount];
    while (node != end) {
        auto& r = node->record;
        if (r.pendingFirst || r.pendingSecond) {
            if (r.resource) {
                if (r.pendingFirst) CallGameMethod<void,0x00576b30>(&r,r.resource);
                // The first callback can change both the flag and resource.
                if (r.pendingSecond) CallGameMethod<void,0x00576b70>(&r,r.resource);
            }
            M77Iterator next;
            EraseM77(self,nullptr,&next,{node,bucket});
            node = next.node;
            bucket = next.bucket;
        } else {
            node = node->next;
            while (!node) node = *++bucket;
        }
    }
}
void SweepGlobalM77() { SweepM77(GlobalMap(),nullptr); }
M77Record* __fastcall GetOrInsertM77(M77Map* self, void*, void* const* key) {
    M77Iterator found;
    CallGameMethod<M77Iterator*,0x00576c30>(self,&found,key);
    if (found.node == self->buckets[self->bucketCount]) {
        // EA constructs and destroys two empty delegate records, not just the inserted one.
        M77Record first;
        InitRecord(first);
        struct Pair { void* key; M77Record record; } pair;
        pair.key = *key;
        InitRecord(pair.record);
        M77InsertResult inserted;
        const auto forwarded = reinterpret_cast<uintptr_t>(key) & ~uintptr_t(0xff);
        CallGameMethod<M77InsertResult*,0x00576df0>(self,&inserted,&pair,forwarded);
        found = inserted.iterator;
        CallGameMethod<void,0x005659d0>(&pair.record);
        CallGameMethod<void,0x005659d0>(&first);
    }
    return &found.node->record;
}
void RegisterM77(void* listener, void* resource) {
    if (resource && listener) {
        auto r = GetOrInsertM77(GlobalMap(),nullptr,&resource);
        r->resource = resource;
        CallGameMethod<uint8_t,0x00566170>(r,listener,resource);
    }
}
void* __fastcall RigValueM77(const M77RigView* self, void*) {
    return self->value;
}
}
