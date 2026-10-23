#pragma once
#include <cstdint>
#include <cstddef>
namespace ts3::audio {
struct B11Entry { union { uint32_t key; B11Entry* freeNext; }; B11Entry* next; void* reserved[3]; void* retained; };
struct B11Map { void* reserved[3]; B11Entry* head; void* unused[2]; B11Entry* free; };
struct B11Bucket { void* reserved[2]; B11Map map; uint8_t tail[0xb8-8-28]; B11Bucket* next; };
struct B11SoundArray {
    void* vtable; int32_t refs; void* untouched; const void* empty; uint32_t one;
    void* zero14; float gain, pitch; uint32_t zero20; const void* mapLabel;
    void* first; void* last; const void* queueLabel; uint32_t untouched34;
    alignas(void*) uint8_t mutex[sizeof(void*) == 8 ? 40 : 32];
    void* nameBegin; void* nameEnd; void* nameCapacity; const void* nameLabel;
    uint32_t active, mode; uint8_t flag; uint8_t padding[3]; float update;
};
// TS3W.exe 0x008689B0: Drain bucket chains, release retained values and recycle entries before freeing buckets.
void B11Clear(B11Bucket** buckets, uint32_t count);
// TS3W.exe 0x00868A50: Construct a SoundArray with its name, mode and low-byte flag.
B11SoundArray* B11ConstructArray(B11SoundArray*, const char*, uint32_t, uint32_t);
// TS3W.exe 0x00868B00: Destroy a SoundArray, optionally freeing it for flags bit zero.
void* B11DeleteArray(void*, uint32_t);
// TS3W.exe 0x00868D00: Dispatch resource loading by its exact type ID, preserving the full callee return.
uint32_t B11Load(void*, void*);
// TS3W.exe 0x00868D70: Construct the base manager and its additional critical section.
void* B11Construct(void*);
// TS3W.exe 0x00868DA0: Install intermediate tables, destroy the critical section then destroy the base.
uintptr_t B11Destruct(void*);
// TS3W.exe 0x00868DD0: Initialize base configuration then register streaming services; only AL is replaced.
uint32_t B11Initialize(void*, void*);
// TS3W.exe 0x00868E00: Submit startup command and update the base with positive zero.
uintptr_t B11Start(void*);
// TS3W.exe 0x00868E40: Stop RW service before stopping the base manager.
uintptr_t B11Stop(void*);
// TS3W.exe 0x00868E50: Tail-forward to the base sample factory operation.
uintptr_t B11Samples(void*);
// TS3W.exe 0x00868E60: Tail-forward to the base voice-template operation.
uintptr_t B11Voices(void*);
// TS3W.exe 0x00868E70: Forward two float arguments through x87 stores to the base update.
uintptr_t B11Update(void*, float, float);
// TS3W.exe 0x00868E90: Adjust the secondary interface back to the manager before deleting.
void* B11DeleteSecondary(void*, uint32_t);
// TS3W.exe 0x00868EA0: Destroy the intermediate manager and optionally free it; return the original pointer.
void* B11Delete(void*, uint32_t);
}

#if defined(_M_IX86)
namespace ts3::audio {
// TS3W.exe 0x008689B0: ABI adapter for B11Clear.
inline void __fastcall HookB11Clear(void* self, void*, B11Bucket** buckets, uint32_t count) { return B11Clear(buckets, count); }
// TS3W.exe 0x00868A50: ABI adapter for B11ConstructArray.
inline B11SoundArray* __fastcall HookB11ConstructArray(B11SoundArray* self, void*, const char* name, uint32_t mode, uint32_t flag) { return B11ConstructArray(self, name, mode, flag); }
// TS3W.exe 0x00868B00: ABI adapter for B11DeleteArray.
inline void* __fastcall HookB11DeleteArray(void* self, void*, uint32_t flags) { return B11DeleteArray(self, flags); }
// TS3W.exe 0x00868D00: ABI adapter for B11Load.
inline uint32_t __fastcall HookB11Load(void* self, void*, void* resource) { return B11Load(self, resource); }
// TS3W.exe 0x00868D70: ABI adapter for B11Construct.
inline void* __fastcall HookB11Construct(void* self, void*) { return B11Construct(self); }
// TS3W.exe 0x00868DA0: ABI adapter for B11Destruct.
inline uintptr_t __fastcall HookB11Destruct(void* self, void*) { return B11Destruct(self); }
// TS3W.exe 0x00868DD0: ABI adapter for B11Initialize.
inline uint32_t __fastcall HookB11Initialize(void* self, void*, void* config) { return B11Initialize(self, config); }
// TS3W.exe 0x00868E00: ABI adapter for B11Start.
inline uintptr_t __fastcall HookB11Start(void* self, void*) { return B11Start(self); }
// TS3W.exe 0x00868E40: ABI adapter for B11Stop.
inline uintptr_t __fastcall HookB11Stop(void* self, void*) { return B11Stop(self); }
// TS3W.exe 0x00868E50: ABI adapter for B11Samples.
inline uintptr_t __fastcall HookB11Samples(void* self, void*) { return B11Samples(self); }
// TS3W.exe 0x00868E60: ABI adapter for B11Voices.
inline uintptr_t __fastcall HookB11Voices(void* self, void*) { return B11Voices(self); }
// TS3W.exe 0x00868E70: ABI adapter for B11Update.
inline uintptr_t __fastcall HookB11Update(void* self, void*, float a, float b) { return B11Update(self, a, b); }
// TS3W.exe 0x00868E90: ABI adapter for B11DeleteSecondary.
inline void* __fastcall HookB11DeleteSecondary(void* self, void*, uint32_t flags) { return B11DeleteSecondary(self, flags); }
// TS3W.exe 0x00868EA0: ABI adapter for B11Delete.
inline void* __fastcall HookB11Delete(void* self, void*, uint32_t flags) { return B11Delete(self, flags); }
}
#endif
