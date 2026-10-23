#pragma once
#include "ts3/anim/m05_B01.h"
namespace ts3::anim::m07 {
struct Evaluator { b01::Node base; float value; uint8_t first,second; };
struct Block { Block* next; unsigned char* begin; unsigned char* end; unsigned char* limit; };
struct Stream { int32_t cursor; void* allocator; Block* readBlock; union { Block* writeBlock; unsigned char* input; }; unsigned char* output; unsigned char* limit; };
TS3_X86_OFFSET(Evaluator,value,0x24);
TS3_X86_OFFSET(Evaluator,first,0x28);
TS3_X86_OFFSET(Stream,output,0x10);
// TS3W.exe 0x0054F280: Construct the evaluator, leaving first and sibling links untouched.
Evaluator* __fastcall Construct(Evaluator*,void*,void*);
// TS3W.exe 0x0054F2B0: Return the evaluator class identifier.
uint32_t __fastcall Type(void*,void*);
// TS3W.exe 0x0054F2C0: Query evaluator or base identity.
void* __fastcall Query(void*,void*,uint32_t);
// TS3W.exe 0x0054F2F0: Reset derived then base vtable; EAX has no defined return.
void __fastcall Reset(Evaluator*,void*);
// TS3W.exe 0x0054F300: Save raw flag bytes and float bits; return cursor or refill EAX.
uintptr_t __fastcall Save(Evaluator*,void*,Stream*);
// TS3W.exe 0x0054F380: Load normalized flags and x87 float; failed flags use the stream pointer low byte.
uintptr_t __fastcall Load(Evaluator*,void*,Stream*);
}
