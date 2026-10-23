#pragma once
#include "ts3/layout.h"
#include <cstdint>
#include <atomic>
namespace ts3::audio::b08 {
struct RoutingWorld { uint8_t prefix[0x74]; void* routing; };
TS3_X86_OFFSET(RoutingWorld, routing, 0x74);
struct Levels {
    uint8_t prefix[0x27c64];
    uint32_t first[4], second[4], third[4];
};
struct Node { uint32_t opaque[2], low, high; Node* next; };
struct Manager {
    void* vtable;
    uint8_t pad0[0x334-sizeof(void*)];
    void* messageBus;
    uint8_t pad1[0x27bd4-0x338];
    Node** buckets;
    uint32_t bucketCount;
    uint8_t pad2[0x27bf0-0x27bdc];
    void* listener;
    void* ambience;
    uint8_t pad3[0x27d04-0x27bf8];
    void* focus;
    uint8_t enabled;
};
TS3_X86_OFFSET(Manager, messageBus, 0x334);
TS3_X86_OFFSET(Manager, listener, 0x27bf0);
TS3_X86_OFFSET(Manager, focus, 0x27d04);
struct ResourceEvent { uint32_t low, high, state, extra; };
struct Message {
    void* vtable;
    std::atomic<int32_t> refs;
    uint8_t payload[0x78];
    uint32_t type;
    uint32_t reserved;
    uint32_t flags;
};
TS3_X86_OFFSET(Message, type, 0x80);
TS3_X86_OFFSET(Message, flags, 0x88);
// TS3W.exe 0x008655F0: emit the four listener command properties and submit.
uintptr_t ListenerCommand(void* self, uint32_t a, const void* b, uint32_t c, uint32_t d);
// TS3W.exe 0x00865660: copy the three indexed level words, returning the third word.
uint32_t ReadLevels(const Levels* self, uint32_t index, uint32_t* a, uint32_t* b, uint32_t* c);
// TS3W.exe 0x00865690: lock the optional RW singleton and return self.
void* LockRW(void* self);
// TS3W.exe 0x008656B0: read a service volume, clamp using float sign bits, and scale.
double ServiceVolume(const char16_t* volume, const char16_t* disabled);
// TS3W.exe 0x00865770: look up the master gain and update it under the optional RW lock.
uintptr_t MasterGain(void* self, float gain);
// TS3W.exe 0x008657D0: accept an empty routing mask or one intersecting the current routing flags.
bool RoutingMask(void* self, const uint32_t* mask);
// TS3W.exe 0x00865840: resolve a script object and forward the three operation arguments.
uintptr_t ScriptOperation(void* self, uint32_t low, uint32_t high, uint32_t a, uint32_t b, uint32_t c);
// TS3W.exe 0x00865890: publish audio settings; EA intentionally sends SoundFX level to two groups.
void* PublishSettings(Manager* self);
// TS3W.exe 0x00865D20: emit a resource event and refresh the modifier resource, including reentrant releases.
uintptr_t ResourceChanged(Manager* self, const ResourceEvent* event);
// TS3W.exe 0x00865E10: update listener, ambience and each script-backed entry in the sparse bucket map.
uintptr_t Tick(Manager* self, float delta);
// TS3W.exe 0x00865F40: allocate, retain, post and release a focus message if allocation succeeds.
uintptr_t SetFocus(Manager* self, uint8_t focused);
// TS3W.exe 0x00865CB0: unwrap the pointed-to interior view, returning the output pointer.
void** Unwrap(void* const* self, void** out);
// TS3W.exe 0x00865CD0: unwrap an interior view, returning the output pointer.
void** UnwrapDirect(void* self, void** out);
// TS3W.exe 0x00865CF0: initialize a message header without touching its payload.
Message* InitMessage(Message* self, uint32_t type);
// ABI adapters: these empty views impose no native layout on the game objects.
struct Entry {
    uintptr_t Listener(uint32_t a,void* b,uint32_t c,uint32_t d) { return ListenerCommand(this,a,b,c,d); }
    uint32_t LevelsAt(uint32_t i,void* a,void* b,void* c) { return ReadLevels(reinterpret_cast<Levels*>(this),i,static_cast<uint32_t*>(a),static_cast<uint32_t*>(b),static_cast<uint32_t*>(c)); }
    void* Lock() { return LockRW(this); }
    uintptr_t Gain(float gain) { return MasterGain(this,gain); }
    bool Mask(void* mask) { return RoutingMask(this,static_cast<uint32_t*>(mask)); }
    uintptr_t Script(uint32_t low,uint32_t high,uint32_t a,uint32_t b,uint32_t c) { return ScriptOperation(this,low,high,a,b,c); }
    void* Settings() { return PublishSettings(reinterpret_cast<Manager*>(this)); }
    void* Indirect(void* out) { return Unwrap(reinterpret_cast<void**>(this),static_cast<void**>(out)); }
    void* Direct(void* out) { return UnwrapDirect(this,static_cast<void**>(out)); }
    void* Init(uint32_t type) { return InitMessage(reinterpret_cast<Message*>(this),type); }
    uintptr_t Resource(void* event) { return ResourceChanged(reinterpret_cast<Manager*>(this),static_cast<ResourceEvent*>(event)); }
    uintptr_t Update(float delta) { return Tick(reinterpret_cast<Manager*>(this),delta); }
    uintptr_t Focus(uint32_t focused) { return SetFocus(reinterpret_cast<Manager*>(this),static_cast<uint8_t>(focused)); }
};
inline double VolumeABI(void* volume, void* disabled) { return ServiceVolume(static_cast<const char16_t*>(volume),static_cast<const char16_t*>(disabled)); }
}
