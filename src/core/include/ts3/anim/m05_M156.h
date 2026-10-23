#pragma once
#include "ts3/layout.h"
#include <cstdint>
namespace ts3::anim {
struct M156Link { M156Link* next; M156Link* previous; };
struct M156Part { void* vtable; M156Link link; void* owner; };
struct M156Body {
    void* vtable;
    void* interfaceVtable;
    uint32_t state;
    M156Link parts;
    void* owner;
    M156Part first;
    M156Part second;
};
TS3_X86_OFFSET(M156Body, first, 0x18);
TS3_X86_OFFSET(M156Body, second, 0x28);
struct M156Resource { void* vtable; volatile int32_t refs; };
struct M156Reference { void* vtable; M156Resource* resource; };
// TS3W.exe 0x00953310: Construct a body containing one locomotion style part; returns self.
M156Body* Construct953310(M156Body*, void* owner);
// TS3W.exe 0x00953390: Construct a body containing one alternate style part; returns self.
M156Body* Construct953390(M156Body*, void* owner);
// TS3W.exe 0x00953410: Construct a body with two ordered style parts; returns self.
M156Body* Construct953410(M156Body*, void* owner);
// TS3W.exe 0x009534A0: Construct a body with two ordered alternate style parts; returns self.
M156Body* Construct9534A0(M156Body*, void* owner);
// TS3W.exe 0x00942C00: Pin a resource, clear its update flag, and release the pin; returns refs address or deleting-slot result.
uintptr_t Update942C00(M156Reference*, uintptr_t unused);
struct M156Stream {
    int32_t cursor;
    uint32_t opaque4, opaque8;
    unsigned char* readEnd;
    unsigned char* positionEnd;
    uint8_t failed;
};
TS3_X86_OFFSET(M156Stream, failed, 0x14);
// TS3W.exe 0x009420E0: Consume animation streams, build/evaluate pose registries, and update the host; returns AL=1.
uint8_t Update9420E0(uint32_t tick, M156Stream*, M156Stream*, float fraction, void* source);
// ABI adapters: ECX self, unused EDX, one callee-popped stack argument.
#if defined(_M_IX86)
inline uintptr_t __fastcall HookUpdate942C00(M156Reference* self, void*, uintptr_t arg) { return Update942C00(self, arg); }
inline M156Body* __fastcall HookConstruct953310(M156Body* self, void*, void* owner) { return Construct953310(self, owner); }
inline M156Body* __fastcall HookConstruct953390(M156Body* self, void*, void* owner) { return Construct953390(self, owner); }
inline M156Body* __fastcall HookConstruct953410(M156Body* self, void*, void* owner) { return Construct953410(self, owner); }
inline M156Body* __fastcall HookConstruct9534A0(M156Body* self, void*, void* owner) { return Construct9534A0(self, owner); }
#endif
}
