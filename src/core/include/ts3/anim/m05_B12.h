#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim {
struct B12Clip {
    Controller base;
    void *begin, *end, *capacity;
    uint32_t opaque34;
    unsigned char storage[0x4b8];
    ClipIkInfo ik;
    void* resource;
    void* source;
    uint32_t opaque5d8;
    float stateDC, stateE0, stateE4, period;
    uint32_t mode;
};
struct B12String {
    char *begin, *end, *capacity;
    void* descriptor;
    char storage[96];
};
struct B12Record { B12String first, second; uint32_t words[11]; uint32_t untouched10c; };
struct B12Vector { unsigned char *begin, *end, *capacity; void* descriptor; };
struct B12Blend {
    Controller base;
    void* secondary;
    void *first, *second;
    uint32_t opaque34;
    float duration;
};
TS3_X86_OFFSET(B12Clip,resource,0x5d0);
TS3_X86_OFFSET(B12Clip,source,0x5d4);
TS3_X86_OFFSET(B12Clip,mode,0x5ec);
TS3_X86_SIZE(B12String,0x70);
TS3_X86_OFFSET(B12Record,words,0xe0);
TS3_X86_SIZE(B12Record,0x110);
TS3_X86_OFFSET(B12Blend,first,0x2c);
TS3_X86_OFFSET(B12Blend,duration,0x38);
// TS3W.exe 0x005682F0: Return unsigned cycle count times period, or the FLT_MAX global.
float __fastcall Duration82F0(B12Clip*, void*);
// TS3W.exe 0x00568330: Retain the new source before replacement and release the old source; no defined EAX on identity.
void __fastcall Source8330(B12Clip*, void*, void*);
// TS3W.exe 0x00568370: Acquire a source by key and transfer the returned reference; result is AL only.
uint8_t __fastcall Acquire8370(B12Clip*, void*, const void*);
// TS3W.exe 0x005687E0: Return the secondary interface type ID.
uint32_t __fastcall Type87E0(void*, void*);
// TS3W.exe 0x005687F0: Query either supported ID, adjusting the secondary interface by minus eight.
void* __fastcall Query87F0(void*, void*, uint32_t);
// TS3W.exe 0x00568C50: Copy construct two inline strings and eleven scalar words, leaving descriptors and final padding untouched.
B12Record* __fastcall Copy8C50(B12Record*, void*, const B12Record*);
// TS3W.exe 0x00568D30: Assign two strings with identity checks and copy eleven scalar words.
B12Record* __fastcall Assign8D30(B12Record*, void*, const B12Record*);
// TS3W.exe 0x005690B0: Append six zero words, delegating capacity growth to the original helper; preserve its EAX.
uintptr_t __fastcall Append90B0(B12Vector*, void*);
// TS3W.exe 0x00569110: Copy construct a range with an out cursor; this is cdecl, not thiscall.
B12Record** CopyRange9110(B12Record**, const B12Record*, const B12Record*, B12Record*);
// TS3W.exe 0x0056A0E0: Set child times using child duration divided by parent duration, in child order.
void __fastcall SetTimeA0E0(B12Blend*, void*, float);
// TS3W.exe 0x0056A140: Scale first child's slot5 result, rounding the duration ratio to float first.
float __fastcall TimeA140(B12Blend*, void*);
// TS3W.exe 0x0056A170: Scale first child's slot6 result, rounding the duration ratio to float first.
float __fastcall TimeA170(B12Blend*, void*);
// TS3W.exe 0x0056A1A0: Scale first child's slot7 result, rounding the duration ratio to float first.
float __fastcall TimeA1A0(B12Blend*, void*);
// TS3W.exe 0x0056A1D0: Short circuit the two child predicates, testing only AL.
uint8_t __fastcall ActiveA1D0(B12Blend*, void*);
// TS3W.exe 0x0056A200: Release both non-null children without clearing the pointers; EAX is undefined on empty input.
void __fastcall ReleaseA200(B12Blend*, void*);
}
