#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m114 {
struct Range { unsigned char* begin; unsigned char* end; unsigned char* capacity; };
struct Builder { void* vtable; int32_t refs; unsigned char list[0x21c]; float weight; };
TS3_X86_OFFSET(Builder,list,8);
TS3_X86_OFFSET(Builder,weight,0x224);
// TS3W.exe 0x0083F0B0: Insert a 124-byte record; preserves the null-destination growth bug and EAX.
uintptr_t __fastcall Insert(Range*,void*,unsigned char*,unsigned char*);
// TS3W.exe 0x0083F2A0: Assign a record range; preserves the null-destination growth bug and EAX.
uintptr_t __fastcall Assign(Range*,void*,unsigned char*,unsigned char*,void*);
// TS3W.exe 0x0083F3B0: Construct builder variant 01023350, preserving untouched storage.
Builder* __fastcall ConstructA(Builder*,void*);
// TS3W.exe 0x0083F400: Construct builder variant 0102338C, preserving untouched storage.
Builder* __fastcall ConstructB(Builder*,void*);
// TS3W.exe 0x0083F4E0: Construct builder variant 010233C8, preserving untouched storage.
Builder* __fastcall ConstructC(Builder*,void*);
// TS3W.exe 0x0083F550: Construct builder variant 01023404, preserving untouched storage.
Builder* __fastcall ConstructD(Builder*,void*);
}
