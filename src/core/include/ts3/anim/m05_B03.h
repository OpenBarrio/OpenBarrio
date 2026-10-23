#pragma once
#include "ts3/layout.h"
namespace ts3::anim {
struct B03Key { uint32_t word[4]; };
struct B03Binding { uint32_t word[4]; };
struct B03Resource { void** vtable; uint32_t refs; B03Key key; };
struct B03Evaluator {
    void* vtable; void* next; void* previous; void* firstChild; void* lastChild; void* parent;
    uint32_t opaque18[4];
    B03Binding binding[5][11];
    B03Key key, secondKey;
    B03Resource* resource; B03Resource* secondResource;
    float time, weight; uint8_t enabled, flag;
};
struct B03StreamBlock { B03StreamBlock* next; unsigned char* begin; unsigned char* end; unsigned char* limit; };
struct B03Stream { int32_t cursor; void* allocator; void* opaque08; B03StreamBlock* block; unsigned char* end; unsigned char* limit; };
TS3_X86_OFFSET(B03Evaluator,binding,0x28);
TS3_X86_OFFSET(B03Evaluator,key,0x398);
TS3_X86_OFFSET(B03Evaluator,resource,0x3b8);
TS3_X86_OFFSET(B03Evaluator,time,0x3c0);
TS3_X86_OFFSET(B03Evaluator,enabled,0x3c8);
TS3_X86_OFFSET(B03Resource,key,8);
TS3_X86_OFFSET(B03Stream,end,0x10);
// TS3W.exe 0x0054CEC0: Query the evaluator or base class ID.
void* __fastcall QueryCEC0(void*, void*, uint32_t);
// TS3W.exe 0x0054CF60: Return the class ID.
uint32_t __fastcall TypeCF60(void*, void*);
// TS3W.exe 0x0054CF70: Query the evaluator or base class ID.
void* __fastcall QueryCF70(void*, void*, uint32_t);
// TS3W.exe 0x0054CFC0: Return the class ID.
uint32_t __fastcall TypeCFC0(void*, void*);
// TS3W.exe 0x0054CFD0: Query the evaluator or base class ID.
void* __fastcall QueryCFD0(void*, void*, uint32_t);
// TS3W.exe 0x0054DC20: Save a key in word order 2,3,0,1; return the final store's buffer register.
uintptr_t __fastcall SaveKeyDC20(B03Stream*, void*, const B03Key*);
// TS3W.exe 0x0054DCB0: Read the pair at +10, or fault reading absolute 8/12 on null.
uint64_t __fastcall ValueDCB0(const void*, void*);
// TS3W.exe 0x0054DCD0: Store the float at +3C0; EAX is untouched (no defined return).
void __fastcall TimeDCD0(B03Evaluator*, void*, float);
// TS3W.exe 0x0054DCF0: Store the float at +3C4; EAX is untouched (no defined return).
void __fastcall WeightDCF0(B03Evaluator*, void*, float);
// TS3W.exe 0x0054DD10: Store and return the argument byte through AL only.
uint8_t __fastcall EnableDD10(B03Evaluator*, void*, uint8_t);
// TS3W.exe 0x0054DD20: Write four words, returning the record base minus 0x28.
void* __fastcall BindingDD20(B03Evaluator*, void*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
// TS3W.exe 0x0054DE50: Return the class ID.
uint32_t __fastcall TypeDE50(void*, void*);
// TS3W.exe 0x0054DE60: Query the evaluator or base class ID.
void* __fastcall QueryDE60(void*, void*, uint32_t);
// TS3W.exe 0x0054DE90: Retain, replace, release, and copy the resource key; null still faults.
uint32_t __fastcall ResourceDE90(B03Evaluator*, void*, B03Resource*);
// TS3W.exe 0x0054DFE0: Save flags, keys, times and the sparse 5-by-11 table, retaining empty-row records.
uint32_t __fastcall SaveDFE0(B03Evaluator*, void*, B03Stream*);
}
