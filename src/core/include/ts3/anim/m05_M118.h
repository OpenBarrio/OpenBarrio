#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m118 {
struct Controller { void** vtable; volatile int32_t refs; };
struct Node { Node* next; Node* previous; Controller* controller; float phase; };
struct Builder {
    void** vtable; uintptr_t unknown;
    Node sentinel; // controller overlays the free-node pool at +10
    unsigned char opaque[0x224-0x18];
    float current;
};
TS3_X86_OFFSET(Builder,sentinel,8);
TS3_X86_OFFSET(Builder,current,0x224);
struct String { char* begin; char* end; char* capacity; const char* descriptor; };
struct Effect { String name; uint32_t kind; };
struct Vector { Effect* begin; Effect* end; Effect* capacity; };
struct Entry { uint32_t key; Vector effects; uintptr_t unknown; Effect storage[4]; uint32_t state; };
TS3_X86_OFFSET(Entry,storage,20);
TS3_X86_OFFSET(Entry,state,100);
// TS3W.exe 0x00841720: dispatch matching phases or create and insert a controller; signed zero and unordered sign quirks retained.
uint8_t __fastcall Dispatch4(Builder*,void*,uint32_t,uint32_t);
// TS3W.exe 0x00841A10: dispatch using the fifth controller factory, preserving the low-byte flag contract.
uint8_t __fastcall Dispatch5(Builder*,void*,uint32_t,uint32_t);
// TS3W.exe 0x00841D00: dispatch using the sixth controller factory, preserving pooled-node ordering.
uint8_t __fastcall Dispatch6(Builder*,void*,uint32_t,uint32_t);
// TS3W.exe 0x00841FF0: replace an effect vector unless source and destination alias; returns destination.
Vector* __fastcall Assign(Vector*,void*,Vector*);
// TS3W.exe 0x00842030: build animation effect descriptions for a style and singular mask bit.
uint8_t __fastcall Effects(void*,void*,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,Vector*);
// TS3W.exe 0x008422F0: allocate and copy one effect-map entry; allocation failure still faults.
Entry* __fastcall Clone(void*,void*,Entry*);
}
