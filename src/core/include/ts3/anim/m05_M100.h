#pragma once
#include "ts3/anim/m05_B20.h"
namespace ts3::anim::m100 {
// The scaffold's TrackManager has an opaque four-byte vptr region; use a native
// pointer here so the observed x86 prefix also has a valid widened x64 layout.
struct Manager {
    void* vtable;
    ListLink tracks;
    void* descriptor;
    uint8_t flag;
    unsigned char untouched[3];
    void* host;
};
TS3_X86_SIZE(Manager,0x18);
TS3_X86_OFFSET(Manager,tracks,4);
TS3_X86_OFFSET(Manager,host,0x14);
struct TrackKey { Controller* controller; void* payload; };
TS3_X86_OFFSET(TrackKey,payload,4);
// TS3W.exe 0x00583C90: Remove every node with the supplied payload key, restoring refs before deletion; no defined EAX on empty lists.
void Remove(ListLink*,const TrackKey*);
// TS3W.exe 0x00583D00: Update live controllers and defer payload-key removals until traversal completes; returns nonnull-controller count.
uint32_t Update(Manager*,float,uint8_t*,uint8_t*);
// TS3W.exe 0x00583E30: Construct the track-manager prefix without clearing padding.
Manager* Construct(Manager*,void*);
// TS3W.exe 0x00583E60: Detach and destroy tracks, optionally freeing the manager, and return its address.
Manager* Destroy(Manager*,uint32_t);
// TS3W.exe 0x00584010: Swap and mirror selected local pose components, then clear the valid bit and return the pose.
b20::Pose* Mirror(b20::Pose*);
// TS3W.exe 0x005841C0: Resolve clip channel names to skeleton indices, treating hash 0x2AF0A8AE as absent; returns true in AL only.
bool Bind(const SampledClipView*,RigResource*,int32_t*);
#if defined(_M_IX86)
void __fastcall HookRemove(ListLink*,void*,const TrackKey*);
uint32_t __fastcall HookUpdate(Manager*,void*,float,uint8_t*,uint8_t*);
Manager* __fastcall HookConstruct(Manager*,void*,void*);
Manager* __fastcall HookDestroy(Manager*,void*,uint32_t);
#endif
}
