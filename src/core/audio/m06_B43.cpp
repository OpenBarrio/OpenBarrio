#include "ts3/audio/m06_B43.h"
namespace ts3::audio::b43impl {
Scene* Destroy(Scene* self, uint32_t flags) {
#if defined(_M_IX86)
    self->vtable = reinterpret_cast<void*>(0x0103A4A0);
    self->secondary = reinterpret_cast<void*>(0x0103A49C);
    if (self->resource) CallGameVirtual<void, 1>(self->resource);
    if (self->storage && self->storage != self->inlineStorage)
        CallGameFunction<void, 0x004E3F70>(self->storage);
    self->secondary = reinterpret_cast<void*>(0x0107396C);
    self->vtable = reinterpret_cast<void*>(0x00FA1B78);
    if (flags & 1) {
        auto& head = *reinterpret_cast<void**>(0x011DA038);
        self->vtable = head;
        head = self;
    }
#else
    (void)flags;
    AbortNotRewritten("B43 scene destruction: original allocator and free-list singleton");
#endif
    return self;
}
float Zero(void*, uint32_t, uint32_t) { return 0.0f; }
const char* EmptyName(void*, uint32_t) {
#if defined(_M_IX86)
    return reinterpret_cast<const char*>(0x00F99020);
#else
    return "";
#endif
}
static bool Find(void* self, uint32_t key, Value** value) {
    // EA reuses its stack argument slot as the lookup out-parameter. Preserve
    // its initial bits for callbacks, and do not add a successful-null check.
    *value = reinterpret_cast<Value*>(static_cast<uintptr_t>(key));
    return CallGameVirtual<bool, 8>(self, key, value);
}
static uint32_t* Data(Value* value) {
    return (value->flags & 0x30) ? static_cast<uint32_t*>(value->payload.array.data) : value->payload.words;
}
bool ReadFour(void* self, uint32_t key, uint32_t* out) {
    Value* v;
    if (!Find(self, key, &v) || v->type != 1000) return false;
    auto p = Data(v);
    out[0] = p[0]; out[1] = p[1]; out[2] = p[2]; out[3] = p[3];
    return true;
}
bool ReadTwo(void* self, uint32_t key, uint32_t* out) {
    Value* v;
    if (!Find(self, key, &v) || v->type != 1000) return false;
    auto p = Data(v);
    out[0] = p[0]; out[1] = p[1];
    return true;
}
bool ReadSplit(void* self, uint32_t key, uint32_t* out, uint32_t* last) {
    Value* v;
    if (!Find(self, key, &v) || v->type != 1000) return false;
    auto p = Data(v);
    out[0] = p[0]; out[1] = p[1]; *last = p[3];
    return true;
}
static bool ReadArray(void* self, uint32_t key, uint32_t* count, void** out, uint16_t type) {
    Value* v;
    if (!Find(self, key, &v) || v->type != type || !(v->flags & 0x30)) return false;
    *count = v->payload.array.count;
    // The first store may alias flags/type: EA tests both again, in this order.
    if (v->flags & 0x30) *out = v->payload.array.data;
    else if (v->type) *out = v->payload.words;
    else *out = nullptr;
    return true;
}
bool Read13(void* s, uint32_t k, uint32_t* n, void** p) { return ReadArray(s,k,n,p,13); }
bool Read10(void* s, uint32_t k, uint32_t* n, void** p) { return ReadArray(s,k,n,p,10); }
bool Read1000(void* s, uint32_t k, uint32_t* n, void** p) { return ReadArray(s,k,n,p,1000); }
uint32_t Compatible(void*, uint32_t from, uint32_t to) {
    return ((from == 0x8070223D || from == 0x02B9F662) && to == 0x02B9F662) ||
        (from == 0x02C9EFF2 && to == from);
}
uint32_t ClassId(void*) { return 0x02B23D7F; }
uint32_t Types(void*, uint32_t* out, uint32_t capacity) {
    if (out) {
        if (capacity < 3) return 0;
        out[0] = 0x02B9F662; out[1] = 0x8070223D; out[2] = 0x02C9EFF2;
    }
    return 3;
}
void* Bind(OwnerView* self, void* first, void* owner) {
    self->owner = owner ? static_cast<uint8_t*>(owner) - 0x34 : nullptr;
    return CallGameMethod<void*, 0x00406250>(self, first, owner);
}
}
