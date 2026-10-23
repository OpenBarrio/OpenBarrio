#include "ts3/audio/m06_B08.h"
#include <atomic>
#include <cstring>
namespace ts3::audio::b08 {
namespace {
template<uintptr_t Address> void* Global() {
#if defined(_M_IX86)
    return *reinterpret_cast<void**>(Address);
#else
    ts3::NotRewritten("m06_B08 game singleton");
#endif
}
uint32_t Bits(float v) { uint32_t r; std::memcpy(&r,&v,4); return r; }
float Float(uint32_t v) { float r; std::memcpy(&r,&v,4); return r; }
}
uintptr_t ListenerCommand(void* self, uint32_t a, const void* b, uint32_t c, uint32_t d) {
    CallGameVirtual<void,17>(self, 0xa935b29eu);
    CallGameVirtual<void,19>(self, 0x0490d2ebu, a);
    CallGameVirtual<void,22>(self, 0x3c91e635u, b);
    CallGameVirtual<void,19>(self, 0xd23d1bd7u, c);
    CallGameVirtual<void,19>(self, 0xc63adaf5u, d);
    return CallGameVirtual<uintptr_t,26>(self);
}
uint32_t ReadLevels(const Levels* self, uint32_t index, uint32_t* a, uint32_t* b, uint32_t* c) {
    *a = self->first[index];
    *b = self->second[index];
    const uint32_t value = self->third[index];
    *c = value;
    return value;
}
void* LockRW(void* self) {
    void* rw = Global<0x011ffc28>();
    if (rw) CallGameMethod<void,0x00ed7170>(rw);
    return self;
}
double ServiceVolume(const char16_t* volume, const char16_t* disabled) {
    void* service = Global<0x0114fab0>();
    if (CallGameFunction<uint8_t,0x0058aad0>(service, disabled, 0)) return 0.0;
    const float value = CallGameFunction<float,0x0058abf0>(Global<0x0114fab0>(), volume, 255.0f);
    // EA's branchless clamp uses sign bits, not ordered comparisons (including NaNs).
    const uint32_t negativeMask = 0u - ((Bits(value) ^ 0x80000000u) >> 31);
    const float lower = Float(Bits(value) & negativeMask);
    const float difference = 255.0f - lower;
    const uint32_t upperMask = 0u - (Bits(difference) >> 31);
    const float clamped = Float((Bits(lower) & ~upperMask) | (Bits(255.0f) & upperMask));
    return static_cast<double>(clamped) * static_cast<double>(Float(0x3b808081));
}
uintptr_t MasterGain(void* self, float gain) {
    void* bus = CallGameVirtual<void*,108>(self, 0xf1cc1687u);
    if (!bus) return 0;
    void* parameter = CallGameMethod<void*,0x008c1370>(static_cast<uint8_t*>(bus)+0x10, 0x47616930u);
    if (!parameter) return 0;
    void* rw = Global<0x011ffc28>();
    if (rw) CallGameMethod<void,0x00ed7170>(rw);
    uintptr_t result = CallGameMethod<uintptr_t,0x00edaf00>(parameter, 0u, gain);
    rw = Global<0x011ffc28>();
    if (rw) result = CallGameMethod<uintptr_t,0x00ed7190>(rw);
    return result;
}
bool RoutingMask(void*, const uint32_t* mask) {
    if (!*mask) return true;
    void* world = Global<0x011ecbc4>();
    if (!world) return true;
    // Only the external world's interface view is observed, not its owned layout.
    void* routing = static_cast<RoutingWorld*>(world)->routing;
    if (!routing) return true;
    struct Query { uint32_t zero; float one; uint32_t flags, a, b; float tail; } query{0,1.0f,0,0,0,0};
    CallGameMethod<void,0x00d69940>(routing, &query);
    return (*mask & query.flags) != 0;
}
uintptr_t ScriptOperation(void*, uint32_t low, uint32_t high, uint32_t a, uint32_t b, uint32_t c) {
    void* object = CallGameFunction<void*,0x00938b50>();
    if (!object) return 0;
    object = CallGameMethod<void*,0x00939560>(object, low, high);
    if (!object) return 0;
    object = CallGameMethod<void*,0x00935770>(object, Global<0x011e8e70>());
    if (!object) return 0;
    return CallGameMethod<uintptr_t,0x00944100>(object, a,b,c);
}
void* PublishSettings(Manager* self) {
    const int32_t quality = CallGameFunction<int32_t,0x0058aba0>(Global<0x0114fab0>(), u"AudioQuality", 3);
    void* command = CallGameFunction<void*,0x008b0a70>();
    auto publish = [](void* target, uint32_t name, int32_t value) {
        if (!target) return;
        CallGameVirtual<void,17>(target, 0x03cdd1a9u);
        CallGameVirtual<void,19>(target, 0x034753a7u, name);
        CallGameVirtual<void,18>(target, 0x034753aau, static_cast<float>(value));
        CallGameVirtual<void,26>(target);
    };
    publish(command, 0x6df5822du, quality);
    const int32_t mode = CallGameFunction<int32_t,0x0058aba0>(Global<0x0114fab0>(), u"AudioOutputMode", 1);
    command = CallGameFunction<void*,0x008b0a70>();
    publish(command, 0x0435b383u, mode);
    self->enabled = CallGameFunction<uint8_t,0x0058aad0>(Global<0x0114fab0>(), u"FocusMute", static_cast<int>(self->enabled));
    const float voice = static_cast<float>(ServiceVolume(u"VoiceLevel", u"VoiceMute"));
    const float effects = static_cast<float>(ServiceVolume(u"SoundFXLevel", u"SoundFXMute"));
    const float ambient = static_cast<float>(ServiceVolume(u"AmbientLevel", u"AmbientMute"));
    float music = 0;
    if (!CallGameFunction<uint8_t,0x0058b100>("nomusic", 0))
        music = static_cast<float>(ServiceVolume(u"MusicLevel", u"MusicMute"));
    CallGameFunction<void,0x008638f0>(0x20681cd4u, 0x25df0108u, voice, 1);
    CallGameFunction<void,0x008638f0>(0x17705d3eu, 0x25df0108u, effects, 1);
    CallGameFunction<void,0x008638f0>(0x57a4661eu, 0x25df0108u, effects, 1);
    CallGameFunction<void,0x008638f0>(0xedf036d6u, 0x25df0108u, music, 1);
    CallGameFunction<void,0x008638f0>(0x05174939u, 0x25df0108u, ambient, 1);
    if (self->listener) static_cast<uint8_t*>(self->listener)[0xa61] = 1;
    return self->listener;
}
uintptr_t ResourceChanged(Manager* self, const ResourceEvent* event) {
    CallGameVirtual<void,17>(self, 0x0436201eu);
    CallGameVirtual<void,19>(self, 0x04362059u, event->state);
    CallGameVirtual<void,20>(self, 0x03475385u, event->low, event->high);
    CallGameVirtual<void,19>(self, 0x04362049u, event->extra);
    CallGameVirtual<void,26>(self);
    if (self->listener) CallGameMethod<void,0x0086d1a0>(self->listener, self);
    if (self->ambience) CallGameMethod<void,0x00862e60>(self->ambience, event->low, event->high);
    // The string is narrow, and the resource key is returned in EDX:EAX.
    const uint64_t key = CallGameVirtual<uint64_t,70>(self, "modifiers");
    if (uint32_t(key) != event->low || uint32_t(key>>32) != event->high) return uint32_t(key);
    // Release may re-enter and publish a new pointer. EA clears/releases twice.
    for (unsigned i=0;i<2;++i) {
        void* old = self->focus;
        if (old) { self->focus = nullptr; CallGameVirtual<void,1>(old); }
    }
    const uint64_t next = CallGameVirtual<uint64_t,70>(self, "modifiers");
    return CallGameVirtual<uintptr_t,7>(self, uint32_t(next), uint32_t(next>>32), &self->focus, 0x001407ecu);
}
uintptr_t Tick(Manager* self, float delta) {
    if (self->listener) CallGameMethod<void,0x0086d250>(self->listener, self);
    CallGameMethod<void,0x008b4730>(self, delta);
    if (self->ambience) {
        const float* position = CallGameVirtual<const float*,6>(self, 0);
        alignas(16) float packed[4]{position[0],position[1],position[2],position[0]};
        CallGameMethod<void,0x008627c0>(self->ambience, packed);
        CallGameMethod<void,0x00862c20>(self->ambience, delta);
    }
    void* script = CallGameFunction<void*,0x00938b50>();
    if (!script) return 0;
    Node** bucket = self->buckets;
    Node* entry = *bucket;
    if (!entry) { do { ++bucket; entry=*bucket; } while (!entry); }
    Node* end = self->buckets[self->bucketCount];
    uintptr_t result = reinterpret_cast<uintptr_t>(self->buckets);
    while (entry != end) {
        void* object = CallGameMethod<void*,0x00939560>(script, entry->low, entry->high);
        result = reinterpret_cast<uintptr_t>(object);
        if (object) {
            object = CallGameMethod<void*,0x00935770>(object, Global<0x011e8e70>());
            result = reinterpret_cast<uintptr_t>(object);
            if (object) result = CallGameMethod<uintptr_t,0x009436a0>(object);
        }
        entry = entry->next;
        if (!entry) { do { ++bucket; entry=*bucket; } while (!entry); }
    }
    return result;
}
uintptr_t SetFocus(Manager* self, uint8_t focused) {
    Message* message = CallGameFunction<Message*,0x004e3f90>(0x90, "Audio/SetFocusMessage", 0,0,0,0);
    if (!message) return 0;
    InitMessage(message, 0x0671158a);
    CallGameVirtual<void,1>(message);
    if (self->messageBus) {
        message->flags |= 1;
        const uint32_t name = 0x09cd802a;
        std::memcpy(message->payload, &name, 4);
        const float value = focused ? 1.0f : 0.0f;
        message->flags |= 2;
        std::memcpy(message->payload+8, &value, 4);
        CallGameVirtual<void,7>(self->messageBus, 0x0671158au, message, 0,0);
    }
    return CallGameVirtual<uintptr_t,2>(message);
}
void** Unwrap(void* const* self, void** out) {
    void* value = *self;
    *out = value ? reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(value)-0x10) : nullptr;
    return out;
}
void** UnwrapDirect(void* self, void** out) {
    *out = self ? reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(self)-0x10) : nullptr;
    return out;
}
Message* InitMessage(Message* self, uint32_t type) {
    self->type = type;
#if defined(_M_IX86)
    self->vtable = reinterpret_cast<void*>(0x0100df80);
#else
    self->vtable = nullptr; // game vtable is only available in the mapped x86 image
#endif
    self->refs.exchange(0);
    self->flags = 0;
    return self;
}
}
