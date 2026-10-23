#pragma once
#include "ts3/layout.h"
namespace ts3::anim::m138 {
struct Style {
 void* refs[20]; uint32_t values[5]; unsigned char flags[8];
 char* begin; char* end; char* capacity; void* descriptor;
};
struct Step { unsigned char opaque[24]; uint32_t key; };
struct Body {
 unsigned char opaque00[0x28]; Style style;
 unsigned char opaquea4[12]; void* slaveSource;
 void** slaves; void** slaveEnd; void** slaveCapacity; void* slaveDescriptor; void* master;
 int32_t slaveCount; uint8_t sync; unsigned char opaqueCD[3];
 unsigned char opaqueD0[0x54]; void* rig;
 unsigned char opaque128[0x14]; float* forwardTimes; float* backwardTimes;
 unsigned char opaque144[0x10]; float forwardRate; float backwardRate;
 unsigned char opaque15C[8]; float timeScale; unsigned char opaque168[0x18];
 uint8_t mirror; unsigned char opaque181[3]; uint8_t forced; unsigned char opaque185[3];
 Step* steps; Step* stepsEnd; Step* stepsCapacity; void* stepsDescriptor;
 uint32_t value198; unsigned char storage[0xa8]; uint32_t value244;
 uint32_t current; uint32_t target; uint8_t flags[5];

};
TS3_X86_SIZE(Style,0x7c);
TS3_X86_OFFSET(Body,slaveCount,0xc8);
TS3_X86_OFFSET(Body,rig,0x124);
TS3_X86_OFFSET(Body,forwardTimes,0x13c);
TS3_X86_OFFSET(Body,forwardRate,0x154);
TS3_X86_OFFSET(Body,mirror,0x180);
TS3_X86_OFFSET(Body,steps,0x188);
TS3_X86_OFFSET(Body,current,0x248);
TS3_X86_OFFSET(Body,flags,0x250);
// TS3W.exe 0x00850B90: create the body controller and its slave controllers, preserving release order.
bool CreateBody(Body* self);
// TS3W.exe 0x00850E20: start a forward style transition; second-create failure still returns true.
bool Forward(Body* self);
// TS3W.exe 0x00851280: start a backward style transition; second-create failure still returns true.
bool Backward(Body* self);
// TS3W.exe 0x00850B90: fastcall bridge, no stack arguments, AL boolean return.
bool __fastcall HookCreateBody(Body* self,void*);
// TS3W.exe 0x00850E20: fastcall bridge, no stack arguments, AL boolean return.
bool __fastcall HookForward(Body* self,void*);
// TS3W.exe 0x00851280: fastcall bridge, no stack arguments, AL boolean return.
bool __fastcall HookBackward(Body* self,void*);

}
