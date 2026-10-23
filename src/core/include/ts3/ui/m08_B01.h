#pragma once
#include "ts3/layout.h"
namespace ts3::ui {
struct B01Animation {
    void* table;
    unsigned char* begin;
    unsigned char* end;
    unsigned char* capacity;
    void* descriptorTable;
    uint32_t descriptorVersion;
    uint32_t references;
};
struct B01Descriptor { void* table; B01Animation* object; uint32_t version; uint32_t type; };
TS3_X86_SIZE(B01Animation,0x1c);
TS3_X86_OFFSET(B01Animation,references,0x18);
TS3_X86_SIZE(B01Descriptor,0x10);
// TS3W.exe 0x00504E70: Allocate with a 16-byte allocator prefix and rounded alignment.
void* __cdecl B01Allocate(uint32_t size,uint32_t alignment,const char* tag,void* allocator);
// TS3W.exe 0x00504F90: Query the curve interface or either common base interface.
void* __fastcall B01QueryCurve(void*,void*,uint32_t);
// TS3W.exe 0x00504FC0: Return the curve type identifier.
uint32_t __fastcall B01CurveType(void*,void*);
// TS3W.exe 0x00505030: Query the path interface or either common base interface.
void* __fastcall B01QueryPath(void*,void*,uint32_t);
// TS3W.exe 0x00505060: Increment the non-atomic reference count, retaining wraparound.
uint32_t __fastcall B01Retain(B01Animation*,void*);
// TS3W.exe 0x00505070: Decrement references and invoke deleting slot 2 exactly at zero.
uint32_t __fastcall B01Release(B01Animation*,void*);
// TS3W.exe 0x00505090: Return the path type identifier.
uint32_t __fastcall B01PathType(void*,void*);
// TS3W.exe 0x00505130: Return the arithmetic-shifted count of eight-byte curve records.
int32_t __fastcall B01CurveCount(B01Animation*,void*);
// TS3W.exe 0x00505140: Copy at most capacity curve records but return the full count.
int32_t __fastcall B01CopyCurve(B01Animation*,void*,void*,uint32_t);
// TS3W.exe 0x00505190: Initialize a borrowed curve descriptor and return the virtual type ID.
uint32_t __fastcall B01DescribeCurve(B01Animation*,void*,B01Descriptor*);
// TS3W.exe 0x005051F0: Return the signed twelve-byte path record count, truncating toward zero.
int32_t __fastcall B01PathCount(B01Animation*,void*);
// TS3W.exe 0x00505210: Copy at most capacity path records but return the full count.
int32_t __fastcall B01CopyPath(B01Animation*,void*,void*,uint32_t);
// TS3W.exe 0x00505280: Initialize a borrowed path descriptor and return the virtual type ID.
uint32_t __fastcall B01DescribePath(B01Animation*,void*,B01Descriptor*);
// TS3W.exe 0x00505380: Allocate and initialize an empty curve, with zero references.
B01Animation* __stdcall B01CreateCurve(void*,void*);
// TS3W.exe 0x005053D0: Allocate and initialize an empty path, with zero references.
B01Animation* __stdcall B01CreatePath(void*,void*);
}
