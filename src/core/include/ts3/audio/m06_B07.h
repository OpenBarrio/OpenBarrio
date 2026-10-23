#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::audio::b07 {
// Pointer-bearing manager prefix; padding describes gaps, not serialized native state.
struct Manager {
    void** vtable; void** secondary;
    uint8_t gap08[0x9c]; void* context;
    uint8_t gapA8[0x58]; uint32_t tier;
    uint8_t gap104[0x230]; void* service;
    uint8_t gap338[0x278b8]; void* listener; void* ambience;
    uint8_t gap27bf8[0x6c]; uint32_t values[4], words[4], flags[4];
    uint32_t counts[29]; uint8_t reserved27d08; bool mode;
};
TS3_X86_OFFSET(Manager,context,0xa4);
TS3_X86_OFFSET(Manager,tier,0x100);
TS3_X86_OFFSET(Manager,service,0x334);
TS3_X86_OFFSET(Manager,listener,0x27bf0);
TS3_X86_OFFSET(Manager,ambience,0x27bf4);
TS3_X86_OFFSET(Manager,counts,0x27c94);
TS3_X86_OFFSET(Manager,mode,0x27d09);
struct Request {
    uint8_t reserved, active, blocked, pad;
    uint8_t gap04[0x34]; uint32_t kind, current, maximum;
    uint8_t gap44[0xc]; uint32_t category, count, limit;
};
TS3_X86_OFFSET(Request,category,0x50);
// TS3W.exe 0x00864EC0: map six category hashes to budget indices, unknown hashes to zero.
uint32_t Category(uint32_t hash);
// TS3W.exe 0x00864F80: unregister eight service notifications, then forward base shutdown.
uintptr_t Shutdown(Manager* self);
// TS3W.exe 0x008650B0: register startup notification, initialize ambience and synchronize device thread.
bool Start(Manager* self);
// TS3W.exe 0x00865110: unregister startup notification and stop the optional ambience layer.
uintptr_t Stop(Manager* self);
// TS3W.exe 0x00865150: register ordered RW effect and source descriptors.
uintptr_t RegisterNodes(Manager* self);
// TS3W.exe 0x00865210: format and hash an optional UTF16 name before forwarding to slot 120.
uintptr_t SelectName(Manager* self, const uint16_t* name);
// TS3W.exe 0x00865260: select mode 300 and mark the optional listener dirty (AL-only result).
bool SetMode(Manager* self, uint32_t mode);
// TS3W.exe 0x00865290: call base budget reset and clear all seven category counts.
uint32_t ResetCounts(Manager* self);
// TS3W.exe 0x008652D0: snapshot budget values and return the request eligibility code.
uint32_t Policy(Manager* self, Request* request);
// TS3W.exe 0x00865350: check signed reservation delta and update category count modulo 2^32.
bool Reserve(Manager* self, void* sound, int32_t delta);
// TS3W.exe 0x008653F0: resolve the named mix group and its parameter map entry.
uintptr_t Lookup(Manager* self);
// TS3W.exe 0x00865420: allocate/construct/retain a Sound; allocation failure still dereferences null.
bool CreateSound(Manager* self, void** output);
// TS3W.exe 0x00865480: dispatch audio messages; missing final scalar publishes entry-stack residue.
void Message(Manager* self, void* message, uint32_t entryScratch);
// TS3W.exe 0x008655E0: tail-forward to the base audio-system operation (exact thunk twin).
bool BaseOperation(Manager* self, void* message);
}

#if defined(_M_IX86)
namespace ts3::audio::b07 {
inline uintptr_t __fastcall HookShutdown(Manager* self, void*) { return Shutdown(self); }
inline bool __fastcall HookStart(Manager* self, void*) { return Start(self); }
inline uintptr_t __fastcall HookStop(Manager* self, void*) { return Stop(self); }
inline uintptr_t __fastcall HookRegisterNodes(Manager* self, void*) { return RegisterNodes(self); }
inline uintptr_t __fastcall HookSelectName(Manager* self, void*, const uint16_t* name) { return SelectName(self, name); }
inline bool __fastcall HookSetMode(Manager* self, void*, uint32_t mode) { return SetMode(self, mode); }
inline uint32_t __fastcall HookResetCounts(Manager* self, void*) { return ResetCounts(self); }
inline uint32_t __fastcall HookPolicy(Manager* self, void*, Request* request) { return Policy(self, request); }
inline bool __fastcall HookReserve(Manager* self, void*, void* sound, int32_t delta) { return Reserve(self, sound, delta); }
inline uintptr_t __fastcall HookLookup(Manager* self, void*) { return Lookup(self); }
inline bool __fastcall HookCreateSound(Manager* self, void*, void** output) { return CreateSound(self, output); }
// Kept disabled in the registration: UnitCountedHook runs before this adapter
// and can destroy EA's entry ESP-8 live-in. A pre-counter capture is required.
void __fastcall HookMessage(Manager* self, void*, void* message);
inline bool __fastcall HookBaseOperation(Manager* self, void*, void* message) { return BaseOperation(self, message); }
}
#endif
