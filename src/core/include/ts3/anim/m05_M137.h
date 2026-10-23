#pragma once
#include "ts3/layout.h"
#include <cstdint>
#include <cstddef>
namespace ts3::anim::m137_detail {
// Consumed fields only; opaque bytes are not initialized by these operations.
struct StyleRecord { unsigned char opaque[24]; void* definition; };
struct Body {
 void* vtable; unsigned char p04[16]; void* canvas;
 unsigned char p18[24]; void* builder;
 unsigned char p34[72]; uint32_t styleArgs[4]; unsigned char p8c; uint8_t scaleEnabled;
 unsigned char p8e[34]; void* slaveConfig; void** slaves; unsigned char pb8[12]; void* master;
 int32_t slaveCount; uint8_t slaveTime; unsigned char pcd[68]; uint8_t special;
 unsigned char p112[18]; uint32_t name; unsigned char p128[4];
 float times[3]; unsigned char p138[12]; float scales[3]; unsigned char p150[20];
 float speed; unsigned char p168[4]; uint32_t initialCounter,counter,otherCounter;
 unsigned char p178[6]; uint8_t started,finished,mirror; unsigned char p181[3]; uint8_t reverse;
 unsigned char p185[3]; StyleRecord* stylesBegin; StyleRecord* stylesEnd; StyleRecord* stylesCapacity;
 unsigned char p194[180]; uint32_t styleIndex;
};
struct Request { uint32_t name; float time; uint32_t a,b; float c; uint8_t mirror,reverse; uint8_t padding[2]; void* map; };
struct ControllerResult { void* controller; uint8_t mirror; };
struct Controllers { void** begin; void** end; void** capacity; const char* descriptor; };
struct Style {
 void* slots[20]; void* taskSource; unsigned char opaque54[17]; uint8_t scaleEnabled; unsigned char opaque66[5];
 void* sound[4];
};
#if defined(_M_IX86)
static_assert(offsetof(Body,builder)==48 && offsetof(Body,master)==196);
static_assert(offsetof(Body,name)==292 && offsetof(Body,scales)==324);
static_assert(offsetof(Body,started)==382 && offsetof(Body,styleIndex)==584);
static_assert(sizeof(Request)==28 && offsetof(Style,scaleEnabled)==101);
#endif
// TS3W.exe 0x0084FB20: create and dispatch the animated body's initial root-motion controllers.
bool Start(Body*);
// TS3W.exe 0x00850120: build and dispatch the continuing root-motion controllers.
bool Continue(Body*);
// TS3W.exe 0x008504D0: build and dispatch completion controllers for the requested mode.
bool Finish(Body*,int32_t mode);
}

namespace ts3::anim {
struct m137 : m137_detail::Body {
 // TS3W.exe 0x0084FB20: initial root-motion ABI entry.
 bool Start() { return m137_detail::Start(this); }
 // TS3W.exe 0x00850120: continuing root-motion ABI entry.
 bool Continue() { return m137_detail::Continue(this); }
 // TS3W.exe 0x008504D0: completion root-motion ABI entry.
 bool Finish(int32_t mode) { return m137_detail::Finish(this,mode); }
};
}
