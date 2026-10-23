#pragma once
#include "ts3/anim/m05_B01.h"
#include <xmmintrin.h>
namespace ts3::anim::m11 {
struct alignas(16) Evaluator {
    b01::Node base;
    unsigned char opaque[12];
    alignas(16) uint32_t direction[4];
    uint32_t key[2], index;
};
struct Cursor {
    int32_t position;
    uint32_t opaque[2];
    unsigned char* input;
    unsigned char* output;
    uint8_t readError;
    uint8_t reserved[3];
    uint8_t writeError;
};
TS3_X86_OFFSET(Evaluator,direction,0x30);
TS3_X86_OFFSET(Evaluator,index,0x48);
TS3_X86_OFFSET(Cursor,readError,0x14);
TS3_X86_OFFSET(Cursor,writeError,0x18);
// TS3W.exe 0x00555000: Return self for either evaluator identity, otherwise null.
void* __fastcall Query(void*,void*,uint32_t);
// TS3W.exe 0x005551D0: Construct evaluator, preserving opaque bytes and sibling links.
Evaluator* __fastcall Construct(Evaluator*,void*,void*);
// TS3W.exe 0x00555210: Reset base and optionally free, returning the freed address.
Evaluator* __fastcall Destroy(Evaluator*,void*,uint32_t);
// TS3W.exe 0x00555240: Save XYZ, key and index using the signed stream cursor; W is omitted.
uintptr_t __fastcall Save(Evaluator*,void*,Cursor*);
// TS3W.exe 0x005552D0: Load XYZ, key and index, preserving W and continuing after stream errors.
uintptr_t __fastcall Load(Evaluator*,void*,Cursor*);
// TS3W.exe 0x00555360: Compute clamped aim correction and compose it with the supplied rotation.
__m128* __cdecl Aim(__m128*,__m128*,__m128*,__m128*);
}
