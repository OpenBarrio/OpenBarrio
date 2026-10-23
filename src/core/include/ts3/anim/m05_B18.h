#pragma once
#include "ts3/layout.h"
namespace ts3::anim {
struct B18Pair { uint32_t low, high; };
struct B18Vector { B18Pair* begin; B18Pair* end; B18Pair* capacity; uint32_t descriptor; };
struct B18Manager { unsigned char opaque[0x18]; B18Vector pending; };
TS3_X86_SIZE(B18Pair,8);
TS3_X86_OFFSET(B18Vector,descriptor,0x0c);
TS3_X86_OFFSET(B18Manager,pending,0x18);
// TS3W.exe 0x00570A30: Returns scratch allocation size 0x3D0.
uint32_t __fastcall B18SizeA(void*,void*);
// TS3W.exe 0x00570AA0: Forwards to the object's slot 5 with argument zero, returning EAX.
uintptr_t __stdcall B18Forward(void*);
// TS3W.exe 0x00570B00: Returns allocation size 0x24.
uint32_t __fastcall B18SizeB(void*,void*);
// TS3W.exe 0x00570B30: Returns allocation size 0x2C.
uint32_t __fastcall B18SizeC(void*,void*);
// TS3W.exe 0x00570B60: Returns allocation size 0x240.
uint32_t __fastcall B18SizeD(void*,void*);
// TS3W.exe 0x00570B70: Returns interface identifier 0x5419E34E.
uint32_t __fastcall B18IdA(void*,void*);
// TS3W.exe 0x00570B80: Returns this for any of three supported interface identifiers.
void* __fastcall B18QueryA(void*,void*,uint32_t);
// TS3W.exe 0x00570BB0: Returns one exactly for argument one.
uint32_t __fastcall B18EqualOne(void*,void*,uint32_t);
// TS3W.exe 0x00570BF0: Returns interface identifier 0x7419E37C.
uint32_t __fastcall B18IdB(void*,void*);
// TS3W.exe 0x00570C00: Returns this for any of three supported interface identifiers.
void* __fastcall B18QueryB(void*,void*,uint32_t);
// TS3W.exe 0x00570C30: Returns one for unsigned arguments greater than one.
uint32_t __fastcall B18GreaterOne(void*,void*,uint32_t);
// TS3W.exe 0x00570C90: Returns allocation size 0x28.
uint32_t __fastcall B18SizeE(void*,void*);
// TS3W.exe 0x00570CC0: Returns allocation size 0x50.
uint32_t __fastcall B18SizeF(void*,void*);
// TS3W.exe 0x00572260: Inserts an identifier pair; spare-capacity EAX is the low word, growth EAX is capacity.
uintptr_t __fastcall B18Insert(B18Vector*,void*,B18Pair*,const B18Pair*);
// TS3W.exe 0x00572450: Appends an identifier pair, preserving the original path-dependent EAX.
uintptr_t __fastcall B18Append(B18Manager*,void*,uint32_t,uint32_t);
}
