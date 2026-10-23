#include "ts3/anim/m05_M76.h"
namespace ts3::anim::m76 {
namespace {
Registry* Global() {
#if defined(_M_IX86)
    return reinterpret_cast<Registry*>(0x0114f690);
#else
    NotRewritten("m76 global registry");
#endif
}
Record* Find(uint32_t key) {
    Iterator out;
    CallGameMethod<Iterator*,0x00576C30>(Global(), &out, &key);
    return out.record;
}
}
uintptr_t Broadcast(void* listener) {
    return CallGameMethod<uintptr_t,0x00576BC0>(Global(), listener);
}
bool Enable(uint32_t key) {
    if (!key) return false;
    Record* record = Find(key);
    Registry* global = Global();
    if (record == global->buckets[global->bucketCount]) return false;
    record->enabled = 1;
    return true;
}
Record* Dirty(uint32_t key) {
    if (!key) return nullptr;
    Record* record = Find(key);
    Registry* global = Global();
    if (record != global->buckets[global->bucketCount]) record->dirty = 1;
    return record;
}
uintptr_t Attach(void* listener, uint32_t key) {
    if (!key || !listener) return key;
    Record* record = Find(key);
    Registry* global = Global();
    if (record == global->buckets[global->bucketCount]) return reinterpret_cast<uintptr_t>(record);
    return CallGameMethod<uintptr_t,0x00566590>(&record->vtable, listener);
}
Record* Clone(Registry* self, const Record* source) {
    auto* record = CallGameFunction<Record*,0x004E3F90>(uint32_t(sizeof(Record)), self->allocator, 0, 0, reinterpret_cast<const char*>(uintptr_t(0x00f98980)), 0xe7);
    if (record) {
        record->key = source->key;
#if defined(_M_IX86)
        record->vtable = reinterpret_cast<void*>(0x00fc2f44);
#else
        record->vtable = nullptr; // No original game table in the native build.
#endif
        record->listeners.next = &record->listeners;
        record->listeners.previous = &record->listeners;
        record->value = source->value;
        record->extra = source->extra;
        record->enabled = source->enabled;
        record->dirty = source->dirty;
    }
    record->next = nullptr;
    return record;
}
InsertResult* Insert(Registry* self, InsertResult* output, const Record* source, uint32_t unused) {
    (void)unused;
    const uint32_t key = source->key;
    uint32_t bucket = key % self->bucketCount;
    for (Record* record = self->buckets[bucket]; record; record = record->next) {
        if (record->key == key) {
            output->record = record;
            output->bucket = &self->buckets[bucket];
            output->inserted = false;
            return output;
        }
    }
    struct Growth { bool needed; uint8_t padding[3]; uint32_t buckets; } growth;
    CallGameMethod<Growth*,0x004E09B0>(self->policy, &growth, self->bucketCount, self->count, 1);
    Record* record = Clone(self, source);
    if (growth.needed) {
        bucket = key % growth.buckets;
        CallGameMethod<void,0x00625C10>(self, growth.buckets);
    }
    record->next = self->buckets[bucket];
    self->buckets[bucket] = record;
    self->count += 1;
    output->record = record;
    output->bucket = &self->buckets[bucket];
    output->inserted = true;
    return output;
}
#if defined(_M_IX86)
namespace hooks {
Record* __fastcall Clone(Registry* s, void*, const Record* r) { return m76::Clone(s,r); }
InsertResult* __fastcall Insert(Registry* s, void*, InsertResult* o, const Record* r, uint32_t u) { return m76::Insert(s,o,r,u); }
}
#endif
}
