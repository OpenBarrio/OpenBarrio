#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::audio::b33 {
// Native pointers expand independently of the observed x86 gaps.
struct Emitter {
    void** vtable; void** secondary; uint8_t gap008[0x368];
    void* resource; void* retained; uint32_t word378; uint8_t enabled, gap37d[3];
    void* voices; uint32_t handle; uint32_t key[4]; uint8_t gap398[0x10];
    void* callback; uint32_t word3ac, property; uint32_t word3b4;
    double time; uint32_t word3c0; int32_t channels;
    uint8_t muted, dirty, gap3ca[2];
    float gain, pitch, pitchScale, spatial[6], word3f0, value3f4, rate, value3fc, value400, value404, value408;
    void* voice; void* rateNode; void* pitchNode; void* gainNode; uint32_t word41c,word420,flags,overrides;
    uint8_t gap42c[0xbc]; uint32_t word4e8;
};
TS3_X86_OFFSET(Emitter,resource,0x370);
TS3_X86_OFFSET(Emitter,callback,0x3a8);
TS3_X86_OFFSET(Emitter,channels,0x3c4);
TS3_X86_OFFSET(Emitter,flags,0x424);
TS3_X86_OFFSET(Emitter,word4e8,0x4e8);
// TS3W.exe 0x008A0C10: store and return the callback (adapted verified SetCallback twin).
void* SetCallback(Emitter*,void*);
// TS3W.exe 0x008A0C20: copy the resource key and force its third word, returning the original fourth word.
uint32_t SetKey(Emitter*,const uint32_t*);
// TS3W.exe 0x008A0C60: toggle flag 0x100 using only the argument low byte.
void SetFlag(Emitter*,uint32_t);
// TS3W.exe 0x008A0C90: notify command system then tail-call slot six.
uintptr_t Notify(Emitter*);
// TS3W.exe 0x008A0CC0: clear override then set rate in [0,10]; unordered values pass.
bool SetRate(Emitter*,float);
// TS3W.exe 0x008A0D10: set rate in [0.5,2], substituting one below two channels.
bool SetChannelRate(Emitter*,float);
// TS3W.exe 0x008A0D70: clear pitch override then forward values in [0,1], including NaN.
bool SetPitch(Emitter*,float);
// TS3W.exe 0x008A0DC0: poll RW handle then unregister and release the emitter when complete.
void Poll(Emitter*);
// TS3W.exe 0x008A0E20: stop active playback, clear retained pointer before release, reset word4e8.
bool Stop(Emitter*);
// TS3W.exe 0x008A0E60: validate and load resource properties, unless already playing.
bool Load(Emitter*);
// TS3W.exe 0x008A0F70: schedule playback and start voices only on the first transition.
bool Play(Emitter*,double);
// TS3W.exe 0x008A1080: update mute and dirty bytes and apply voice gain under RW lock.
bool Mute(Emitter*,uint32_t);
// TS3W.exe 0x008A1120: dispatch float property writes and override flags under the RW lock.
bool SetProperty(Emitter*,uint32_t,float,uint32_t);
// TS3W.exe 0x008A1510: dispatch float property reads with x87 NaN quieting, falling back to voice container.
bool GetProperty(Emitter*,uint32_t,float*);
}

#if defined(_M_IX86)
namespace ts3::audio::b33 {
inline void* __fastcall HookSetCallback(Emitter* s,void*,void* value) { return SetCallback(s,value); }
inline uint32_t __fastcall HookSetKey(Emitter* s,void*,const uint32_t* key) { return SetKey(s,key); }
inline void __fastcall HookSetFlag(Emitter* s,void*,uint32_t value) { SetFlag(s,value); }
inline uintptr_t __fastcall HookNotify(Emitter* s,void*) { return Notify(s); }
inline bool __fastcall HookSetRate(Emitter* s,void*,float value) { return SetRate(s,value); }
inline bool __fastcall HookSetChannelRate(Emitter* s,void*,float value) { return SetChannelRate(s,value); }
inline bool __fastcall HookSetPitch(Emitter* s,void*,float value) { return SetPitch(s,value); }
inline void __fastcall HookPoll(Emitter* s,void*) { Poll(s); }
inline bool __fastcall HookStop(Emitter* s,void*) { return Stop(s); }
inline bool __fastcall HookLoad(Emitter* s,void*) { return Load(s); }
inline bool __fastcall HookPlay(Emitter* s,void*,double value) { return Play(s,value); }
inline bool __fastcall HookMute(Emitter* s,void*,uint32_t value) { return Mute(s,value); }
inline bool __fastcall HookSetProperty(Emitter* s,void*,uint32_t id,float value,uint32_t deferred) { return SetProperty(s,id,value,deferred); }
inline bool __fastcall HookGetProperty(Emitter* s,void*,uint32_t id,float* out) { return GetProperty(s,id,out); }
}
#endif
