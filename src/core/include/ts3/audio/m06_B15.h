#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::audio {
struct B15Packet { void* primary; void** secondary; volatile long refs; };
struct B15Node { B15Node* next; B15Node* previous; B15Packet* packet; };
struct B15Player {
    void* primary; void* secondary; void* tertiary;
    uint8_t pad0[0x380-12]; void* system;
    uint8_t pad1[0x3A8-0x384]; uint32_t handle; uint32_t mode;
    uint8_t pad2[0x40C-0x3B0]; void* voice;
    uint8_t pad3[0x4F0-0x410]; B15Node packets;
    uint8_t started, stopped, notified;
};
TS3_X86_OFFSET(B15Player, system, 0x380);
TS3_X86_OFFSET(B15Player, voice, 0x40C);
TS3_X86_OFFSET(B15Player, packets, 0x4F0);
TS3_X86_OFFSET(B15Player, started, 0x4FC);
struct B15Cache { uint32_t low, high; uint8_t result; };
// TS3W.exe 0x0086B030: Select stereo packet mode and return true (AL only).
bool B15SelectMode(B15Player* self);
// TS3W.exe 0x0086B040: Read the voice position under the RW lock, rounding double to float.
float B15Position(B15Player* self);
// TS3W.exe 0x0086B0A0: Query the packet-player property 0x50506C30 from the audio system.
uint32_t B15Property(B15Player* self);
// TS3W.exe 0x0086B0C0: Release a retained packet; reset its reference count before deleting.
uintptr_t B15Release(B15Packet** packet);
// TS3W.exe 0x0086B0F0: Release and free every list node without resetting the sentinel links.
void B15Clear(B15Node* sentinel);
// TS3W.exe 0x0086B3C0: Adjust the tertiary interface by two pointer fields and delete the player.
B15Player* B15DestroySecondary(void* self, uint32_t flags);
// TS3W.exe 0x0086B860: Construct the emitter base and initialize packet-player tables and sentinel.
B15Player* B15Construct(B15Player* self);
// TS3W.exe 0x0086B8B0: Destroy the packet list and emitter base, optionally freeing the object.
B15Player* B15Destroy(B15Player* self, uint32_t flags);
// TS3W.exe 0x0086B8F0: Zero the cached two-word key and byte result without touching tail padding.
B15Cache* B15InitCache(B15Cache* self);
struct B15Playlist {
    uint8_t prefix[12]; uint32_t flags;
    uint8_t padding[0x58-0x10]; void* player;
};
struct B15KeySource { uint32_t prefix[2]; uint32_t low,high; };
// TS3W.exe 0x0086AF70: Process playlist suspend/start flags, poll completion and advance time.
uint32_t B15PlaylistTick(B15Playlist* self,uint32_t unused,float elapsed);
// TS3W.exe 0x0086B150: Handle packet arrival and stop/start notifications, retaining queued packets.
bool B15Event(B15Player* self,uint32_t event,uint32_t arg,uintptr_t token);
// TS3W.exe 0x0086B3D0: Stop the voice, wait for packets to drain and publish an audio-stopped message.
bool B15Stop(B15Player* self);
// TS3W.exe 0x0086B5F0: Retire consumed packets, notify on an empty stopped queue and tick the base emitter.
uint32_t B15Tick(void* secondary,float first,float second);
// TS3W.exe 0x0086B900: Cache a resource-key spatial query; early failures leave the previous key untouched.
uint8_t B15Query(B15Cache* self,const B15KeySource* source);
// Stateless member adapters for the original thiscall ABI.
struct B15Hooks {
    bool SelectMode() { return B15SelectMode(reinterpret_cast<B15Player*>(this)); }
    float Position() { return B15Position(reinterpret_cast<B15Player*>(this)); }
    uint32_t Property() { return B15Property(reinterpret_cast<B15Player*>(this)); }
    uintptr_t Release() { return B15Release(reinterpret_cast<B15Packet**>(this)); }
    void Clear() { B15Clear(reinterpret_cast<B15Node*>(this)); }
    B15Player* Destroy(uint32_t flags) { return B15Destroy(reinterpret_cast<B15Player*>(this), flags); }
    B15Cache* InitCache() { return B15InitCache(reinterpret_cast<B15Cache*>(this)); }
};
inline bool __fastcall B15SelectModeAbi(B15Player* self,void*) { return B15SelectMode(self); }
inline float __fastcall B15PositionAbi(B15Player* self,void*) { return B15Position(self); }
inline uint32_t __fastcall B15PropertyAbi(B15Player* self,void*) { return B15Property(self); }
inline uintptr_t __fastcall B15ReleaseAbi(B15Packet** self,void*) { return B15Release(self); }
inline void __fastcall B15ClearAbi(B15Node* self,void*) { B15Clear(self); }
inline B15Player* __fastcall B15ConstructAbi(B15Player* self,void*) { return B15Construct(self); }
inline B15Player* __fastcall B15DestroySecondaryAbi(void* self,void*,uint32_t flags) { return B15DestroySecondary(self,flags); }
inline B15Player* __fastcall B15DestroyAbi(B15Player* self,void*,uint32_t flags) { return B15Destroy(self,flags); }
inline B15Cache* __fastcall B15InitCacheAbi(B15Cache* self,void*) { return B15InitCache(self); }
inline uint32_t __fastcall B15PlaylistTickAbi(B15Playlist* self,void*,uint32_t unused,float elapsed) { return B15PlaylistTick(self,unused,elapsed); }
inline bool __fastcall B15EventAbi(B15Player* self,void*,uint32_t event,uint32_t arg,uintptr_t token) { return B15Event(self,event,arg,token); }
inline bool __fastcall B15StopAbi(B15Player* self,void*) { return B15Stop(self); }
inline uint32_t __fastcall B15TickAbi(void* self,void*,float first,float second) { return B15Tick(self,first,second); }
inline uint8_t __fastcall B15QueryAbi(B15Cache* self,void*,const B15KeySource* source) { return B15Query(self,source); }
}
