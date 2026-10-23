#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::audio::b02impl {
struct Binding { uint32_t id; void* evaluator; };
struct Ambience {
    void** vtable; void** secondary;
    uint32_t reserved[2], token, handles[32], used, count, unused, key[2];
    uint8_t enabled, evaluating, autoStart, initialized;
    float interval, elapsed;
    Binding* begin; Binding* end;
};
TS3_X86_OFFSET(Ambience, handles, 0x14);
TS3_X86_OFFSET(Ambience, count, 0x98);
TS3_X86_OFFSET(Ambience, enabled, 0xA8);
TS3_X86_OFFSET(Ambience, begin, 0xB4);
// Native bridge injection; null uses the original x86 singleton getter.
void SetCommandSystem(void* system);
// TS3W.exe 0x00861130: Submit a sound key, handle and three float parameters.
void Play(uint32_t lo,uint32_t hi,uint32_t handle,float a,float b,float c);
// TS3W.exe 0x008611D0: Submit a sound key and handle.
void PlaySimple(uint32_t lo,uint32_t hi,uint32_t handle);
// TS3W.exe 0x00861230: Stop a nonzero handle with a supplied mode.
void Stop(uint32_t handle,uint32_t mode);
// TS3W.exe 0x00861290: Unregister the callback, stop and release all occupied slots.
void Reset(Ambience* self);
// TS3W.exe 0x00861380: Invoke auto-start only when disabled and configured.
void TickAuto(Ambience* self,float dt);
// TS3W.exe 0x008613A0: Forward to virtual slot 14.
void Forward(Ambience* self);
// TS3W.exe 0x008613B0: Query enable/count; preserve stale upper return bits.
uint32_t Query(Ambience* self,uint32_t id,float* out);
// TS3W.exe 0x00861420: Stop occupied handles without clearing or decrementing them.
void StopAll(Ambience* self);
// TS3W.exe 0x008614B0: Advance spawn timer and fill the first free handle slot.
void Update(Ambience* self,float dt);
// TS3W.exe 0x00861580: Read ambience properties, register callback and initialize slots.
uint32_t Configure(Ambience* self,void* properties);
// TS3W.exe 0x00861740: Evaluate active bindings through the secondary interface; EA forwards vector count instead of binding ID.
void Evaluate(void* secondary,uint32_t unused1,uint32_t unused2);
// TS3W.exe 0x008617E0: Release a completed handle through the token interface; AL is always zero.
uint8_t Complete(void* tokenView,uint32_t unused,void* message);
// TS3W.exe 0x00861850: Set enable/count, retaining fabs/truncate/clamp and NaN enable behavior.
void Set(Ambience* self,uint32_t id,float value,uint32_t unused);
}
// ABI adapters are member functions so the registration tool can express thiscall.
namespace ts3::audio {
struct b02 : b02impl::Ambience {
    void Reset() { b02impl::Reset(this); }
    void TickAuto(float dt) { b02impl::TickAuto(this,dt); }
    void Forward() { b02impl::Forward(this); }
    uint32_t Query(uint32_t id,void* out) { return b02impl::Query(this,id,static_cast<float*>(out)); }
    void StopAll() { b02impl::StopAll(this); }
    void Update(float dt) { b02impl::Update(this,dt); }
    uint32_t Configure(void* r) { return b02impl::Configure(this,r); }
    void Evaluate(uint32_t a,uint32_t b) { b02impl::Evaluate(this,a,b); }
    bool Complete(uint32_t a,void* m) { return b02impl::Complete(this,a,m)!=0; }
    void Set(uint32_t id,float v,uint32_t a) { b02impl::Set(this,id,v,a); }
};
}
