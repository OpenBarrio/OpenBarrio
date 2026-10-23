#pragma once
#include "ts3/layout.h"
#include "ts3/scratch/clu_4b4250_Scratch.h"
namespace ts3::audio::b13 {
struct Player {
    void** vtable;
    uint32_t reserved[3];
    uint32_t flags;
    void* resource;
    uint32_t reserved18[2];
    uint32_t key[4];
    float position[3], gain, distance, pitch, pan, level, speed, rolloff, maximum;
    void* handle;
};
TS3_X86_OFFSET(Player, flags, 0x10);
TS3_X86_OFFSET(Player, key, 0x20);
TS3_X86_OFFSET(Player, handle, 0x5c);
struct PlaylistEntry { scratch::WideString name; uint32_t payload[32]; };
TS3_X86_SIZE(PlaylistEntry, 0x90);
// TS3W.exe 0x008698C0: Set or clear a flag mask using the low byte of enabled.
uint32_t __fastcall SetFlags(Player*, void*, uint32_t, uint8_t);
// TS3W.exe 0x00869900: Close an open player after its virtual stop callback.
uint8_t __fastcall Close(Player*, void*);
// TS3W.exe 0x00869930: Stop a started player under the RW lock.
uint8_t __fastcall Stop(Player*, void*);
// TS3W.exe 0x00869960: Set looping and mark whether the raw byte differs from the old Boolean.
uint8_t __fastcall SetLoop(Player*, void*, uint8_t);
// TS3W.exe 0x008699B0: Read the looping flag.
uint8_t __fastcall IsLooping(Player*, void*);
// TS3W.exe 0x008699D0: Copy four key words sequentially and return the final word.
uint32_t __fastcall SetKey(Player*, void*, const uint32_t*);
// TS3W.exe 0x008699F0: Set the deferred-start flag from a byte.
void __fastcall SetDeferred(Player*, void*, uint8_t);
// TS3W.exe 0x00869A10: Compute reciprocal distance attenuation; NaN and zero comparisons retained.
float __fastcall Attenuation(Player*, void*, float, float, float, float);
// TS3W.exe 0x00869A50: Start unless already active, optionally reading a resource property.
uint8_t __fastcall Start(Player*, void*);
// TS3W.exe 0x00869AA0: Dispatch a float-valued property getter.
uint8_t __fastcall GetProperty(Player*, void*, uint32_t, float*);
// TS3W.exe 0x00869AA9: Continue the property getter (entry inherits key in EAX and comparison flags).
uint8_t __fastcall GetPropertyContinuation(Player*, void*, uint32_t, float*);
// TS3W.exe 0x00869CE0: Retain a new resource before publishing it and releasing the old one.
void __fastcall SetResource(Player*, void*, void*);
// TS3W.exe 0x00869D20: Apply spatial gain and dirty player properties in original lock/callback order.
void __fastcall Update(Player*, void*, uint32_t, uint32_t);
// TS3W.exe 0x00869F10: Swap playlist values by copying strings and the 128-byte payload.
void __cdecl SwapEntries(PlaylistEntry*, PlaylistEntry*);
// TS3W.exe 0x00869FC0: Copy playlist values backwards, retaining sequential forward payload copies.
PlaylistEntry* __cdecl CopyBackward(const PlaylistEntry*, const PlaylistEntry*, PlaylistEntry*);
}
