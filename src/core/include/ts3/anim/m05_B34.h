#pragma once
#include "ts3/anim/m05_layouts.h"
namespace ts3::anim::b34 {
struct PoolSlot { PoolSlot* next; unsigned char opaque[12]; };
struct Pool { ListLink sentinel; PoolSlot* free; PoolSlot storage[32]; };
struct Node { Node* next; Node* previous; void* child; };
struct Tree { void* vtable; uint32_t refs; Node children; unsigned char opaque14[0x210]; float value; };
// Only the first two links of children are the sentinel; its payload is opaque.
struct Parameters { unsigned char opaque[0xa0]; uint32_t distance, moveAngle, turnAngle; };
struct FourValues { uint32_t values[4]; };
TS3_X86_OFFSET(Pool,storage,0xc);
TS3_X86_OFFSET(Tree,children,8);
TS3_X86_OFFSET(Tree,value,0x224);
TS3_X86_OFFSET(Parameters,distance,0xa0);
// TS3W.exe 0x0083E190: Initialize the circular list and inline free-slot pool without clearing slot payloads.
Pool* __fastcall ConstructPool(Pool*,void*);
// TS3W.exe 0x0083E360: Return the Tree Builder name (constant-leaf twin with a different immediate).
const char* __fastcall TreeName(void*,void*);
// TS3W.exe 0x0083E370: Copy the first scalar's raw bits and return the output pointer.
uint32_t* __fastcall CopyFirst(void*,void*,uint32_t*,const FourValues*);
// TS3W.exe 0x0083EA70: Load the tree scalar through x87, including signaling-NaN quieting.
float __fastcall TreeValue(const Tree*,void*);
// TS3W.exe 0x0083EA80: Test children in list order, stopping at the first nonzero low byte; empty is true.
uint8_t __fastcall AnyChild(Tree*,void*);
// TS3W.exe 0x0083EAC0: Visit children with depth+1; return the last callback EAX or the input depth if empty.
uint32_t __fastcall Visit(Tree*,void*,uint32_t);
// TS3W.exe 0x0083F3F0: Copy the second scalar's raw bits and return the output pointer.
uint32_t* __fastcall CopySecond(void*,void*,uint32_t*,const FourValues*);
// TS3W.exe 0x0083F520: Return the TurnAngle name (constant-leaf twin with a different immediate).
const char* __fastcall TurnName(void*,void*);
// TS3W.exe 0x0083F530: Copy the turn-angle bits and return zero.
uint32_t __fastcall Turn(void*,void*,const Parameters*,uint32_t*);
// TS3W.exe 0x0083F590: Copy the distance bits and return zero.
uint32_t __fastcall Distance(void*,void*,const Parameters*,uint32_t*);
// TS3W.exe 0x0083F600: Copy the fourth scalar's raw bits and return the output pointer.
uint32_t* __fastcall CopyFourth(void*,void*,uint32_t*,const FourValues*);
// TS3W.exe 0x0083F650: Return the Distance name (constant-leaf twin with a different immediate).
const char* __fastcall DistanceName(void*,void*);
// TS3W.exe 0x0083F6E0: Return the MoveAngle name (constant-leaf twin with a different immediate).
const char* __fastcall MoveName(void*,void*);
// TS3W.exe 0x0083F6F0: Copy the third scalar's raw bits and return the output pointer.
uint32_t* __fastcall CopyThird(void*,void*,uint32_t*,const FourValues*);
// TS3W.exe 0x0083F700: Copy move-angle bits, returning one only for ordered zero distance (either sign).
uint32_t __fastcall Move(void*,void*,const Parameters*,uint32_t*);
}
