#include "ts3/anim/m05_B09.h"
#include <limits>
#include <cstring>
namespace ts3::anim::b09impl {
namespace {
float FloatStore(float value) {
#if defined(_M_IX86)
    __asm { fld value }
    __asm { fstp value }
#else
    uint32_t bits; std::memcpy(&bits,&value,4);
    if ((bits & 0x7f800000u) == 0x7f800000u && (bits & 0x007fffffu)) {
        bits |= 0x00400000u; std::memcpy(&value,&bits,4);
    }
#endif
    return value;
}
}

float Forward7(Controller* self) { return CallGameVirtual<float,7>(self); }
float Forward8(Controller* self) { return CallGameVirtual<float,8>(self); }
void* FindFactory(uint32_t key) {
#if defined(_M_IX86)
    void* result[2];
    auto* registry = reinterpret_cast<unsigned char*>(0x011cd508);
    CallGameMethod<void,0x00d38b00>(registry, result, &key);
    auto** buckets = *reinterpret_cast<void***>(registry+4);
    auto index = *reinterpret_cast<uint32_t*>(registry+8);
    return result[0] == buckets[index] ? nullptr : static_cast<void**>(result[0])[1];
#else
    (void)key; NotRewritten("B09 global factory registry");
#endif
}
Controller* ReadController(void* argument, Reader* reader) {
    uint32_t version = ~0u;
    auto status = CallGameFunction<uintptr_t,0x004dd700>(reader->source, &version, reader->context);
    CallGameMethod<uintptr_t,0x007d77c0>(reader, status);
    if (!reader->good || version > 1) return nullptr;
    uint32_t key = 0;
    status = CallGameFunction<uintptr_t,0x004dd700>(reader->source, &key, reader->context);
    CallGameMethod<uintptr_t,0x007d77c0>(reader, status);
    if (!reader->good || !key) return nullptr;
    auto* factory = FindFactory(key);
    if (!factory) return nullptr;
    auto* result = CallGameVirtual<Controller*,0>(factory, argument);
    if (result) {
        CallGameVirtual<void,16>(result, reader);
        if (!reader->good) return nullptr;
    }
    return result;
}
uint32_t RegisterFactory(uint32_t key, void* factory) {
#if defined(_M_IX86)
    struct Pair { uint32_t key; void* factory; } pair{key,factory};
    uintptr_t result[3];
    auto value = CallGameMethod<uint32_t,0x008bc3d0>(reinterpret_cast<void*>(0x011cd508),result,&pair,key & 0xffffff00u);
    return (value & 0xffffff00u) | 1u;
#else
    (void)key; (void)factory; NotRewritten("B09 registry insert");
#endif
}
uintptr_t WriteFloat(Reader* self, float value) {
    auto status = CallGameFunction<uintptr_t,0x004ddff0>(self->source, FloatStore(value), self->context);
    return CallGameMethod<uintptr_t,0x007d77c0>(self,status);
}
// Normalized twin 00ADB0E0 uses -1 at FAA210; this relocation is FLT_MAX.
float MinusOne(Sequence*) { return std::numeric_limits<float>::max(); }
Controller* Active(Sequence* self) { return self->second ? self->second : self->first; }
uintptr_t SetFlag(Sequence* self, uint32_t enabled) {
    auto result = reinterpret_cast<uintptr_t>(&self->nodes);
    if (self->nodes.next != &self->nodes) {
        auto* node = reinterpret_cast<Node*>(self->nodes.previous);
        result = reinterpret_cast<uintptr_t>(node);
        if (static_cast<uint8_t>(enabled)) node->flags |= 1;
        else node->flags &= ~1u;
    }
    if (self->second) {
        if (static_cast<uint8_t>(enabled)) self->flags58 |= 1;
        else self->flags58 &= ~1u;
    }
    return result;
}
uintptr_t GetFlag(Sequence* self) {
    if (self->nodes.next != &self->nodes)
        return reinterpret_cast<Node*>(self->nodes.previous)->flags & 1;
    if (self->second) return self->flags58 & 1;
    return reinterpret_cast<uintptr_t>(&self->nodes) & ~uintptr_t(255);
}
bool Query11(Sequence* self) {
    if (self->first && CallGameVirtual<uint8_t,11>(self->first)) return true;
    if (self->second && CallGameVirtual<uint8_t,11>(self->second)) return true;
    return false;
}
bool Query12(Sequence* self) {
    if (self->first && CallGameVirtual<uint8_t,12>(self->first)) return true;
    if (self->second && CallGameVirtual<uint8_t,12>(self->second)) return true;
    return false;
}
bool AddDelegate(Delegates* self, void* callback, uint32_t key) {
#if defined(_M_IX86)
    const void* descriptor = reinterpret_cast<void*>(0x00fc1a08);
#else
    const void* descriptor = nullptr;
#endif
    auto* node = CallGameFunction<Delegate*,0x004e3f90>(sizeof(Delegate),descriptor,0,0,0,0);
    if (!node) return false;
    node->callback = callback;
    node->key = key;
    node->links.previous = self->list.previous;
    node->links.next = &self->list;
    self->list.previous = &node->links;
    node->links.previous->next = &node->links;
    return true;
}
uintptr_t Reset(State* self) {
    auto* controller = self->controller;
    self->a = 0; self->b = 0; self->x = 0; self->y = 0;
    self->controller = nullptr;
    if (!controller) return reinterpret_cast<uintptr_t>(self);
    auto refs = static_cast<uint32_t>(controller->refs) - 1u;
    controller->refs = static_cast<int32_t>(refs);
    if (refs) return refs;
    controller->refs = 1;
    return CallGameVirtual<uintptr_t,0>(controller,1u);
}
State* Copy(State* self, const State* source) {
    self->controller = source->controller;
    if (self->controller) self->controller->refs = static_cast<int32_t>(static_cast<uint32_t>(self->controller->refs)+1u);
    self->a = source->a; self->b = source->b;
    self->x = FloatStore(source->x); self->y = FloatStore(source->y);
    return self;
}
}
